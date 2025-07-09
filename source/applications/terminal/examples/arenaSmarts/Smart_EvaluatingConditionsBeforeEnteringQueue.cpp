/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_EvaluatingConditionsBeforeEnteringQueue.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include <cmath>

#include "Smart_EvaluatingConditionsBeforeEnteringQueue.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_EvaluatingConditionsBeforeEnteringQueue::Smart_EvaluatingConditionsBeforeEnteringQueue() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_EvaluatingConditionsBeforeEnteringQueue::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	Variable* procStartTime = plugins->newInstance<Variable>(model, "procstarttime");
	Variable* procTime = plugins->newInstance<Variable>(model, "proctime");

	EntityType* entityType = plugins->newInstance<EntityType>(model, "Entity 1");

	Create* create = plugins->newInstance<Create>(model, "Create Entity");
	create->setEntityType(entityType);
	create->setTimeBetweenCreationsExpression("60");
	create->setTimeUnit(Util::TimeUnit::minute);
	create->setEntitiesPerCreation(1);
	create->setFirstCreation(0);

	Attribute* timeIn = plugins->newInstance<Attribute>(model, "Time_In");

	Assign* assign1 = plugins->newInstance<Assign>(model, "Assign attribute time in equal to current time variable TNOW");
	assign1->getAssignments()->insert(new Assignment(model, "Time_In", "TNOW"));

	std::string timeGoneBy = "(TNOW - procstarttime)";
	std::string remainingProcTime = "mx(0,(proctime - " + timeGoneBy + "))";
	Decide* decide = plugins->newInstance<Decide>(model, "Is the remaining process time less than 20?");
	decide->getConditions()->insert(remainingProcTime + " < 20");

	Resource* resource = plugins->newInstance<Resource>(model);
	resource->setCapacity(1);

	// Process 1        
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->setAllocationType(Util::AllocationType::ValueAdded);
	seize1->getSeizeRequests()->insert(new SeizableItem(resource));
	seize1->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	seize1->setPriority(2);

	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setAllocation(Util::AllocationType::ValueAdded);
	delay1->setDelayTimeUnit(Util::TimeUnit::minute);
	delay1->setDelay(0);

	Assign* assign2 = plugins->newInstance<Assign>(model, "Assign variables Procstarttime and Proctime");
	assign2->getAssignments()->insert(new Assignment(model, "procstarttime", "TNOW", false));
	assign2->getAssignments()->insert(new Assignment(model, "proctime", "expo(480)", false));

	// Process 2
	Delay* delay2 = plugins->newInstance<Delay>(model);
	delay2->setAllocation(Util::AllocationType::ValueAdded);
	delay2->setDelayTimeUnit(Util::TimeUnit::minute);
	delay2->setDelayExpression("proctime");

	Release* release = plugins->newInstance<Release>(model);
	release->getReleaseRequests()->insert(new SeizableItem(resource));

	Dispose* disposeTrue = plugins->newInstance<Dispose>(model, "Dispose of entity");
	Dispose* disposeFalse = plugins->newInstance<Dispose>(model, "Dispose of entity if condition not met");


	// connect model components to create a "workflow"
	create->getConnectionManager()->insert(assign1);
	assign1->getConnectionManager()->insert(decide);
	// decide == true
	decide->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(assign2);
	assign2->getConnectionManager()->insert(delay2);
	delay2->getConnectionManager()->insert(release);
	release->getConnectionManager()->insert(disposeTrue);
	// decide == false
	decide->getConnectionManager()->insert(disposeFalse);


	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();

	//sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
	sim->setReplicationLength(4800, Util::TimeUnit::minute); //tem q variar
	sim->setNumberOfReplications(3);
	sim->setWarmUpPeriod(sim->getReplicationLength()*0.05); //5% de 480 = 24
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_EvaluatingConditionsBeforeEnteringQueue.gen");
	sim->start();

	for (int i = 0; i < 1e9; i++);
	delete genesys;
	return 0;
};
