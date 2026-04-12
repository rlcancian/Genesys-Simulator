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

TEST(SupportSimulationScenarioClassTest, SettersAndGettersForMetadata) {
    SimulationScenario scenario;

    scenario.setScenarioName("Scenario A");
    scenario.setScenarioDescription("Description A");
    scenario.setModelFilename("model.gen");

    EXPECT_EQ(scenario.getScenarioName(), "Scenario A");
    EXPECT_EQ(scenario.getScenarioDescription(), "Description A");
    EXPECT_EQ(scenario.getModelFilename(), "model.gen");
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

TEST(SupportSimulationScenarioClassTest, SetControlAccumulatesInsertedPairs) {
    SimulationScenario scenario;

    scenario.setControl("a", 1.0);
    scenario.setControl("b", 2.0);
    scenario.setControl("c", 3.0);

    ASSERT_NE(scenario.getControlValues(), nullptr);
    EXPECT_EQ(scenario.getControlValues()->size(), 3u);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("a"), 1.0);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("b"), 2.0);
    EXPECT_DOUBLE_EQ(scenario.getControlValue("c"), 3.0);
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

    delete controls;
}

TEST(SupportSimulationScenarioClassTest, ReplacingSelectedControlsKeepsIndependentCopy) {
    SimulationScenario scenario;

    auto* first = new std::list<std::string>({"x", "y"});
    scenario.setSelectedControls(first);
    first->clear();

    auto* second = new std::list<std::string>({"r"});
    scenario.setSelectedControls(second);
    second->push_back("s");

    ASSERT_NE(scenario.getSelectedControls(), nullptr);
    EXPECT_EQ(scenario.getSelectedControls()->size(), 1u);
    EXPECT_EQ(scenario.getSelectedControls()->front(), "r");

    delete first;
    delete second;
}

TEST(SupportSimulationScenarioClassTest, StartSimulationReturnsFalseAndClearsErrorMessage) {
    SimulationScenario scenario;
    std::string errorMessage = "previous error";

    EXPECT_FALSE(scenario.startSimulation(nullptr, errorMessage));
    EXPECT_EQ(errorMessage, "");
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
