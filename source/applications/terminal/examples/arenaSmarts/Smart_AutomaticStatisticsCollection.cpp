/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_AutomaticStatisticsCollection.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_AutomaticStatisticsCollection.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_AutomaticStatisticsCollection::Smart_AutomaticStatisticsCollection() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_AutomaticStatisticsCollection::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	EntityType* entity1 = plugins->newInstance<EntityType>(model);

	Create* create = plugins->newInstance<Create>(model, "Equipment Arrives");
	create->setEntityType(entity1);
	create->setTimeBetweenCreationsExpression("NORM(8,9)");
	create->setTimeUnit(Util::TimeUnit::minute);

	Resource* resource = plugins->newInstance<Resource>(model, "Processor");
	resource->setCostBusyTimeUnit(5.25);

	Process* process = plugins->newInstance<Process>(model, "Equipment is Processed");
	process->getSeizeRequests()->insert(new SeizableItem(resource));
	process->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	process->setDelayTimeUnit(Util::TimeUnit::hour);
	process->setDelayExpression("tria(1,2,3)");
	process->setPriority(1);
	process->setAllocationType(Util::AllocationType::ValueAdded);

	Dispose* dispose = plugins->newInstance<Dispose>(model);

	// connect model components to create a "workflow"
	create->getConnectionManager()->insert(process);
	process->getConnectionManager()->insert(dispose);

	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();

	//sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
	sim->setNumberOfReplications(3);
	sim->setReplicationLength(480, Util::TimeUnit::minute); 
	sim->setWarmUpPeriod(sim->getReplicationLength()*0.05); 
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_EvaluatingConditionsBeforeEnteringQueue.gen");
	sim->start();

	for (int i = 0; i < 1e9; i++);
	delete genesys;
	return 0;
};
