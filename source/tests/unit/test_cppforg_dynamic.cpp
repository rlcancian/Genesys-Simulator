#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstdlib>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelManager.h"
#include "kernel/simulator/model/ModelComponentManager.h"
#include "kernel/simulator/model/ModelSimulation.h"
#include "kernel/simulator/essentialPlugins/Counter.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ExternalIntegration/CppForG.h"

namespace {
namespace fs = std::filesystem;
class ScopedCppForGDirectory {
public:
    ScopedCppForGDirectory() {
        std::string pattern = (fs::temp_directory_path() / "genesys-cppforg test-XXXXXX").string();
        std::vector<char> chars(pattern.begin(), pattern.end());
        chars.push_back('\0');
        if (char* made = ::mkdtemp(chars.data())) _path = made;
    }
    ~ScopedCppForGDirectory() {
        if (!_path.empty()) {
            std::error_code error;
            fs::remove_all(_path, error);
        }
    }
    const fs::path& path() const { return _path; }
private:
    fs::path _path;
};

class ScopedCurrentDirectory {
public:
    explicit ScopedCurrentDirectory(const fs::path& path) : _previous(fs::current_path()) {
        fs::current_path(path);
    }
    ~ScopedCurrentDirectory() {
        std::error_code error;
        fs::current_path(_previous, error);
    }
private:
    fs::path _previous;
};

unsigned countLines(const fs::path& filename) {
    unsigned lines = 0;
    std::ifstream input(filename);
    std::string row;
    while (std::getline(input, row)) ++lines;
    return lines;
}

std::string markerCode(const fs::path& filename) {
    return "std::ofstream stream(\"" + filename.string() +
        "\", std::ios::app); if (stream && entity) stream << \"hit\\n\";";
}

std::string entityApiCode(const fs::path& filename) {
    return "entity->setAttributeValue(\"CppForG.TestValue\", 42.0, \"\", true); "
        "std::ofstream stream(\"" + filename.string() +
        "\"); if (stream && entity) stream << entity->getAttributeValue(\"CppForG.TestValue\");";
}
} // namespace

TEST(CppForGDynamicTest, CreateCppForGDisposeExecutesGeneratedCodeOnEntityArrival) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const auto marker = scratch.path() / "arrived.txt";
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* create = new Create(model, "create");
    auto* cpp = new CppForG(model, "cppforg");
    auto* dispose = new Dispose(model, "dispose");
    create->getConnectionManager()->insert(cpp);
    cpp->getConnectionManager()->insert(dispose);
    create->setMaxCreations(1ul);
    create->setEntitiesPerCreation(1);
    create->setTimeBetweenCreationsExpression("10");
    cpp->setIncludesCode("#include <fstream>\n");
    cpp->setOnDispatchEventCode(markerCode(marker));

    ModelSimulation* simulation = model->getSimulation();
    simulation->setNumberOfReplications(1);
    simulation->setReplicationLength(2);
    simulation->setShowReportsAfterReplication(false);
    simulation->setShowReportsAfterSimulation(false);
    ASSERT_TRUE(model->check()) << "Generated C++ must compile and load before simulation";
    const void* handle = cpp->getCppCompiler()->getDynamicLibraryHandler();
    ASSERT_NE(handle, nullptr);
    simulation->start();
    EXPECT_EQ(countLines(marker), 1u) << "The generated function must run once per arriving entity";

    auto* counter = dynamic_cast<Counter*>(
        model->getDataManager()->getDataDefinition("Counter", "dispose.CountNumberIn"));
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(counter->getCountValue(), 1.0) << "Dispose must receive the forwarded entity";
    EXPECT_EQ(cpp->getCppCompiler()->getDynamicLibraryHandler(), handle)
        << "Model recheck must reuse the already loaded valid code";
}

TEST(CppForGDynamicTest, GeneratedCodeUsesEntityAttributeApiOnArrival) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    ScopedCurrentDirectory workingDirectory(scratch.path());
    const auto observedValue = scratch.path() / "entity-value.txt";
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* create = new Create(model, "create");
    auto* cpp = new CppForG(model, "cppforg_entity_api");
    auto* dispose = new Dispose(model, "dispose");
    create->getConnectionManager()->insert(cpp);
    cpp->getConnectionManager()->insert(dispose);
    create->setMaxCreations(1ul);
    create->setEntitiesPerCreation(1);
    create->setTimeBetweenCreationsExpression("10");
    cpp->setIncludesCode("#include <fstream>\n");
    cpp->setOnDispatchEventCode(entityApiCode(observedValue));
    model->getSimulation()->setNumberOfReplications(1);
    model->getSimulation()->setReplicationLength(2);
    model->getSimulation()->setShowReportsAfterReplication(false);
    model->getSimulation()->setShowReportsAfterSimulation(false);
    std::string checkError;
    ASSERT_TRUE(ModelDataDefinition::Check(cpp, checkError)) << checkError;
    model->getSimulation()->start();

    std::ifstream input(observedValue);
    double value = 0.0;
    ASSERT_TRUE(input >> value);
    EXPECT_DOUBLE_EQ(value, 42.0);
    auto* counter = dynamic_cast<Counter*>(
        model->getDataManager()->getDataDefinition("Counter", "dispose.CountNumberIn"));
    ASSERT_NE(counter, nullptr);
    EXPECT_DOUBLE_EQ(counter->getCountValue(), 1.0);
}

