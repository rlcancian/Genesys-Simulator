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
    // insert "fake plugins" since plugins based on dynamic loaded library are not implemented yet
    this->insertFakePluginsByHand(simulator);

    {
        // creates an empty model
        Model * model = new Model(simulator);
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

        simulator->getModels()->remove(model);
        delete model;
    }

    std::cout << std::endl;
    auto controls = manager->extractControlsFromModel(filename);
    std::cout << "Controls:" << std::endl;
    for (auto i = 0; i < controls->size(); ++i) {
        std::cout << controls->getAtRank(i)->getName() << std::endl;
    }
    std::cout << std::endl;

    auto responses = manager->extractResponsesFromModel(filename);
    std::cout << "Responses:" << std::endl;
    for (auto i = 0; i < responses->size(); ++i) {
        std::cout << responses->getAtRank(i)->getName() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Press ENTER to run the simulation..." << std::endl;
    std::string s;
    std::getline(std::cin, s);

    SimulationScenario * scenario1 = new SimulationScenario();
    scenario1->setScenarioName("scenario1");
    scenario1->setModelFilename(filename);
    scenario1->setControlValue("NumberOfReplications", 1);
    scenario1->setControlValue("ReplicationLength", 60);
    scenario1->setControlValue("WarmupPeriod", 10);
    scenario1->selectResponse("Type_of_Representative_Entity.TotalTimeInSystem.average");

    SimulationScenario * scenario2 = new SimulationScenario();
    scenario2->setScenarioName("scenario2");
    scenario2->setModelFilename(filename);
    scenario2->setControlValue("NumberOfReplications", 1);
    scenario2->setControlValue("ReplicationLength", 30);
    scenario2->setControlValue("WarmupPeriod", 0);
    scenario2->selectResponse("Create_1.CountNumberIn");

    SimulationScenario * scenario3 = new SimulationScenario();
    scenario3->setScenarioName("scenario3");
    scenario3->setModelFilename(filename);
    scenario3->setControlValue("NumberOfReplications", 1);
    scenario3->setControlValue("ReplicationLength", 120);
    scenario3->setControlValue("WarmupPeriod", 25);
    scenario3->selectResponse("Delay_1.WaitTime.average");

    manager->getScenarios()->insert(scenario1);
    manager->getScenarios()->insert(scenario2);
    manager->getScenarios()->insert(scenario3);

    // Handle traces and simulation events to output them
    this->setDefaultTraceHandlers(simulator->getTracer());
    simulator->getTracer()->setTraceLevel(Util::TraceLevel::L8_mostDetailed);

    manager->startExperiment();

    std::cout << std::endl;
    std::cout << "Responses from simulation:" << std::endl;
    auto list_scenario = manager->getScenarios();
    for (auto scenario = list_scenario->front(); !list_scenario->empty(); scenario = list_scenario->front()) {

        auto response_values = scenario->getResponseValues();
        for (auto it = response_values->begin(); it != response_values->end(); ++it) {
            std::cout << scenario->getScenarioName() << ": " << it->first << " = " << it->second << std::endl;
        }

        list_scenario->pop_front();
    }
    std::cout << std::endl;

    delete manager;
    return 0;
}
