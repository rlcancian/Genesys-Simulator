#include <gtest/gtest.h>

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/persistence/GenSerializer.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string>

namespace {

std::string complexTextPayload() {
    return
        "include gro;\n"
        "set ( \"dt\", 0.18 );\n"
        "set ( \"population_max\", 256 );\n"
        "\n"
        "\tprogram p() := {\n"
        "  selected : { message ( 1, tostring ( volume ) ) }\n"
        "  // source comment that belongs to the Gro payload\n"
        "  path := \"C:\\\\genesys\\\\gro\";\n"
        "  equation := \"x=y\";\n"
        "  marker := \"e\\\"still-payload\\\"\";\n"
        "};\n";
}

std::unique_ptr<PersistenceRecord> makeGroProgramRecord(GenSerializer& serializer,
                                                        const std::string& name,
                                                        const std::string& sourceCode) {
    auto fields = std::unique_ptr<PersistenceRecord>(serializer.newPersistenceRecord());
    fields->saveField("id", 143u);
    fields->saveField("typename", "GroProgram");
    fields->saveField("name", name);
    fields->saveField("sourceCode", sourceCode);
    return fields;
}

} // namespace

std::string dumpSingleTextRecord(const std::string& name, const std::string& sourceCode) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    GenSerializer writer(model);
    auto fields = makeGroProgramRecord(writer, name, sourceCode);
    EXPECT_TRUE(writer.put(name, "GroProgram", 143u, fields.get()));

    std::ostringstream serialized;
    EXPECT_TRUE(writer.dump(serialized));
    return serialized.str();
}

std::string loadSingleTextRecord(const std::string& serialized, const std::string& name) {
    Simulator simulator;
    simulator.getPluginManager()->autoInsertPlugins();
    Model* model = simulator.getModelManager()->newModel();
    GenSerializer reader(model);
    std::istringstream input(serialized);
    EXPECT_TRUE(reader.load(input));

    auto loadedFields = std::unique_ptr<PersistenceRecord>(reader.newPersistenceRecord());
    EXPECT_TRUE(reader.get(name, loadedFields.get()));
    return loadedFields->loadField("sourceCode", std::string{});
}

TEST(GenSerializerTextPersistence, ComplexTextFieldRoundTripsExactly) {
    Simulator savingSimulator;
    ASSERT_NE(savingSimulator.getPluginManager(), nullptr);
    savingSimulator.getPluginManager()->autoInsertPlugins();

    Model* savingModel = savingSimulator.getModelManager()->newModel();
    ASSERT_NE(savingModel, nullptr);

    const std::string sourceCode = complexTextPayload();
    GenSerializer writer(savingModel);
    auto fields = makeGroProgramRecord(writer, "ComplexGro", sourceCode);
    ASSERT_TRUE(writer.put("ComplexGro", "GroProgram", 143u, fields.get()));

    std::ostringstream serialized;
    ASSERT_TRUE(writer.dump(serialized));

    Simulator loadingSimulator;
    ASSERT_NE(loadingSimulator.getPluginManager(), nullptr);
    loadingSimulator.getPluginManager()->autoInsertPlugins();

    Model* loadingModel = loadingSimulator.getModelManager()->newModel();
    ASSERT_NE(loadingModel, nullptr);

    GenSerializer reader(loadingModel);
    std::istringstream input(serialized.str());
    ASSERT_TRUE(reader.load(input));

    auto loadedFields = std::unique_ptr<PersistenceRecord>(reader.newPersistenceRecord());
    ASSERT_TRUE(reader.get("ComplexGro", loadedFields.get()));
    EXPECT_EQ(loadedFields->loadField("sourceCode", std::string{}), sourceCode);
}

TEST(GenSerializerTextPersistence, SimpleLegacyQuotedTextRemainsLoadable) {
    const std::string legacy =
        R"(143 GroProgram "LegacyGro" sourceCode="C:\new\test program p() := { tick(); }" )" "\n";

    Simulator simulator;
    Model* model = simulator.getModelManager()->newModel();
    GenSerializer reader(model);
    std::istringstream input(legacy);
    ASSERT_TRUE(reader.load(input));

    auto fields = std::unique_ptr<PersistenceRecord>(reader.newPersistenceRecord());
    ASSERT_TRUE(reader.get("LegacyGro", fields.get()));
    EXPECT_EQ(fields->loadField("sourceCode", std::string{}),
              R"(C:\new\test program p() := { tick(); })");
}

