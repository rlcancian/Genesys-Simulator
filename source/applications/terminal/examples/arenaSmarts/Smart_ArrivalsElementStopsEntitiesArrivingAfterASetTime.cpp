/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime::Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	EntityType* entityType = plugins->newInstance<EntityType>(model);
	Create* create = plugins->newInstance<Create>(model);
	create->setEntityType(entityType);
	create->setEntitiesPerCreation(1);
	create->setTimeUnit(Util::TimeUnit::minute);
	create->setTimeBetweenCreationsExpression("EXPO(5)");

	Resource* resourceTeller = plugins->newInstance<Resource>(model, "Teller");

	Process* processTeller = plugins->newInstance<Process>(model, "Work with Teller");
	processTeller->getSeizeRequests()->insert(new SeizableItem(resourceTeller));
	processTeller->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	processTeller->setDelayTimeUnit(Util::TimeUnit::minute);
	processTeller->setAllocationType(Util::AllocationType::ValueAdded);
	processTeller->setDelayExpression("tria(1,2,3)");
	processTeller->setPriority(2);

	Decide* decide = plugins->newInstance<Decide>(model);
	decide->getConditions()->insert("UNIF(0,1)<0.25");

	Resource* resourceSupervisor = plugins->newInstance<Resource>(model, "Supervisor");

	Process* processSupervisor = plugins->newInstance<Process>(model, "Work with Supervisor");
	processSupervisor->getSeizeRequests()->insert(new SeizableItem(resourceSupervisor));
	processSupervisor->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	processSupervisor->setDelayTimeUnit(Util::TimeUnit::minute);
	processSupervisor->setAllocationType(Util::AllocationType::ValueAdded);
	processSupervisor->setDelayExpression("tria(5,10,15)");
	processSupervisor->setPriority(2);

	Dispose* dispose = plugins->newInstance<Dispose>(model);

	// connect model components to create a "workflow"
	create->getConnections()->insert(processTeller);
	processTeller->getConnections()->insert(decide);
	// True
	decide->getConnections()->insert(processSupervisor);
	processSupervisor->getConnections()->insert(processTeller);
	// False
	decide->getConnections()->insert(dispose);

	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();

	//sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
	sim->setReplicationLength(51, Util::TimeUnit::minute); 
	sim->setNumberOfReplications(3);
	sim->setWarmUpPeriod(sim->getReplicationLength()*0.05); //5% de 510
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.gen");
	sim->start();

	for (int i = 0; i < 1e9; i++);
	delete genesys;
	return 0;
};
