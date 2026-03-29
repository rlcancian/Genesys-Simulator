#include <gtest/gtest.h>

#include "kernel/simulator/TraceManager.h"
#include "kernel/simulator/ParserManager.h"
#include "kernel/simulator/LicenceManager.h"
#include "kernel/simulator/ExperimentManager.h"
#include "kernel/simulator/ModelInfo.h"
#include "kernel/simulator/Persistence.h"
#include "kernel/simulator/Simulator.h"


// Test-only link shim:
// ExperimentManager.cpp references Simulator::getTraceManager() const
// in methods that are not exercised by this unit test target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
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

