#include <gtest/gtest.h>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "tools/Optimization/OptimizerDefaultImpl1.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

namespace {

TEST(OptimizerDefaultImpl1Test, InitialStateAndReadiness) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    OptimizerDefaultImpl1 optimizer;
    
    EXPECT_FALSE(optimizer.checkReady()); // Needs model
    
    optimizer.setModel(model);
    EXPECT_FALSE(optimizer.checkReady()); // Needs objective and controls
    
    // Add objective
    Optimizer_if::ObjectiveDefinition obj;
    obj.name = "obj1";
    obj.expression = "10.0";
    obj.sense = Optimizer_if::ObjectiveSense::MAXIMIZE;
    optimizer.addObjective(obj);
    
    EXPECT_FALSE(optimizer.checkReady()); // Needs controls
    
    // Add a dummy control
    SimulationControlDouble* dummyControl = new SimulationControlDouble(
        []() { return 0.0; },
        [](double) {},
        "DummyClass", "DummyElement", "DummyProperty"
    );
    List<SimulationControl*> controls;
    controls.insert(dummyControl);
    optimizer.setSelectedControls(&controls);
    
    EXPECT_TRUE(optimizer.checkReady());
    
    delete dummyControl;
    delete model;
    delete sim;
}

TEST(OptimizerDefaultImpl1Test, LifecycleAndOptimizationStep) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    OptimizerDefaultImpl1 optimizer;
    optimizer.setModel(model);
    
    Optimizer_if::ObjectiveDefinition obj;
    obj.name = "MaxObj";
    obj.expression = "25.0"; // A constant expression
    obj.sense = Optimizer_if::ObjectiveSense::MAXIMIZE;
    optimizer.addObjective(obj);
    
    SimulationControlDouble* dummyControl = new SimulationControlDouble(
        []() { return 0.0; },
        [](double) {},
        "DummyClass", "DummyElement", "DummyProperty"
    );
    List<SimulationControl*> controls;
    controls.insert(dummyControl);
    optimizer.setSelectedControls(&controls);
    
    Optimizer_if::OptimizationSettings settings;
    settings.maxIterations = 2;
    settings.bestSolutionsToKeep = 1;
    settings.replicationsPerSolution = 1;
    optimizer.setSettings(settings);
    
    EXPECT_TRUE(optimizer.start());
    EXPECT_EQ(optimizer.getExecutionState(), Optimizer_if::ExecutionState::RUNNING);
    
    EXPECT_TRUE(optimizer.step());
    EXPECT_EQ(optimizer.getCurrentIteration(), 1);
    
    auto bestSolutions = optimizer.getBestSolutions();
    EXPECT_EQ(bestSolutions->size(), 1);
    
    auto currentBest = optimizer.getCurrentBestSolution();
    ASSERT_NE(currentBest, nullptr);
    EXPECT_EQ(currentBest->aggregatedObjectiveValue, 25.0);
    
    EXPECT_TRUE(optimizer.step());
    EXPECT_EQ(optimizer.getExecutionState(), Optimizer_if::ExecutionState::FINISHED);
    
    delete dummyControl;
    delete model;
    delete sim;
}

TEST(OptimizerDefaultImpl1Test, InfeasibleSolutionsAreDiscarded) {
    Simulator* sim = new Simulator();
    Model* model = new Model(sim);
    
    OptimizerDefaultImpl1 optimizer;
    optimizer.setModel(model);
    
    Optimizer_if::ObjectiveDefinition obj;
    obj.name = "MaxObj";
    obj.expression = "10.0";
    obj.sense = Optimizer_if::ObjectiveSense::MAXIMIZE;
    optimizer.addObjective(obj);
    
    Optimizer_if::ConstraintDefinition constraint;
    constraint.name = "ImpossibleConstraint";
    constraint.expression = "0.0"; // Evaluates to 0, means violated
    optimizer.addConstraint(constraint);
    
    SimulationControlDouble* dummyControl = new SimulationControlDouble(
        []() { return 0.0; },
        [](double) {},
        "DummyClass", "DummyElement", "DummyProperty"
    );
    List<SimulationControl*> controls;
    controls.insert(dummyControl);
    optimizer.setSelectedControls(&controls);
    
    optimizer.start();
    optimizer.step(); // Runs one iteration
    
    auto bestSolutions = optimizer.getBestSolutions();
    EXPECT_EQ(bestSolutions->size(), 0); // Discarded due to constraint violation
    
    delete dummyControl;
    delete model;
    delete sim;
}

} // namespace
