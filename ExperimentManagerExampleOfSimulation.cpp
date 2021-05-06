/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ExperimentManagerExampleOfSimulation.cpp
 * Author: Guilherme
 * 
 * Created on 29 de março de 2021, 21:32
 */

#include "ExperimentManagerExampleOfSimulation.h"
#include "Traits.h"
#include "SourceModelComponent.h"
#include <string>
#include <iostream>
// you have to included need libs

// GEnSyS Simulator
#include "Simulator.h"

// Model Components
#include "Create.h"
#include "Delay.h"
#include "Dispose.h"

// Model model
#include "EntityType.h"

ExperimentManagerExampleOfSimulation::ExperimentManagerExampleOfSimulation() {
}

ExperimentManagerExampleOfSimulation::ExperimentManagerExampleOfSimulation(const ExperimentManagerExampleOfSimulation& orig) {
}

ExperimentManagerExampleOfSimulation::~ExperimentManagerExampleOfSimulation() {
}

int ExperimentManagerExampleOfSimulation::main(int argc, char** argv) {
    std::string filename{ "./temp/experimentManagerExampleOfSimulation.txt"};

    ExperimentManager_if * manager = new Traits<ExperimentManager_if>::Implementation();
    Simulator* simulator = manager->simulator();
    // Handle traces and simulation events to output them
    this->setDefaultTraceHandlers(simulator->getTracer());
    simulator->getTracer()->setTraceLevel(Util::TraceLevel::L8_mostDetailed);
    // insert "fake plugins" since plugins based on dynamic loaded library are not implemented yet
    this->insertFakePluginsByHand(simulator);
    bool wantToCreateNewModelAndSaveInsteadOfJustLoad = false;
    Model* model;
    if (wantToCreateNewModelAndSaveInsteadOfJustLoad) {
        // creates an empty model
        model = new Model(simulator);
        //
        // build the simulation model
        // if no ModelInfo is provided, then the model will be simulated once (one replication) and the replication length will be 3600 seconds (simulated time)
        model->getSimulation()->setReplicationLength(60);
        // create a (Source)ModelElement of type EntityType, used by a ModelComponent that follows
        EntityType* entityType1 = new EntityType(model, "Type_of_Representative_Entity");
        // create a ModelComponent of type Create, used to insert entities into the model
        Create* create1 = new Create(model);
        create1->setEntityType(entityType1);
        create1->setTimeBetweenCreationsExpression("1.5"); // create one new entity every 1.5 seconds
        // create a ModelComponent of type Delay, used to represent a time delay
        Delay* delay1 = new Delay(model);
        // if nothing else is set, the delay will take 1 second
        // create a (Sink)ModelComponent of type Dispose, used to remove entities from the model
        Dispose* dispose1 = new Dispose(model); // insert the component into the model
        // connect model components to create a "workflow" -- should always start from a SourceModelComponent and end at a SinkModelComponent (it will be checked)
        create1->getNextComponents()->insert(delay1);
        delay1->getNextComponents()->insert(dispose1);
        // insert the model into the simulator
        simulator->getModels()->insert(model);
        model->check();
        // save the model into a text file
        model->save(filename);
    } else {
        simulator->getModels()->loadModel(filename);
        model = simulator->getModels()->current();
    }

    // execute the simulation util completed and show the report

    SimulationScenario * scenario1 = new SimulationScenario();
    scenario1->setScenarioName("scenario1");
    scenario1->setModelFilename(filename);
    scenario1->getSelectedControls()->push_back(new std::pair<std::string, double>("NumberOfReplications", 1));
    scenario1->getSelectedControls()->push_back(new std::pair<std::string, double>("ReplicationLength", 60));
    scenario1->getSelectedControls()->push_back(new std::pair<std::string, double>("WarmupPeriod", 10));
    scenario1->getSelectedResponses()->push_back("Create_1.CountNumberIn");
    manager->getScenarios()->insert(scenario1);

    SimulationScenario * scenario2 = new SimulationScenario();
    scenario2->setScenarioName("scenario2");
    scenario2->setModelFilename(filename);
    scenario2->getSelectedControls()->push_back(new std::pair<std::string, double>("NumberOfReplications", 1));
    scenario2->getSelectedControls()->push_back(new std::pair<std::string, double>("ReplicationLength", 30));
    scenario2->getSelectedControls()->push_back(new std::pair<std::string, double>("WarmupPeriod", 0));
    scenario2->getSelectedResponses()->push_back("Create_1.CountNumberIn");
    manager->getScenarios()->insert(scenario2);

    SimulationScenario * scenario3 = new SimulationScenario();
    scenario3->setScenarioName("scenario3");
    scenario3->setModelFilename(filename);
    scenario3->getSelectedControls()->push_back(new std::pair<std::string, double>("NumberOfReplications", 1));
    scenario3->getSelectedControls()->push_back(new std::pair<std::string, double>("ReplicationLength", 120));
    scenario3->getSelectedControls()->push_back(new std::pair<std::string, double>("WarmupPeriod", 25));
    scenario3->getSelectedResponses()->push_back("Create_1.CountNumberIn");
    manager->getScenarios()->insert(scenario3);

    manager->startExperiment();

    for (SimulationScenario *scenario = manager->getScenarios()->front();
            !manager->getScenarios()->empty();
            manager->getScenarios()->pop_front(), scenario = manager->getScenarios()->front()) {
        double value = scenario->getResponseValue("Create_1.CountNumberIn");
        std::cout << scenario->getScenarioName() << " -> Created Creates: " << value << std::endl;
    }

    delete manager;
    return 0;
}
