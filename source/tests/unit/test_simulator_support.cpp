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
#include <type_traits>


// Test-only link shim:
// ExperimentManager.cpp references Simulator::getTraceManager() const
// in methods that are not exercised by this unit test target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}



namespace {
TraceManager::Level g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;

void CaptureTraceErrorLevel(TraceErrorEvent e) {
    g_last_trace_error_level = e.getTracelevel();
}
}



Model::Model(Simulator* simulator, unsigned int level) {
    (void)simulator;
    (void)level;
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
    bool getOption(ModelPersistence_if::Options) override { return false; }
    void setOption(ModelPersistence_if::Options, bool) override {}
    std::string getFormatedField(PersistenceRecord*) override { return ""; }
};



TEST(SimulatorSupportTest, TraceManagerInitializesErrorMessagesList) {
    TraceManager tm(nullptr);
    ASSERT_NE(tm.errorMessages(), nullptr);
}

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

TEST(SimulatorSupportTest, ExperimentManagerStartsWithoutCurrentExperiment) {
    ExperimentManager em(nullptr);
    EXPECT_EQ(em.current(), nullptr);
}

TEST(SimulatorSupportTest, NewSimulationExperimentBecomesCurrent) {
    ExperimentManager em(nullptr);
    SimulationExperiment* exp = em.newSimulationExperiment();
    ASSERT_NE(exp, nullptr);
    EXPECT_EQ(em.current(), exp);
}

TEST(SimulatorSupportTest, ModelInfoStartsMarkedAsUnchanged) {
    ModelInfo info;
    EXPECT_FALSE(info.hasChanged());
    EXPECT_FALSE(info.getName().empty());
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

// ModelManager class-focused tests moved to test_support_modelmanager.cpp

// OnEventManager class-focused tests moved to test_support_oneventmanager.cpp

TEST(SimulatorSupportTest, TraceManagerTraceErrorPreservesExplicitLevel) {
    TraceManager tm(nullptr);
    tm.addTraceErrorHandler(&CaptureTraceErrorLevel);

    g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;
    tm.traceError("fatal", TraceManager::Level::L1_errorFatal);

    EXPECT_EQ(g_last_trace_error_level, TraceManager::Level::L1_errorFatal);
}

TEST(SimulatorSupportTest, ConnectionManagerStartsEmpty) {
    ConnectionManager manager;
    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(manager.getFrontConnection(), nullptr);
    EXPECT_EQ(manager.getConnectionAtPort(0), nullptr);
}

TEST(SimulatorSupportTest, ConnectionManagerInsertCreatesConnectionAtPortZero) {
    ConnectionManager manager;

    manager.insert(nullptr, 3);

    ASSERT_EQ(manager.size(), 1u);
    Connection* conn = manager.getFrontConnection();
    ASSERT_NE(conn, nullptr);
    EXPECT_EQ(conn->component, nullptr);
    EXPECT_EQ(conn->channel.portNumber, 3u);
}

TEST(SimulatorSupportTest, ConnectionManagerRemoveAtPortClearsInsertedConnection) {
    ConnectionManager manager;
    manager.insert(nullptr, 7);

    ASSERT_EQ(manager.size(), 1u);
    manager.removeAtPort(0);

    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(manager.getConnectionAtPort(0), nullptr);
}

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
}

TEST(SimulatorSupportTest, PluginMarksFactoryFailureAsInvalid) {
    Plugin plugin(&ThrowingPluginInformationFactory);

    EXPECT_FALSE(plugin.isIsValidPlugin());
}

// PersistenceRecord class-focused tests moved to test_support_persistence.cpp

// ParserManager class-focused tests moved to test_support_parsermanager.cpp

TEST(SimulatorSupportTest, SimulationScenarioStartsWithEmptyNamesAndLists) {
    SimulationScenario scenario;

    EXPECT_EQ(scenario.getScenarioName(), "");
    EXPECT_EQ(scenario.getScenarioDescription(), "");
    EXPECT_EQ(scenario.getModelFilename(), "");
    ASSERT_NE(scenario.getSelectedControls(), nullptr);
    ASSERT_NE(scenario.getSelectedResponses(), nullptr);
    ASSERT_NE(scenario.getControlValues(), nullptr);
    EXPECT_EQ(scenario.getSelectedControls()->size(), 0u);
    EXPECT_EQ(scenario.getSelectedResponses()->size(), 0u);
    EXPECT_EQ(scenario.getControlValues()->size(), 0u);
}

TEST(SimulatorSupportTest, SimulationScenarioStoresControlValuesSafely) {
    SimulationScenario scenario;

    scenario.setControl("replications", 10.0);
    scenario.setControl("length", 25.5);

    ASSERT_NE(scenario.getControlValues(), nullptr);
    EXPECT_EQ(scenario.getControlValues()->size(), 2u);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("replications"), 10.0);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("length"), 25.5);
}

TEST(SimulatorSupportTest, SimulationScenarioCopiesSelectedControlsList) {
    SimulationScenario scenario;
    auto* controls = new std::list<std::string>();
    controls->push_back("a");
    controls->push_back("b");

    scenario.setSelectedControls(controls);
    controls->push_back("c");

    ASSERT_NE(scenario.getSelectedControls(), nullptr);
    EXPECT_EQ(scenario.getSelectedControls()->size(), 2u);
}

TEST(SimulatorSupportTest, SimulationScenarioThrowsForMissingControlValue) {
    SimulationScenario scenario;
    EXPECT_THROW(scenario.getControlValue("missing"), std::invalid_argument);
}

TEST(SimulatorSupportTest, ExperimentManagerInsertAndRemoveWorkWithoutSimulatorTrace) {
    ExperimentManager manager(nullptr);

    auto* first = new SimulationExperiment();
    auto* second = new SimulationExperiment();

    manager.insert(first);
    manager.insert(second);

    ASSERT_EQ(manager.size(), 2u);
    EXPECT_EQ(manager.current(), second);
    EXPECT_EQ(manager.front(), first);

    manager.remove(second);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.current(), first);

    manager.remove(first);
    EXPECT_EQ(manager.size(), 0u);
}

TEST(SimulatorSupportTest, ExperimentManagerSaveAndLoadRemainGracefullyUnimplemented) {
    ExperimentManager manager(nullptr);

    EXPECT_FALSE(manager.saveSimulationExperiment("exp.gen"));
    EXPECT_FALSE(manager.loadSimulationExperiment("exp.gen"));
}

TEST(SimulatorSupportTest, SimulationScenarioStartsWithInitializedResponseStorage) {
    SimulationScenario scenario;

    ASSERT_NE(scenario.getResponseValues(), nullptr);
    EXPECT_EQ(scenario.getResponseValues()->size(), 0u);
}

TEST(SimulatorSupportTest, SimulationScenarioThrowsForMissingResponseValue) {
    SimulationScenario scenario;
    EXPECT_THROW(scenario.getResponseValue("missing"), std::invalid_argument);
}

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

