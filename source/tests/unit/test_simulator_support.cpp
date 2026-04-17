#include <gtest/gtest.h>

#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/OnEventManager.h"
#include "kernel/simulator/ConnectionManager.h"
#include "kernel/simulator/ParserManager.h"
#include "kernel/simulator/ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/LicenceManager.h"
#include "kernel/simulator/ExperimentManager.h"
#include "kernel/simulator/ModelInfo.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/PropertyManager.h"
#include "kernel/simulator/Property.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationScenario.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/SystemDependencyResolver.h"
#include <map>
#include <type_traits>
#include <vector>


// Test-only link shim:
// ExperimentManager.cpp references Simulator::getTraceManager() const
// in methods that are not exercised by this unit test target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}



Model::Model(Simulator* simulator, unsigned int level) {
    (void)simulator;
    (void)level;
}

// Provides a trivial out-of-line destructor for the test double after Model gained an explicit virtual destructor.
Model::~Model() = default;

void ModelDataDefinition::CreateInternalData(ModelDataDefinition*) {
}

bool Model::save(std::string filename) {
    (void)filename;
    return true;
}

bool Model::load(std::string filename) {
    (void)filename;
    return true;
}


PluginInformation* BuildTestComponentPluginInfo() {
    auto* info = new PluginInformation("TestComponent", static_cast<StaticLoaderComponentInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
    info->setCategory("Test");
    info->setSource(true);
    info->setGenerateReport(true);
    info->insertDynamicLibFileDependence("depA.so");
    info->getFields()->insert({"fieldA", ""});
    info->setLanguageTemplate("template-body ");
    info->setDescriptionHelp("help-text");
    return info;
}

PluginInformation* BuildTestElementPluginInfo() {
    auto* info = new PluginInformation("TestElement", static_cast<StaticLoaderDataDefinitionInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));
    info->setCategory("Test");
    return info;
}

PluginInformation* GetTestComponentPluginInformation() {
    return BuildTestComponentPluginInfo();
}

PluginInformation* GetTestElementPluginInformation() {
    return BuildTestElementPluginInfo();
}

PluginInformation* ThrowingPluginInformationFactory() {
    throw 42;
}


class TestSimulationResponse : public SimulationResponse {
public:
    explicit TestSimulationResponse(std::function<std::string()> getter, bool isEnum = false)
        : SimulationResponse("TestClass", "TestElement", "TestResponse", "", false, false, isEnum),
          _getter(std::move(getter)) {
    }

    std::string getValue() const override {
        return _getter();
    }

private:
    std::function<std::string()> _getter;
};

class TestSimulationControl : public SimulationControl {
public:
    TestSimulationControl(std::function<std::string()> getter, std::function<void(std::string)> setter)
        : SimulationControl("TestClass", "TestElement", "TestControl"),
          _getter(std::move(getter)),
          _setter(std::move(setter)) {
        _readonly = !_setter;
    }

    std::string getValue() const override {
        return _getter();
    }

    void setValue(std::string value, bool remove = false) override {
        (void)remove;
        _setter(value);
    }

private:
    std::function<std::string()> _getter;
    std::function<void(std::string)> _setter;
};

struct KernelAccessorProbe {
    std::string name = "alpha";

    std::string getName() const {
        return name;
    }

    void setName(std::string newName) {
        name = newName;
    }
};

class FakeModelPersistence : public ModelPersistence_if {
public:
    bool save(std::string) override { return false; }
    bool load(std::string) override { return false; }
    bool hasChanged() override { return false; }
    void setHasChanged(bool) override {}
    bool getOption(ModelPersistence_if::Options) override { return false; }
    void setOption(ModelPersistence_if::Options, bool) override {}
    std::string getFormatedField(PersistenceRecord*) override { return ""; }
};

class FakeSystemCommandExecutor : public SystemCommandExecutor_if {
public:
    std::map<std::string, SystemCommandResult> results;
    std::vector<std::string> commands;

    SystemCommandResult run(const std::string& command) override {
        commands.push_back(command);
        auto it = results.find(command);
        if (it != results.end()) {
            return it->second;
        }
        return {};
    }
};

SystemCommandResult CommandResultWithExitCode(int exitCode) {
    SystemCommandResult result;
    result.started = true;
    result.exitCode = exitCode;
    return result;
}