TEST(CppForGDynamicTest, ChangingUserCodeRebuildsAndReplacesLoadedLibrary) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const auto marker = scratch.path() / "changed-code.txt";
    const auto initializationMarker = scratch.path() / "initializations.txt";
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* create = new Create(model, "create");
    auto* cpp = new CppForG(model, "cppforg_changed_code");
    auto* dispose = new Dispose(model, "dispose");
    create->getConnectionManager()->insert(cpp);
    cpp->getConnectionManager()->insert(dispose);
    create->setMaxCreations(1ul);
    create->setEntitiesPerCreation(1);
    create->setTimeBetweenCreationsExpression("10");
    cpp->setIncludesCode("#include <fstream>\n");
    cpp->setOnDispatchEventCode("(void)entity;");
    cpp->setInitBetweenReplicationCode("std::ofstream stream(\"" + initializationMarker.string() +
        "\", std::ios::app); if (stream) stream << \"initialized\\n\";");
    ASSERT_TRUE(model->check());
    ASSERT_NE(cpp->getCppCompiler()->getDynamicLibraryHandler(), nullptr);

    cpp->setOnDispatchEventCode(markerCode(marker));
    std::string error;
    ASSERT_TRUE(ModelDataDefinition::Check(cpp, error)) << error;
    ASSERT_NE(cpp->getCppCompiler()->getDynamicLibraryHandler(), nullptr);

    model->getSimulation()->setNumberOfReplications(1);
    model->getSimulation()->setReplicationLength(2);
    model->getSimulation()->setShowReportsAfterReplication(false);
    model->getSimulation()->setShowReportsAfterSimulation(false);
    model->getSimulation()->start();
    EXPECT_EQ(countLines(marker), 1u);
    EXPECT_EQ(countLines(initializationMarker), 1u);
    auto* counter = dynamic_cast<Counter*>(
        model->getDataManager()->getDataDefinition("Counter", "dispose.CountNumberIn"));
    ASSERT_NE(counter, nullptr);
    EXPECT_DOUBLE_EQ(counter->getCountValue(), 1.0);

    model->getSimulation()->start();
    EXPECT_EQ(countLines(marker), 2u);
    EXPECT_EQ(countLines(initializationMarker), 2u);
    EXPECT_DOUBLE_EQ(counter->getCountValue(), 1.0);
}

TEST(CppForGDynamicTest, TwoComponentsUseIndependentGeneratedLibraries) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const auto marker1 = scratch.path() / "first.txt";
    const auto marker2 = scratch.path() / "second.txt";
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* create = new Create(model, "create");
    auto* first = new CppForG(model, "first");
    auto* second = new CppForG(model, "second");
    auto* dispose = new Dispose(model, "dispose");
    create->getConnectionManager()->insert(first);
    first->getConnectionManager()->insert(second);
    second->getConnectionManager()->insert(dispose);
    create->setMaxCreations(1ul);
    create->setEntitiesPerCreation(1);
    create->setTimeBetweenCreationsExpression("10");
    first->setIncludesCode("#include <fstream>\n");
    first->setOnDispatchEventCode(markerCode(marker1));
    second->setIncludesCode("#include <fstream>\n");
    second->setOnDispatchEventCode(markerCode(marker2));
    model->getSimulation()->setNumberOfReplications(1);
    model->getSimulation()->setReplicationLength(2);
    model->getSimulation()->setShowReportsAfterReplication(false);
    model->getSimulation()->setShowReportsAfterSimulation(false);
    ASSERT_TRUE(model->check());
    ASSERT_NE(first->getCppCompiler()->getDynamicLibraryHandler(), nullptr);
    ASSERT_NE(second->getCppCompiler()->getDynamicLibraryHandler(), nullptr);
    EXPECT_NE(first->getCppCompiler()->getDynamicLibraryHandler(),
              second->getCppCompiler()->getDynamicLibraryHandler());
    model->getSimulation()->start();
    EXPECT_EQ(countLines(marker1), 1u);
    EXPECT_EQ(countLines(marker2), 1u);
}

TEST(CppForGDynamicTest, InvalidUserCodeFailsModelCheck) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    auto* code = new CppForG(model, "bad_code");
    code->setOnDispatchEventCode("definitely invalid C++ syntax");
    std::string error;
    EXPECT_FALSE(ModelDataDefinition::Check(code, error));
    EXPECT_FALSE(error.empty());
    EXPECT_EQ(code->getCppCompiler()->getDynamicLibraryHandler(), nullptr);
}

TEST(CppForGDynamicTest, PersistsConfiguredSourceAndReusesCompilerDefinition) {
    ScopedCppForGDirectory scratch;
    ASSERT_FALSE(scratch.path().empty());
    const auto filename = scratch.path() / "configured.gen";
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    auto* code = new CppForG(model, "persisted_code");
    code->setOnDispatchEventCode("std::cout << \"arrived\";");
    code->setInitBetweenReplicationCode("std::cout << \"initialized\";");
    code->setIncludesCode("#include <iostream>\n");
    ASSERT_TRUE(model->save(filename.string()));

    Simulator restoredSimulator;
    restoredSimulator.getPluginManager()->autoInsertPlugins();
    Model* restored = restoredSimulator.getModelManager()->loadModel(filename.string());
    ASSERT_NE(restored, nullptr);
    auto* loaded = dynamic_cast<CppForG*>(restored->getComponentManager()->find("persisted_code"));
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->getOnDispatchEventCode(), code->getOnDispatchEventCode());
    EXPECT_EQ(loaded->getInitBetweenReplicationCode(), code->getInitBetweenReplicationCode());
    EXPECT_EQ(loaded->getIncludesCode(), code->getIncludesCode());
    ASSERT_NE(loaded->getCppCompiler(), nullptr);
    EXPECT_EQ(loaded->getCppCompiler()->getName(), "persisted_code.CppCompiler");
}
