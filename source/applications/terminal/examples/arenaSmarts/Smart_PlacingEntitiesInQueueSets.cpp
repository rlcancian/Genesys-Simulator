/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_PlacingEntitiesInQueueSets.cpp
 * Author: Joçao Janini
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_PlacingEntitiesInQueueSets.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../TraitsApp.h"

Smart_PlacingEntitiesInQueueSets::Smart_PlacingEntitiesInQueueSets() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */

int Smart_PlacingEntitiesInQueueSets::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	// Create Customer into system
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Entity 1");
	create1->setTimeBetweenCreationsExpression("tria(6,7,8)"); 
	create1->setTimeUnit(Util::TimeUnit::minute);
	create1->setEntitiesPerCreation(20);
	create1->setMaxCreations(1); 

	// Assign aCustomerType
	Assign* assign = plugins->newInstance<Assign>(model);
	// O aCustomerType é sempre = 0 com a expressão do Arena: disc(0.25, 1, 0.5, 2, 0.75, 3, 1, 4)
	// assign->getAssignments()->insert(new Assignment(model, "aCustomerType", "disc(0.25, 1, 0.5, 2, 0.75, 3, 1, 4)"));
	// Optamos por alterar a experessão por "trunc(unif(1, 5))" (aqui e no Arena)
	assign->getAssignments()->insert(new Assignment(model, "aCustomerType", "trunc(unif(1, 5))"));

	Queue* queue1 =  plugins->newInstance<Queue>(model, "Queue for Customer Type 1");
	Queue* queue2 =  plugins->newInstance<Queue>(model, "Queue for Customer Type 2");
	Queue* queue3 =  plugins->newInstance<Queue>(model, "Queue for Customer Type 3");
	Queue* queue4 =  plugins->newInstance<Queue>(model, "Queue for Customer Type 4");
	//queue1->setAttributeName("aCustomerType");
	//queue2->setAttributeName("aCustomerType");
	//queue3->setAttributeName("aCustomerType");
	//queue4->setAttributeName("aCustomerType");

	Decide* decide1 = plugins->newInstance<Decide>(model, "Decide_1");
	decide1->getConditions()->insert("aCustomerType == 1");
	decide1->getConditions()->insert("aCustomerType == 2");
	decide1->getConditions()->insert("aCustomerType == 3");
	decide1->getConditions()->insert("aCustomerType == 4");

	Wait* wait1 = plugins->newInstance<Wait>(model);
	wait1->setWaitType(Wait::WaitType::InfiniteHold);
	wait1->setQueue(queue1);

	Wait* wait2 = plugins->newInstance<Wait>(model);
	wait2->setWaitType(Wait::WaitType::InfiniteHold);
	wait2->setQueue(queue2);

	Wait* wait3 = plugins->newInstance<Wait>(model); 
	wait3->setWaitType(Wait::WaitType::InfiniteHold);
	wait3->setQueue(queue3);

	Wait* wait4 = plugins->newInstance<Wait>(model);
	wait4->setWaitType(Wait::WaitType::InfiniteHold);
	wait4->setQueue(queue4);

	Dispose* dummy_dispose = plugins->newInstance<Dispose>(model, "Dummy_Dispose");

	// connect model components to create a "workflow"
	create1->getConnections()->insert(assign);
	assign->getConnections()->insert(decide1);

	decide1->getConnections()->insert(wait1);
	decide1->getConnections()->insert(wait2);
	decide1->getConnections()->insert(wait3);
	decide1->getConnections()->insert(wait4);

	wait1->getConnections()->insert(dummy_dispose); // just because "Wait" is not a SinkComponent
	wait2->getConnections()->insert(dummy_dispose); // just because "Wait" is not a SinkComponent
	wait3->getConnections()->insert(dummy_dispose); // just because "Wait" is not a SinkComponent
	wait4->getConnections()->insert(dummy_dispose); // just because "Wait" is not a SinkComponent
 
	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(300);
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::minute);
	
	double replication_length = model->getSimulation()->getReplicationLength();
	model->getSimulation()->setWarmUpPeriod(replication_length * 0.05); 
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	
	model->save("./models/Smart_PlacingEntitiesInQueueSets.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0; 
};