SystemDependency::OS NonHostOS() {
    switch (SystemDependencyResolver::currentOS()) {
        case SystemDependency::OS::Linux:
            return SystemDependency::OS::Windows;
        case SystemDependency::OS::Windows:
            return SystemDependency::OS::Linux;
        case SystemDependency::OS::MacOS:
            return SystemDependency::OS::Linux;
        case SystemDependency::OS::Any:
        case SystemDependency::OS::Unknown:
            return SystemDependency::OS::Linux;
    }
    return SystemDependency::OS::Linux;
}



// TraceManager class-focused tests moved to test_support_tracemanager.cpp

TEST(SimulatorSupportTest, ParserManagerCanBeConstructed) {
    ParserManager pm;
    SUCCEED();
}

TEST(SimulatorSupportTest, LicenceManagerCanBeConstructedWithoutSimulatorUse) {
    LicenceManager lm(nullptr);
    EXPECT_FALSE(lm.showLicence().empty());
}

TEST(SimulatorSupportTest, DefaultActivationCodeReportsNotFound) {
    LicenceManager lm(nullptr);
    EXPECT_EQ(lm.showActivationCode(), "ACTIVATION CODE: Not found.");
}

TEST(SimulatorSupportTest, LicenceManagerDefaultLimitsAndResetAreStable) {
    LicenceManager lm(nullptr);

    EXPECT_EQ(lm.getModelComponentsLimit(), 100u);
    EXPECT_EQ(lm.getModelDatasLimit(), 300u);
    EXPECT_EQ(lm.getEntityLimit(), 300u);
    EXPECT_EQ(lm.getHostsLimit(), 1u);
    EXPECT_EQ(lm.getThreadsLimit(), 1u);
    EXPECT_NE(lm.showLimits().find("100 components"), std::string::npos);
    EXPECT_NE(lm.showLimits().find("300 elements"), std::string::npos);
    EXPECT_FALSE(lm.insertActivationCode());
    EXPECT_FALSE(lm.lookforActivationCode());

    lm.removeActivationCode();

    EXPECT_EQ(lm.showActivationCode(), "ACTIVATION CODE: Not found.");
    EXPECT_EQ(lm.getModelComponentsLimit(), 100u);
}

// ExperimentManager class-focused tests moved to test_support_experimentmanager.cpp

TEST(SimulatorSupportTest, ModelInfoStartsMarkedAsUnchanged) {
    ModelInfo info;
    EXPECT_FALSE(info.hasChanged());
    EXPECT_FALSE(info.getName().empty());
    EXPECT_EQ(info.getAnalystName(), "");
    EXPECT_EQ(info.getDescription(), "");
    EXPECT_EQ(info.getProjectTitle(), "");
    EXPECT_EQ(info.getVersion(), "1.0");
}

TEST(SimulatorSupportTest, ModelInfoSettersMarkObjectAsChanged) {
    ModelInfo info;
    info.setAnalystName("Analyst");
    EXPECT_TRUE(info.hasChanged());
    EXPECT_EQ(info.getAnalystName(), "Analyst");
}

TEST(SimulatorSupportTest, ModelInfoSaveAndLoadRoundTrip) {
    FakeModelPersistence persistence;
    PersistenceRecord fields(persistence);

    ModelInfo saved;
    saved.setName("Model_X");
    saved.setAnalystName("Analyst_Y");
    saved.setDescription("Description_Z");
    saved.setProjectTitle("Project_W");
    saved.setVersion("2.5");

    saved.saveInstance(&fields);
    EXPECT_FALSE(saved.hasChanged());

    ModelInfo loaded;
    loaded.loadInstance(&fields);

    EXPECT_EQ(loaded.getName(), "Model_X");
    EXPECT_EQ(loaded.getAnalystName(), "Analyst_Y");
    EXPECT_EQ(loaded.getDescription(), "Description_Z");
    EXPECT_EQ(loaded.getProjectTitle(), "Project_W");
    EXPECT_EQ(loaded.getVersion(), "2.5");
    EXPECT_FALSE(loaded.hasChanged());
}

TEST(SimulatorSupportTest, ModelInfoShowReflectsConfiguredFields) {
    ModelInfo info;
    info.setName("Model_S");
    info.setAnalystName("Analyst_S");
    info.setDescription("Desc_S");
    info.setVersion("9.8");

    const std::string shown = info.show();
    EXPECT_NE(shown.find("analystName=\"Analyst_S\""), std::string::npos);
    EXPECT_NE(shown.find("description=\"Desc_S\""), std::string::npos);
    EXPECT_NE(shown.find("name=\"Model_S\""), std::string::npos);
    EXPECT_NE(shown.find("version=9.8"), std::string::npos);
}

