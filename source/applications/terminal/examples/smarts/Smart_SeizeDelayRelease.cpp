/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SeizeDelayRelease.cpp
 * Author: rlcancian
 * 
 * Created on 24 de Setembro de 2019, 16:43
 */

#include "Smart_SeizeDelayRelease.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_SeizeDelayRelease::Smart_SeizeDelayRelease() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SeizeDelayRelease::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Customer");
	create1->setTimeBetweenCreationsExpression("expo(20)");
	create1->setTimeUnit(Util::TimeUnit::second);
	create1->setEntitiesPerCreation(1);
	Resource* machine1 = plugins->newInstance<Resource>(model, "Machine_1");
	machine1->setCapacity(1);
	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Seize_1.Queue");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(machine1, "1"));
	seize1->setQueue(queueSeize1);
	seize1->setAllocationType(Util::AllocationType::Transfer);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("unif(10,30)", Util::TimeUnit::second);
	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(machine1, "1"));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnections()->insert(seize1);
	seize1->getConnections()->insert(delay1);
	delay1->getConnections()->insert(release1);
	release1->getConnections()->insert(dispose1);
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(1000, Util::TimeUnit::second);
	sim->setNumberOfReplications(30);
    model->getTracer()->setTraceLevel(TraceManager::Level::L5_event);
	model->save("./models/Smart_SeizeDelayRelease.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};
