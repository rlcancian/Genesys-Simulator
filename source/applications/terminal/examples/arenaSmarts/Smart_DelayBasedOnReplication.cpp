/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_DelayBasedOnReplication.cpp
 * Author: João Janini
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_DelayBasedOnReplication.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../TraitsApp.h"




Smart_DelayBasedOnReplication::Smart_DelayBasedOnReplication() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */

// this is executed on each replication start
void Smart_DelayBasedOnReplication::TestEventHandler(SimulationEvent *re) {
	std::cout << "(Event Handler) " << "Replication " << re->getCurrentReplicationNumber() << std::endl;
	int rnum = re->getCurrentReplicationNumber();
	// updates delay expression based on current repĺication (check prints on log)
	this->delay->setDelay(this->proctimes[rnum-1]);
	std::cout << "Delay expression: " << this->delay->delayExpression() << std::endl;
}


int Smart_DelayBasedOnReplication::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Entity 1");
	create1->setTimeBetweenCreationsExpression("EXPO(9)"); // Random(Expo), 9
	create1->setTimeUnit(Util::TimeUnit::minute);
	create1->setEntitiesPerCreation(1);
	create1->setFirstCreation(0.0); 

    Resource* resource1 = plugins->newInstance<Resource>(model, "Resource_1");
	resource1->setCapacity(1);

	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Seize_1.Queue");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);

	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(resource1, "1"));
	seize1->setQueue(queueSeize1);

	// Variable* var = plugins->newInstance<Variable>(model, "rnum");
    // var->setInitialValue(0);

	// StartReplicationEventHandler does not take any more arguments
	// Sets deplay as a class attribute to ensure it's accessible there
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayTimeUnit(Util::TimeUnit::minute);
	// delay1->setDelayExpression("rnum", Util::TimeUnit::minute);
	this->delay = delay1;

	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(resource1, "1"));

    // Dispose 1
    Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Dispose_1");

	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(dispose1);


	// add replication start handler. Smart_DelayBasedOnReplication::TestEventHandler executes 
	// when a replication starts
	OnEventManager* oem = model->getOnEventManager();
	oem->addOnReplicationStartHandler(this, &Smart_DelayBasedOnReplication::TestEventHandler);

	// set options, save and simulate

	// Make sure all values from array are initialized
	int replicationNum = 3; 
	this->proctimes = new int[replicationNum];
	this->proctimes[0] = 20;
	this->proctimes[1] = 12;
	this->proctimes[2] = 16;

	model->getSimulation()->setNumberOfReplications(replicationNum);

	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::minute);
	// model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000"); 
	
	double replication_length = model->getSimulation()->getReplicationLength();

	// Set warm up period to 5 % of replication length
	model->getSimulation()->setWarmUpPeriod(replication_length * 0.05);
	
	model->save("./models/Smart_DelayBasedOnReplication.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
