#include <gtest/gtest.h>

#include "kernel/simulator/ExperimentManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

// Link shim for support-level target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}

namespace {

class CountingSimulationExperiment : public SimulationExperiment {
public:
    ~CountingSimulationExperiment() override {
        ++destroyedCount;
    }

    static int destroyedCount;
};

int CountingSimulationExperiment::destroyedCount = 0;

} // namespace

TEST(SupportExperimentManagerClassTest, StartsWithoutCurrentExperiment) {
    ExperimentManager manager(nullptr);

    EXPECT_EQ(manager.current(), nullptr);
}

TEST(SupportExperimentManagerClassTest, NewSimulationExperimentBecomesCurrent) {
    ExperimentManager manager(nullptr);

    SimulationExperiment* exp = manager.newSimulationExperiment();
    ASSERT_NE(exp, nullptr);
    EXPECT_EQ(manager.current(), exp);

    delete exp;
}

TEST(SupportExperimentManagerClassTest, InsertAndRemoveWorkWithoutSimulatorTrace) {
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

TEST(SupportExperimentManagerClassTest, SaveAndLoadRemainGracefullyUnimplemented) {
    ExperimentManager manager(nullptr);

    EXPECT_FALSE(manager.saveSimulationExperiment("exp.gen"));
    EXPECT_FALSE(manager.loadSimulationExperiment("exp.gen"));
}

TEST(SupportExperimentManagerClassTest, SetCurrentUpdatesCurrentPointer) {
    ExperimentManager manager(nullptr);

    auto* first = new SimulationExperiment();
    auto* second = new SimulationExperiment();
    manager.insert(first);
    manager.insert(second);

    manager.setCurrent(first);
    EXPECT_EQ(manager.current(), first);

    manager.remove(second);
    manager.remove(first);
}

TEST(SupportExperimentManagerClassTest, GetExperimentsReturnsContainerWithInsertions) {
    ExperimentManager manager(nullptr);

    ASSERT_NE(manager.getExperiments(), nullptr);
    EXPECT_EQ(manager.getExperiments()->size(), 0u);

    auto* first = new SimulationExperiment();
    auto* second = new SimulationExperiment();
    manager.insert(first);
    manager.insert(second);

    EXPECT_EQ(manager.getExperiments()->size(), 2u);

    manager.remove(second);
    manager.remove(first);
}

TEST(SupportExperimentManagerClassTest, NextNavigatesListAfterFront) {
    ExperimentManager manager(nullptr);

    auto* first = new SimulationExperiment();
    auto* second = new SimulationExperiment();
    auto* third = new SimulationExperiment();
    manager.insert(first);
    manager.insert(second);
    manager.insert(third);

    EXPECT_EQ(manager.front(), first);
    EXPECT_EQ(manager.next(), second);
    EXPECT_EQ(manager.next(), third);
    EXPECT_EQ(manager.next(), nullptr);

    manager.remove(second);
    manager.remove(third);
    manager.remove(first);
}

TEST(SupportExperimentManagerClassTest, RemoveNonCurrentPreservesCurrent) {
    ExperimentManager manager(nullptr);

    auto* first = new SimulationExperiment();
    auto* second = new SimulationExperiment();
    manager.insert(first);
    manager.insert(second);
    manager.setCurrent(second);

    manager.remove(first);

    EXPECT_EQ(manager.size(), 1u);
    EXPECT_EQ(manager.current(), second);

    manager.remove(second);
}

TEST(SupportExperimentManagerClassTest, RemoveDestroysRemovedExperimentOwnership) {
    CountingSimulationExperiment::destroyedCount = 0;
    ExperimentManager manager(nullptr);

    auto* counted = new CountingSimulationExperiment();
    manager.insert(counted);

    manager.remove(counted);

    EXPECT_EQ(CountingSimulationExperiment::destroyedCount, 1);
}

TEST(SupportExperimentManagerClassTest, ManagerDestructorDestroysRemainingExperiments) {
    CountingSimulationExperiment::destroyedCount = 0;

    {
        ExperimentManager manager(nullptr);
        manager.insert(new CountingSimulationExperiment());
        manager.insert(new CountingSimulationExperiment());
    }

    EXPECT_EQ(CountingSimulationExperiment::destroyedCount, 2);
}
