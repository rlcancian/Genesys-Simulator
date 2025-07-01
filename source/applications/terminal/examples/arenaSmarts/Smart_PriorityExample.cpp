/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Dummy.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */
#include "Smart_PriorityExample.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../TraitsApp.h"

Smart_PriorityExample::Smart_PriorityExample() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_PriorityExample::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	// create components
	Create* create1 = plugins->newInstance<Create>(model, "GP");
	Seize* seize1 = plugins->newInstance<Seize>(model, "General Triage");
	Delay* delay1 = plugins->newInstance<Delay>(model, "General Triage Delay");
	Release* release1 = plugins->newInstance<Release>(model, "General Triage Release");
	Create* create2 = plugins->newInstance<Create>(model, "CIP");
	Seize* seize2 = plugins->newInstance<Seize>(model, "Emergency Procedure");
	Delay* delay2 = plugins->newInstance<Delay>(model, "Emergency Procedure Delay");
	Release* release2 = plugins->newInstance<Release>(model, "Emergency Procedure Release");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Patient is discharged or admitted");
	// create data
	Resource* resource1 = plugins->newInstance<Resource>(model, "Doctor");

	// connect model components to create a "workflow"
	create1->getConnections()->insert(seize1);
	seize1->getConnections()->insert(delay1);
	delay1->getConnections()->insert(release1);
	release1->getConnections()->insert(dispose1);
	//
	create2->getConnections()->insert(seize2);
	seize2->getConnections()->insert(delay2);
	delay2->getConnections()->insert(release2);
	release2->getConnections()->insert(dispose1);

	// configure data
	resource1->setCapacity(1);
	resource1->setCostBusyTimeUnit(50.0);
	resource1->setCostIdleTimeUnit(50.0);

	// configure components
	create1->setTimeBetweenCreationsExpression("EXPO(10)", Util::TimeUnit::minute);
	create2->setTimeBetweenCreationsExpression("EXPO(15)", Util::TimeUnit::minute);

	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Queue_Seize_1");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	seize1->getSeizeRequests()->insert(new SeizableItem(resource1));
	seize1->setQueue(queueSeize1);
	seize1->setPriority(2);

	Queue* queueSeize2 = plugins->newInstance<Queue>(model, "Queue_Seize_2");
	queueSeize2->setOrderRule(Queue::OrderRule::FIFO);
	seize2->getSeizeRequests()->insert(new SeizableItem(resource1));
	seize2->setQueue(queueSeize2);
	seize2->setPriority(1);

	delay1->setDelayExpression("TRIA(0.5, 1, 1.5)", Util::TimeUnit::minute);
	delay2->setDelayExpression("TRIA(0.5, 1, 2)", Util::TimeUnit::minute);

	release1->getReleaseRequests()->insert(new SeizableItem(resource1));
	release2->getReleaseRequests()->insert(new SeizableItem(resource1));

	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationLength(690, Util::TimeUnit::minute);
	model->getSimulation()->setWarmUpPeriod(34);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_PriorityExample.gen");
	
	model->getSimulation()->start();
	for (int i = 0; i < 1e9; ++i);
	delete genesys;
	return 0;
};