// ModelManager class-focused tests moved to test_support_modelmanager.cpp

// OnEventManager class-focused tests moved to test_support_oneventmanager.cpp

// ConnectionManager class-focused tests moved to test_support_connectionmanager.cpp

TEST(SimulatorSupportTest, ParserChangesInformationStartsEmpty) {
    ParserChangesInformation info;

    EXPECT_EQ(info.getincludes(), "");
    EXPECT_EQ(info.gettokens(), "");
    EXPECT_EQ(info.gettypeObjs(), "");
    EXPECT_EQ(info.getexpressions(), "");
    EXPECT_EQ(info.getexpressionProductions(), "");
    EXPECT_EQ(info.getassignments(), "");
    EXPECT_EQ(info.getfunctionProdutions(), "");
}

TEST(SimulatorSupportTest, ParserChangesInformationStoresAllConfiguredSections) {
    ParserChangesInformation info;

    info.setIncludes("inc");
    info.setTokens("tok");
    info.setTypeObjs("types");
    info.setExpressions("expr");
    info.setExpressionProductions("prod");
    info.setAssignments("assign");
    info.setFunctionProdutions("func");

    EXPECT_EQ(info.getincludes(), "inc");
    EXPECT_EQ(info.gettokens(), "tok");
    EXPECT_EQ(info.gettypeObjs(), "types");
    EXPECT_EQ(info.getexpressions(), "expr");
    EXPECT_EQ(info.getexpressionProductions(), "prod");
    EXPECT_EQ(info.getassignments(), "assign");
    EXPECT_EQ(info.getfunctionProdutions(), "func");
}

TEST(SimulatorSupportTest, ParserChangesInformationSupportsMultilineAndOverwrite) {
    ParserChangesInformation info;

    info.setIncludes("#include <x>\n#include <y>");
    info.setTokens("TOK_A TOK_B");
    info.setIncludes("just-one");
    info.setFunctionProdutions("f1\nf2");

    EXPECT_EQ(info.getincludes(), "just-one");
    EXPECT_EQ(info.gettokens(), "TOK_A TOK_B");
    EXPECT_EQ(info.getfunctionProdutions(), "f1\nf2");
}

TEST(SimulatorSupportTest, PropertyManagerCanBeConstructed) {
    PropertyManager manager;
    SUCCEED();
}

TEST(SimulatorSupportTest, PluginInformationComponentConstructorConfiguresComponentMode) {
    PluginInformation info("Create", static_cast<StaticLoaderComponentInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));

    EXPECT_TRUE(info.isComponent());
    EXPECT_EQ(info.getPluginTypename(), "Create");
    EXPECT_EQ(info.GetComponentLoader(), nullptr);
    EXPECT_EQ(info.getDataDefinitionLoader(), nullptr);
    EXPECT_EQ(info.getDataDefinitionConstructor(), nullptr);
}

TEST(SimulatorSupportTest, PluginInformationDataDefinitionConstructorConfiguresElementMode) {
    PluginInformation info("Queue", static_cast<StaticLoaderDataDefinitionInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));

    EXPECT_FALSE(info.isComponent());
    EXPECT_EQ(info.getPluginTypename(), "Queue");
    EXPECT_EQ(info.GetComponentLoader(), nullptr);
    EXPECT_EQ(info.getDataDefinitionLoader(), nullptr);
    EXPECT_EQ(info.getDataDefinitionConstructor(), nullptr);
}

