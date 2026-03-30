#include <gtest/gtest.h>

#include "kernel/simulator/SimulationScenario.h"

TEST(SupportSimulationScenarioClassTest, StartsWithEmptyNamesAndLists) {
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

TEST(SupportSimulationScenarioClassTest, StoresControlValuesSafely) {
    SimulationScenario scenario;

    scenario.setControl("replications", 10.0);
    scenario.setControl("length", 25.5);

    ASSERT_NE(scenario.getControlValues(), nullptr);
    EXPECT_EQ(scenario.getControlValues()->size(), 2u);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("replications"), 10.0);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("length"), 25.5);
}

TEST(SupportSimulationScenarioClassTest, CopiesSelectedControlsList) {
    SimulationScenario scenario;
    auto* controls = new std::list<std::string>();
    controls->push_back("a");
    controls->push_back("b");

    scenario.setSelectedControls(controls);
    controls->push_back("c");

    ASSERT_NE(scenario.getSelectedControls(), nullptr);
    EXPECT_EQ(scenario.getSelectedControls()->size(), 2u);
}

TEST(SupportSimulationScenarioClassTest, ThrowsForMissingControlValue) {
    SimulationScenario scenario;
    EXPECT_THROW(scenario.getControlValue("missing"), std::invalid_argument);
}

TEST(SupportSimulationScenarioClassTest, StartsWithInitializedResponseStorage) {
    SimulationScenario scenario;

    ASSERT_NE(scenario.getResponseValues(), nullptr);
    EXPECT_EQ(scenario.getResponseValues()->size(), 0u);
}

TEST(SupportSimulationScenarioClassTest, ThrowsForMissingResponseValue) {
    SimulationScenario scenario;
    EXPECT_THROW(scenario.getResponseValue("missing"), std::invalid_argument);
}
