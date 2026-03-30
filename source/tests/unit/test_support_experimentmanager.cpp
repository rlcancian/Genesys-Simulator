#include <gtest/gtest.h>

#include "kernel/simulator/ExperimentManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

// Link shim for support-level target.
TraceManager* Simulator::getTraceManager() const {
    return nullptr;
}

TEST(SupportExperimentManagerClassTest, StartsWithoutCurrentExperiment) {
    ExperimentManager manager(nullptr);

    EXPECT_EQ(manager.current(), nullptr);
}

TEST(SupportExperimentManagerClassTest, NewSimulationExperimentBecomesCurrent) {
    ExperimentManager manager(nullptr);

    SimulationExperiment* exp = manager.newSimulationExperiment();
    ASSERT_NE(exp, nullptr);
    EXPECT_EQ(manager.current(), exp);
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
