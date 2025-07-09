/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_SeizeDelayReleaseNoDataDefs.cpp
 * Author: rlcancian
 * 
 * Created on 23 de maio de 2022, 12:39
 */

#include "Smart_SeizeDelayReleaseNoDataDefs.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"

Smart_SeizeDelayReleaseNoDataDefs::Smart_SeizeDelayReleaseNoDataDefs() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SeizeDelayReleaseNoDataDefs::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	//genesys->getTracer()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
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
	create1->setFirstCreation(0.0);
	//Resource* machine1 = plugins->newInstance<Resource>(model, "Machine_1");
	//machine1->setCapacity(1);
	//Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Seize_1.Queue");
	//queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(model, "Machine_1"));
	seize1->setQueueableItem(new QueueableItem(model, "Seize_1.Queue"));
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("unif(15,30)", Util::TimeUnit::second);
	Release* release1 = plugins->newInstance<Release>(model);
	////////////////////	release1->getReleaseRequests()->insert(new SeizableItem(machine1, "1"));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(120, Util::TimeUnit::second);
	sim->setNumberOfReplications(2);
	model->save("./models/Smart_SeizeDelayReleaseNoDataDefs.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};


