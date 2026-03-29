#include <gtest/gtest.h>

#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/OnEventManager.h"
#include "kernel/simulator/ParserManager.h"
#include "kernel/simulator/LicenceManager.h"
#include "kernel/simulator/ExperimentManager.h"
#include "kernel/simulator/ModelInfo.h"
#include "kernel/simulator/ModelManager.h"
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