TEST(GenSerializerTextPersistence, EmptyTextFieldRoundTripsExactly) {
    const std::string serialized = dumpSingleTextRecord("EmptyGro", "");
    EXPECT_EQ(loadSingleTextRecord(serialized, "EmptyGro"), "");
}

TEST(GenSerializerTextPersistence, ComplexTextUsesEscapedLiteralAndSurvivesSecondRoundTrip) {
    const std::string sourceCode = complexTextPayload();
    const std::string firstSerialization = dumpSingleTextRecord("ComplexGro2", sourceCode);

    EXPECT_NE(firstSerialization.find("sourceCode=e\""), std::string::npos);
    EXPECT_NE(firstSerialization.find("\\n"), std::string::npos);
    EXPECT_NE(firstSerialization.find("\\\"dt\\\""), std::string::npos);
    EXPECT_EQ(loadSingleTextRecord(firstSerialization, "ComplexGro2"), sourceCode);

    Simulator secondSimulator;
    secondSimulator.getPluginManager()->autoInsertPlugins();
    Model* secondModel = secondSimulator.getModelManager()->newModel();
    GenSerializer secondReader(secondModel);
    std::istringstream firstInput(firstSerialization);
    ASSERT_TRUE(secondReader.load(firstInput));

    std::ostringstream secondSerialization;
    ASSERT_TRUE(secondReader.dump(secondSerialization));
    EXPECT_EQ(loadSingleTextRecord(secondSerialization.str(), "ComplexGro2"), sourceCode);
}


TEST(GenSerializerTextPersistence, RealGroProgramModelSaveLoadRoundTripsExactly) {
    const std::string sourceCode = complexTextPayload();
    const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
    const std::filesystem::path filename =
        std::filesystem::temp_directory_path()
        / ("genesys-groprogram-roundtrip-" + std::to_string(uniqueId) + ".gen");

    struct TemporaryFileCleanup {
        std::filesystem::path path;
        ~TemporaryFileCleanup() {
            std::error_code error;
            std::filesystem::remove(path, error);
        }
    } cleanup{filename};

    Simulator savingSimulator;
    PluginManager* savingManager = savingSimulator.getPluginManager();
    ASSERT_NE(savingManager, nullptr);
    savingManager->autoInsertPlugins();
    Model* savingModel = savingSimulator.getModelManager()->newModel();
    ASSERT_NE(savingModel, nullptr);

    GroProgram* savingProgram =
        savingManager->newInstance<GroProgram>(savingModel, "GroProgram_RoundTrip");
    ASSERT_NE(savingProgram, nullptr);
    savingProgram->setSourceCode(sourceCode);

    ASSERT_TRUE(savingModel->save(filename.string()));

    Simulator loadingSimulator;
    PluginManager* loadingManager = loadingSimulator.getPluginManager();
    ASSERT_NE(loadingManager, nullptr);
    loadingManager->autoInsertPlugins();
    Model* loadingModel = loadingSimulator.getModelManager()->newModel();
    ASSERT_NE(loadingModel, nullptr);

    ASSERT_TRUE(loadingModel->load(filename.string()));
    auto* loadedProgram = dynamic_cast<GroProgram*>(
        loadingModel->getDataManager()->getDataDefinition(
            Util::TypeOf<GroProgram>(), "GroProgram_RoundTrip"));
    ASSERT_NE(loadedProgram, nullptr);
    EXPECT_EQ(loadedProgram->getSourceCode(), sourceCode);

    ASSERT_TRUE(loadingModel->save(filename.string()));
    Simulator secondLoadingSimulator;
    secondLoadingSimulator.getPluginManager()->autoInsertPlugins();
    Model* secondLoadingModel = secondLoadingSimulator.getModelManager()->newModel();
    ASSERT_NE(secondLoadingModel, nullptr);
    ASSERT_TRUE(secondLoadingModel->load(filename.string()));
    auto* secondLoadedProgram = dynamic_cast<GroProgram*>(
        secondLoadingModel->getDataManager()->getDataDefinition(
            Util::TypeOf<GroProgram>(), "GroProgram_RoundTrip"));
    ASSERT_NE(secondLoadedProgram, nullptr);
    EXPECT_EQ(secondLoadedProgram->getSourceCode(), sourceCode);
}