TEST(SimulatorSupportTest, PluginInformationStoresMetadataAndLimits) {
    PluginInformation info("Delay", static_cast<StaticLoaderComponentInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));

    info.setGenerateReport(true);
    info.setSendTransfer(true);
    info.setReceiveTransfer(true);
    info.setSink(true);
    info.setSource(true);
    info.setObservation("obs");
    info.setVersion("1.2.3");
    info.setDate("2026-03-29");
    info.setAuthor("author");
    info.setMaximumOutputs(5);
    info.setMinimumOutputs(2);
    info.setMaximumInputs(4);
    info.setMinimumInputs(1);
    info.setDescriptionHelp("help");
    info.setLanguageTemplate("template");
    info.setCategory("category");
    info.insertDynamicLibFileDependence("libA.so");

    EXPECT_TRUE(info.isGenerateReport());
    EXPECT_TRUE(info.isSendTransfer());
    EXPECT_TRUE(info.isReceiveTransfer());
    EXPECT_TRUE(info.isSink());
    EXPECT_TRUE(info.isSource());
    EXPECT_EQ(info.getObservation(), "obs");
    EXPECT_EQ(info.getVersion(), "1.2.3");
    EXPECT_EQ(info.getDate(), "2026-03-29");
    EXPECT_EQ(info.getAuthor(), "author");
    EXPECT_EQ(info.getMaximumOutputs(), 5u);
    EXPECT_EQ(info.getMinimumOutputs(), 2u);
    EXPECT_EQ(info.getMaximumInputs(), 4u);
    EXPECT_EQ(info.getMinimumInputs(), 1u);
    EXPECT_EQ(info.getDescriptionHelp(), "help");
    EXPECT_EQ(info.getLanguageTemplate(), "template");
    EXPECT_EQ(info.getCategory(), "category");
    ASSERT_NE(info.getDynamicLibFilenameDependencies(), nullptr);
    EXPECT_EQ(info.getDynamicLibFilenameDependencies()->size(), 1u);
}

TEST(SimulatorSupportTest, SystemDependencyStoresDeclarativeCommands) {
    SystemDependency dependency(
        SystemDependency::OS::Linux,
        "libSBML",
        "sudo apt install libsbml5-dev -y",
        "pkg-config --exists libsbml");

    EXPECT_EQ(dependency.getOS(), SystemDependency::OS::Linux);
    EXPECT_EQ(dependency.getName(), "libSBML");
    EXPECT_EQ(dependency.getInstallCommand(), "sudo apt install libsbml5-dev -y");
    EXPECT_EQ(dependency.getCheckCommand(), "pkg-config --exists libsbml");
    EXPECT_EQ(SystemDependency::osToString(SystemDependency::OS::Linux), "Linux");
    EXPECT_NE(dependency.show().find("name=\"libSBML\""), std::string::npos);
    EXPECT_NE(dependency.show().find("checkCommand=\"pkg-config --exists libsbml\""), std::string::npos);
}

TEST(SimulatorSupportTest, PluginInformationStoresSystemDependenciesInOrder) {
    PluginInformation info("BioSimulatorRunner", static_cast<StaticLoaderDataDefinitionInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));

    EXPECT_FALSE(info.hasSystemDependencies());
    ASSERT_NE(info.getSystemDependencies(), nullptr);
    EXPECT_TRUE(info.getSystemDependencies()->empty());

    info.insertSystemDependency(SystemDependency(
        SystemDependency::OS::Linux,
        "libSBML",
        "sudo apt install libsbml5-dev -y",
        "pkg-config --exists libsbml"));
    info.insertSystemDependency(SystemDependency(
        SystemDependency::OS::Linux,
        "COPASI",
        "sudo apt install copasi -y",
        "CopasiSE --version"));

    ASSERT_TRUE(info.hasSystemDependencies());
    ASSERT_NE(info.getSystemDependencies(), nullptr);
    ASSERT_EQ(info.getSystemDependencies()->size(), 2u);

    auto it = info.getSystemDependencies()->begin();
    EXPECT_EQ(it->getName(), "libSBML");
    ++it;
    EXPECT_EQ(it->getName(), "COPASI");
}

TEST(SimulatorSupportTest, SystemDependencyResolverMarksSatisfiedDependency) {
    FakeSystemCommandExecutor executor;
    executor.results["pkg-config --exists libsbml"] = CommandResultWithExitCode(0);
    std::list<SystemDependency> dependencies;
    dependencies.emplace_back(
        SystemDependency::OS::Any,
        "libSBML",
        "sudo apt install libsbml5-dev -y",
        "pkg-config --exists libsbml");

    const SystemDependencyCheckResult result = SystemDependencyResolver::evaluate(&dependencies, executor);

    ASSERT_EQ(result.entries().size(), 1u);
    EXPECT_TRUE(result.canInsertPlugin());
    EXPECT_EQ(result.entries().front().status(), SystemDependencyCheckEntry::Status::Satisfied);
    ASSERT_EQ(executor.commands.size(), 1u);
    EXPECT_EQ(executor.commands.front(), "pkg-config --exists libsbml");
}

