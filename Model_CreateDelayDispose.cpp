/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FirstExampleOfSimulation.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Model_CreateDelayDispose.h"

// you have to included need libs

// GEnSyS Simulator
#include "Simulator.h"

// Model Components
#include "Create.h"
#include "Delay.h"
#include "Dispose.h"

// Model model
#include "EntityType.h"

Model_CreateDelayDispose::Model_CreateDelayDispose() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Model_CreateDelayDispose::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	// insert "fake plugins" since plugins based on dynamic loaded library are not implemented yet
	this->insertFakePluginsByHand(genesys);
	// Handle traces and simulation events to output them
	this->setDefaultTraceHandlers(genesys->getTracer());
	genesys->getTracer()->setTraceLevel(Util::TraceLevel::L5_arrival);
	Model* model = genesys->getModels()->newModel();
	//model->load("./models/Model_CreateDelayDispose.txt");
	//model->getSimulation()->start();
	//return 0;
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
	// save the model into a text file
	model->save("./models/Model_CreateDelayDispose.txt");
	// execute the simulation util completed and show the report
	model->getSimulation()->start();
	genesys->~Simulator();
	return 0;
};

