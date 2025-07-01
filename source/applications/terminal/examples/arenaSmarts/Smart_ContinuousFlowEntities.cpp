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

#include "Smart_ContinuousFlowEntities.h"

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
#include "../../../../plugins/components/Clone.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Variable.h"
#include "../../../TraitsApp.h"

Smart_ContinuousFlowEntities::Smart_ContinuousFlowEntities() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ContinuousFlowEntities::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Seize* seize1 = plugins->newInstance<Seize>(model);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	Release* release1 = plugins->newInstance<Release>(model);
	Clone* clone1 = plugins->newInstance<Clone>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// create data
	Resource* resource1 = plugins->newInstance<Resource>(model, "Resource 1");

	// connect model components to create a "workflow"
	create1->getConnections()->insert(seize1);
	seize1->getConnections()->insert(clone1);
	clone1->getConnections()->insert(delay1);
	clone1->getConnections()->insert(seize1);
	delay1->getConnections()->insert(release1);
	release1->getConnections()->insert(dispose1);

	// configure components
	create1->setTimeBetweenCreationsExpression("EXPO(1)", Util::TimeUnit::hour);

	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Queue_Seize_1");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	seize1->getSeizeRequests()->insert(new SeizableItem(resource1));
	seize1->setQueue(queueSeize1);

	clone1->setNumClonesExpression("1");
	delay1->setDelayExpression("10", Util::TimeUnit::minute);
	release1->getReleaseRequests()->insert(new SeizableItem(resource1));

	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationLength(48, Util::TimeUnit::hour);
	model->getSimulation()->setWarmUpPeriod(2.4);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	model->save("./models/Smart_ContinuousFlowEntities.gen");
	model->getSimulation()->start();
	for (int i = 0; i < 1e9; ++i);
	delete genesys;
	return 0;
};

