/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_SeizeDelayReleaseMany.cpp
 * Author: rlcancian
 *
 * Created on 9 de abril de 2021, 18:09
 */

#include "Smart_SeizeDelayReleaseMany.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_SeizeDelayReleaseMany::Smart_SeizeDelayReleaseMany() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SeizeDelayReleaseMany::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntitiesPerCreation(2);
	Resource* machine1 = plugins->newInstance<Resource>(model);
	Resource* machine2 = plugins->newInstance<Resource>(model);
	Resource* machine3 = plugins->newInstance<Resource>(model);
	Resource* machine4 = plugins->newInstance<Resource>(model);
	Resource* machine5 = plugins->newInstance<Resource>(model);
	Queue* queueSeize1 = plugins->newInstance<Queue>(model);
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(machine1));
	seize1->getSeizeRequests()->insert(new SeizableItem(machine2));
	seize1->getSeizeRequests()->insert(new SeizableItem(machine3));
	seize1->getSeizeRequests()->insert(new SeizableItem(machine4));
	seize1->getSeizeRequests()->insert(new SeizableItem(machine5));
	seize1->setQueue(queueSeize1);
	seize1->setAllocationType(Util::AllocationType::Others);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("unif(0.6,1.5)");
	delay1->setAllocation(Util::AllocationType::ValueAdded);
	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(machine1));
	release1->getReleaseRequests()->insert(new SeizableItem(machine2));
	release1->getReleaseRequests()->insert(new SeizableItem(machine3));
	release1->getReleaseRequests()->insert(new SeizableItem(machine4));
	release1->getReleaseRequests()->insert(new SeizableItem(machine5));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(10);
	sim->setNumberOfReplications(3);
	model->save("./models/Smart_SeizeDelayReleaseMany.gen");
	do {
		sim->start(); //step();
		//std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	} while (false); //(sim->isPaused());
	for(int i=0;i<1e6;i++);
	delete genesys;
	return 0;
};