TEST(SimulatorSupportTest, SystemDependencyResolverMarksMissingDependency) {
    FakeSystemCommandExecutor executor;
    SystemCommandResult failedCheck = CommandResultWithExitCode(1);
    failedCheck.output = "missing-tool: command not found\n";
    executor.results["missing-tool --version"] = failedCheck;
    std::list<SystemDependency> dependencies;
    dependencies.emplace_back(
        SystemDependency::OS::Any,
        "MissingTool",
        "sudo apt install missing-tool -y",
        "missing-tool --version");

    const SystemDependencyCheckResult result = SystemDependencyResolver::evaluate(&dependencies, executor);

    ASSERT_EQ(result.entries().size(), 1u);
    EXPECT_FALSE(result.canInsertPlugin());
    EXPECT_TRUE(result.hasBlockingEntries());
    EXPECT_TRUE(result.canAttemptInstallForAllMissing());
    EXPECT_EQ(result.entries().front().status(), SystemDependencyCheckEntry::Status::Missing);
    EXPECT_NE(result.diagnosticText(false).find("Check command: missing-tool --version"), std::string::npos);
    EXPECT_NE(result.diagnosticText(false).find("Install command: sudo apt install missing-tool -y"), std::string::npos);
    EXPECT_NE(result.diagnosticText(false).find("missing-tool: command not found"), std::string::npos);
}

TEST(SimulatorSupportTest, SystemDependencyResolverIgnoresDifferentOperatingSystem) {
    FakeSystemCommandExecutor executor;
    std::list<SystemDependency> dependencies;
    dependencies.emplace_back(
        NonHostOS(),
        "OtherOSTool",
        "install-other-os-tool",
        "other-os-tool --version");

    const SystemDependencyCheckResult result = SystemDependencyResolver::evaluate(&dependencies, executor);

    ASSERT_EQ(result.entries().size(), 1u);
    EXPECT_TRUE(result.canInsertPlugin());
    EXPECT_EQ(result.entries().front().status(), SystemDependencyCheckEntry::Status::IgnoredDifferentOS);
    EXPECT_TRUE(executor.commands.empty());
}

TEST(SimulatorSupportTest, SystemDependencyResolverMarksApplicableDependencyWithoutCheckCommandAsNotVerifiable) {
    FakeSystemCommandExecutor executor;
    std::list<SystemDependency> dependencies;
    dependencies.emplace_back(
        SystemDependency::OS::Any,
        "ManualOnlyTool",
        "sudo apt install manual-only-tool -y",
        "");

    const SystemDependencyCheckResult result = SystemDependencyResolver::evaluate(&dependencies, executor);

    ASSERT_EQ(result.entries().size(), 1u);
    EXPECT_FALSE(result.canInsertPlugin());
    EXPECT_FALSE(result.canAttemptInstallForAllMissing());
    EXPECT_EQ(result.entries().front().status(), SystemDependencyCheckEntry::Status::NotVerifiable);
    EXPECT_TRUE(executor.commands.empty());
}

TEST(SimulatorSupportTest, SystemDependencyResolverInstallDiagnosticIncludesCommandAndOutput) {
    FakeSystemCommandExecutor executor;
    executor.results["missing-tool --version"] = CommandResultWithExitCode(1);
    SystemCommandResult failedInstall = CommandResultWithExitCode(100);
    failedInstall.output = "package not found\n";
    failedInstall.errorMessage = "apt failed\n";
    executor.results["sudo apt install missing-tool -y"] = failedInstall;

    std::list<SystemDependency> dependencies;
    dependencies.emplace_back(
        SystemDependency::OS::Any,
        "MissingTool",
        "sudo apt install missing-tool -y",
        "missing-tool --version");

    const SystemDependencyCheckResult check = SystemDependencyResolver::evaluate(&dependencies, executor);
    const SystemDependencyInstallResult result = SystemDependencyResolver::installMissingDependencies(check, executor);
    const std::string diagnostic = result.diagnosticText();

    EXPECT_FALSE(result.succeeded());
    EXPECT_NE(diagnostic.find("Dependency: MissingTool"), std::string::npos);
    EXPECT_NE(diagnostic.find("Install command: sudo apt install missing-tool -y"), std::string::npos);
    EXPECT_NE(diagnostic.find("Install exit code: 100"), std::string::npos);
    EXPECT_NE(diagnostic.find("package not found"), std::string::npos);
    EXPECT_NE(diagnostic.find("apt failed"), std::string::npos);
    ASSERT_EQ(executor.commands.size(), 2u);
    EXPECT_EQ(executor.commands.at(0), "missing-tool --version");
    EXPECT_EQ(executor.commands.at(1), "sudo apt install missing-tool -y");
}

