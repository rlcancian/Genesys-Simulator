/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ThirdExampleOfSimultion.cpp
 * Author: rlcancian
 * 
 * Created on 24 de Setembro de 2019, 16:43
 */

#include "Model_SeizeDelayRelease1.h"

// you have to included need libs

// GEnSyS Simulator
#include "Simulator.h"

// Model Components
#include "Create.h"
#include "Seize.h"
#include "Delay.h"
#include "Release.h"
#include "Dispose.h"

// Model elements
#include "EntityType.h"

Model_SeizeDelayRelease1::Model_SeizeDelayRelease1() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Model_SeizeDelayRelease1::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	// Handle traces and simulation events to output them
	// insert plugins
	this->insertFakePluginsByHand(genesys);
	this->setDefaultTraceHandlers(genesys->getTracer());
	genesys->getTracer()->setTraceLevel(Util::TraceLevel::L6_internal);
	Model* model = genesys->getModels()->newModel();
	// build the simulation model
	// set model infos
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(120);
	sim->setReplicationLengthTimeUnit(Util::TimeUnit::second);
	sim->setNumberOfReplications(2);
	//
	EntityType* customer = new EntityType(model, "Customer");
	//
	Create* create1 = new Create(model);
	create1->setEntityType(customer);
	create1->setTimeBetweenCreationsExpression("expo(20)");
	create1->setTimeUnit(Util::TimeUnit::second);
	create1->setEntitiesPerCreation(1);
	create1->setFirstCreation(0.0);
	//
	Resource* machine1 = new Resource(model, "Machine_1");
	machine1->setCapacity(1);
	//
	Queue* queueSeize1 = new Queue(model, "Seize_1.Queue");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	//
	Seize* seize1 = new Seize(model);
	//seize1->setResource(machine1);
	//seize1->setQuantity("1");
	seize1->getSeizeRequests()->insert(new SeizableItemRequest(machine1, "1"));
	seize1->setQueue(queueSeize1);
	//
	Delay* delay1 = new Delay(model);
	delay1->setDelayExpression("unif(15,30)");
	delay1->setDelayTimeUnit(Util::TimeUnit::second);
	//
	Release* release1 = new Release(model);
	//release1->setResource(machine1);
	//release1->setQuantity("1");
	release1->getReleaseRequests()->insert(new SeizableItemRequest(machine1, "1"));
	//
	Dispose* dispose1 = new Dispose(model);
	// connect model components to create a "workflow"
	create1->getNextComponents()->insert(seize1);
	seize1->getNextComponents()->insert(delay1);
	delay1->getNextComponents()->insert(release1);
	release1->getNextComponents()->insert(dispose1);
	// save the model into a text file
	model->save("./models/Model_SeizeDelayRelease1.txt");
	// execute the simulation
	sim->start();

	return 0;
};
