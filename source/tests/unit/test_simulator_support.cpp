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
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"


// Test-only link shim:
// ExperimentManager.cpp references Simulator::getTraceManager() const
// in methods that are not exercised by this unit test target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}



namespace {
int g_fake_model_construction_count = 0;
}


namespace {
int g_replication_start_calls = 0;

void CountReplicationStart(SimulationEvent*) {
    ++g_replication_start_calls;
}


namespace {
TraceManager::Level g_last_trace_error_level = TraceManager::Level::L9_mostDetailed;

void CaptureTraceErrorLevel(TraceErrorEvent e) {
    g_last_trace_error_level = e.getTracelevel();
}
}



namespace {
int g_model_check_success_calls = 0;

void CountModelCheckSuccess(ModelEvent*) {
    ++g_model_check_success_calls;
}

struct ModelEventObserver {
    int calls = 0;

    void OnModelCheckSuccess(ModelEvent*) {
        ++calls;
    }
};
}


struct SimulationEventObserver {
    int calls = 0;

    void OnReplicationStart(SimulationEvent*) {
        ++calls;
    }
};
}


Model::Model(Simulator* simulator, unsigned int level) {
    (void)simulator;
    (void)level;
    ++g_fake_model_construction_count;
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

TEST(SimulatorSupportTest, ModelManagerStartsWithoutCurrentModel) {
    ModelManager manager(nullptr);
    EXPECT_EQ(manager.current(), nullptr);
    EXPECT_EQ(manager.size(), 0u);
}

TEST(SimulatorSupportTest, ModelManagerNewModelSetsCurrentWithoutInsertion) {
    const int before = g_fake_model_construction_count;

    ModelManager manager(nullptr);
    Model* model = manager.newModel();

    ASSERT_NE(model, nullptr);
    EXPECT_EQ(manager.current(), model);
    EXPECT_EQ(manager.size(), 0u);
    EXPECT_EQ(g_fake_model_construction_count, before + 1);
}

TEST(SimulatorSupportTest, ModelManagerSaveModelWithoutCurrentReturnsFalse) {
    ModelManager manager(nullptr);
    EXPECT_FALSE(manager.saveModel("dummy.gen"));
}

TEST(SimulatorSupportTest, ModelManagerSaveModelUsesCurrentModel) {
    ModelManager manager(nullptr);
    manager.newModel();
    EXPECT_TRUE(manager.saveModel("dummy.gen"));
}

TEST(SimulatorSupportTest, OnEventManagerDeduplicatesFunctionHandlers) {
    g_replication_start_calls = 0;

    OnEventManager manager;
    manager.addOnReplicationStartHandler(&CountReplicationStart);
    manager.addOnReplicationStartHandler(&CountReplicationStart);

    manager.NotifyReplicationStartHandlers(nullptr);

    EXPECT_EQ(g_replication_start_calls, 1);
}

TEST(SimulatorSupportTest, OnEventManagerInvokesMethodHandlers) {
    OnEventManager manager;
    SimulationEventObserver observer;

    manager.addOnReplicationStartHandler(&observer, &SimulationEventObserver::OnReplicationStart);
    manager.NotifyReplicationStartHandlers(nullptr);

    EXPECT_EQ(observer.calls, 1);
}

TEST(SimulatorSupportTest, OnEventManagerDeduplicatesModelFunctionHandlers) {
    g_model_check_success_calls = 0;

    OnEventManager manager;
    manager.addOnModelCheckSucessHandler(&CountModelCheckSuccess);
    manager.addOnModelCheckSucessHandler(&CountModelCheckSuccess);

    manager.NotifyModelCheckSuccessHandlers(nullptr);

    EXPECT_EQ(g_model_check_success_calls, 1);
}

TEST(SimulatorSupportTest, OnEventManagerInvokesModelMethodHandlers) {
    OnEventManager manager;
    ModelEventObserver observer;

    manager.addOnModelCheckSuccessHandler(&observer, &ModelEventObserver::OnModelCheckSuccess);
    manager.NotifyModelCheckSuccessHandlers(nullptr);

    EXPECT_EQ(observer.calls, 1);
}

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

TEST(SimulatorSupportTest, PersistenceRecordSaveLoadAndEraseWorkflow) {
    FakeModelPersistence persistence;
    PersistenceRecord record(persistence);

    record.saveField("name", std::string("alpha"));
    record.saveField("count", 7u);
    record.saveField("ratio", 2.5, 0.0, true);

    EXPECT_EQ(record.size(), 3u);
    EXPECT_EQ(record.loadField("name", std::string("fallback")), "alpha");
    EXPECT_EQ(record.loadField("count", 0u), 7u);
    EXPECT_DOUBLE_EQ(record.loadField("ratio", 0.0), 2.5);

    record.erase("count");

    EXPECT_EQ(record.size(), 2u);
    EXPECT_EQ(record.loadField("count", 123u), 123u);
}

TEST(SimulatorSupportTest, PersistenceRecordInsertIteratorRangeCopiesEntries) {
    FakeModelPersistence persistence;
    PersistenceRecord source(persistence);
    PersistenceRecord target(persistence);

    source.saveField("a", std::string("one"));
    source.saveField("b", 2u);

    target.insert(source.begin(), source.end());

    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(target.loadField("a", std::string("fallback")), "one");
    EXPECT_EQ(target.loadField("b", 0u), 2u);
}

TEST(SimulatorSupportTest, PersistenceRecordClearRemovesAllEntries) {
    FakeModelPersistence persistence;
    PersistenceRecord record(persistence);

    record.saveField("x", std::string("value"));
    record.saveField("y", 9u);

    ASSERT_EQ(record.size(), 2u);
    record.clear();

    EXPECT_EQ(record.size(), 0u);
    EXPECT_EQ(record.loadField("x", std::string("missing")), "missing");
    EXPECT_EQ(record.loadField("y", 77u), 77u);
}