TEST(SimulatorSupportTest, PluginInformationDefaultsAndContainerReplacementWork) {
    PluginInformation info("Delay", static_cast<StaticLoaderComponentInstance>(nullptr), static_cast<StaticConstructorDataDefinitionInstance>(nullptr));

    EXPECT_EQ(info.getCategory(), "Discrete Processing");
    EXPECT_FALSE(info.isGenerateReport());
    EXPECT_FALSE(info.isSource());
    EXPECT_FALSE(info.isSink());
    EXPECT_EQ(info.getMinimumInputs(), 1u);
    EXPECT_EQ(info.getMaximumInputs(), 1u);
    EXPECT_EQ(info.getMinimumOutputs(), 1u);
    EXPECT_EQ(info.getMaximumOutputs(), 1u);

    auto* deps = new std::list<std::string>{"dep1.so", "dep2.so"};
    auto* fields = new std::map<std::string, std::string>{{"k1", "v1"}, {"k2", "v2"}};
    info.setDynamicLibFilenameDependencies(deps);
    info.setFields(fields);

    ASSERT_NE(info.getDynamicLibFilenameDependencies(), nullptr);
    EXPECT_EQ(info.getDynamicLibFilenameDependencies()->size(), 2u);
    ASSERT_NE(info.getFields(), nullptr);
    EXPECT_EQ(info.getFields()->at("k1"), "v1");
    EXPECT_EQ(info.getFields()->at("k2"), "v2");
}

TEST(SimulatorSupportTest, PluginConstructedFromFactoryCanExposePluginInformation) {
    Plugin plugin(&GetTestComponentPluginInformation);

    ASSERT_TRUE(plugin.isIsValidPlugin());
    ASSERT_NE(plugin.getPluginInfo(), nullptr);
    EXPECT_TRUE(plugin.getPluginInfo()->isComponent());
    EXPECT_EQ(plugin.getPluginInfo()->getPluginTypename(), "TestComponent");
}

TEST(SimulatorSupportTest, PluginShowIncludesConfiguredMetadata) {
    Plugin plugin(&GetTestComponentPluginInformation);

    const std::string text = plugin.show();

    EXPECT_NE(text.find("<Test>"), std::string::npos);
    EXPECT_NE(text.find("Source"), std::string::npos);
    EXPECT_NE(text.find("GenerateReport"), std::string::npos);
    EXPECT_NE(text.find("\"TestComponent\""), std::string::npos);
    EXPECT_NE(text.find("depA.so"), std::string::npos);
    EXPECT_NE(text.find("fieldA"), std::string::npos);
    EXPECT_NE(text.find("help-text"), std::string::npos);
}

TEST(SimulatorSupportTest, PluginCanRepresentElementPluginKind) {
    Plugin plugin(&GetTestElementPluginInformation);

    ASSERT_TRUE(plugin.isIsValidPlugin());
    ASSERT_NE(plugin.getPluginInfo(), nullptr);
    EXPECT_FALSE(plugin.getPluginInfo()->isComponent());
    const std::string text = plugin.show();
    EXPECT_NE(text.find("Element"), std::string::npos);
    EXPECT_NE(text.find("\"TestElement\""), std::string::npos);
}

TEST(SimulatorSupportTest, PluginMarksFactoryFailureAsInvalid) {
    Plugin plugin(&ThrowingPluginInformationFactory);

    EXPECT_FALSE(plugin.isIsValidPlugin());
}

// PersistenceRecord class-focused tests moved to test_support_persistence.cpp

// ParserManager class-focused tests moved to test_support_parsermanager.cpp

// SimulationScenario class-focused tests moved to test_support_simulationscenario.cpp

// ExperimentManager class-focused tests moved to test_support_experimentmanager.cpp

TEST(SimulatorSupportTest, SimulationResponseProvidesReadOnlyKernelAccess) {
    std::string value = "alpha";
    TestSimulationResponse response([&]() { return value; }, true);

    EXPECT_EQ(response.getValue(), "alpha");
    EXPECT_TRUE(response.getIsEnum());
    EXPECT_EQ(response.getName(), "TestResponse");
}

TEST(SimulatorSupportTest, SimulationControlInheritsReadPathAndAddsWritePath) {
    std::string value = "alpha";
    TestSimulationControl control(
        [&]() { return value; },
        [&](std::string newValue) { value = newValue; }
    );

    EXPECT_FALSE(control.isReadOnly());
    EXPECT_EQ(control.getValue(), "alpha");

    control.setValue("beta");

    EXPECT_EQ(value, "beta");
    EXPECT_EQ(control.getValue(), "beta");
}

TEST(SimulatorSupportTest, SimulationControlStringReadOnlyRejectsWrites) {
    std::string value = "alpha";
    SimulationControlString control(
        [&]() { return value; },
        nullptr,
        "C",
        "E",
        "P"
    );

    EXPECT_TRUE(control.isReadOnly());
    EXPECT_THROW(control.setValue("beta"), std::logic_error);
    EXPECT_EQ(control.getValue(), "alpha");
}

TEST(SimulatorSupportTest, SimulationControlBoolParsesTextAndNumericValues) {
    bool value = false;
    SimulationControlBool control(
        [&]() { return value; },
        [&](bool newValue) { value = newValue; },
        "C",
        "E",
        "B"
    );

    control.setValue("true");
    EXPECT_TRUE(value);
    control.setValue("0");
    EXPECT_FALSE(value);
}

TEST(SimulatorSupportTest, DefineSimulationGetterAndSetterBindKernelMethods) {
    KernelAccessorProbe probe;

    auto getter = DefineSimulationGetter(&probe, &KernelAccessorProbe::getName);
    auto setter = DefineSimulationSetter(&probe, &KernelAccessorProbe::setName);

    TestSimulationControl control(getter, setter);

    EXPECT_EQ(control.getValue(), "alpha");

    control.setValue("gamma");

    EXPECT_EQ(probe.name, "gamma");
    EXPECT_EQ(control.getValue(), "gamma");
}

TEST(SimulatorSupportTest, ModelDataDefinitionGetPropertiesNowReturnsSimulationControlList) {
    using ReturnType = decltype(std::declval<const ModelDataDefinition*>()->getProperties());
    constexpr bool is_expected = std::is_same_v<ReturnType, List<SimulationControl*>*>;
    EXPECT_TRUE(is_expected);
}

TEST(SimulatorSupportTest, ModelGetResponsesNowReturnsSimulationResponseList) {
    using ReturnType = decltype(std::declval<const Model*>()->getResponses());
    constexpr bool is_expected = std::is_same_v<ReturnType, List<SimulationResponse*>*>;
    EXPECT_TRUE(is_expected);
}

TEST(SimulatorSupportTest, ModelGetControlsStillReturnSimulationControlList) {
    using ReturnType = decltype(std::declval<const Model*>()->getControls());
    constexpr bool is_expected = std::is_same_v<ReturnType, List<SimulationControl*>*>;
    EXPECT_TRUE(is_expected);
}

TEST(SimulatorSupportTest, SimulationResponseDoubleIsNotAWritableControl) {
    double value = 2.5;
    SimulationResponseDouble response(
        [&]() { return value; },
        "TestClass",
        "TestElement",
        "Metric"
    );

    EXPECT_EQ(response.getValue(), std::to_string(2.5));

    SimulationResponse* base = &response;
    EXPECT_EQ(dynamic_cast<SimulationControl*>(base), nullptr);
}

TEST(SimulatorSupportTest, LegacyPropertyBaseCanCoexistWithKernelPropertyBaseAlias) {
    PropertyT<int> property(
        "LegacyPropertyClass",
        "LegacyValue",
        []() { return 7; },
        [](int) {}
    );

    EXPECT_EQ(property.getClassname(), "LegacyPropertyClass");
    EXPECT_EQ(property.getName(), "LegacyValue");

    SimulationControlInt control(
        []() { return 3; },
        [](int) {},
        "KernelControlClass",
        "KernelElement",
        "KernelValue"
    );

    SimulationResponse* response = &control;
    EXPECT_NE(response, nullptr);
}
