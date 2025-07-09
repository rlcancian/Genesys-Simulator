/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_EntitiesProcessedByPriority.cpp
 * Author: bepasar
 *
 * Created on 09/12/22
 */

#include "Smart_EntitiesProcessedByPriority.h"
#include <thread>
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Attribute.h"

#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_EntitiesProcessedByPriority::Smart_EntitiesProcessedByPriority() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_EntitiesProcessedByPriority::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	// entities
	// EntityType* regularJob = plugins->newInstance<EntityType>(model, "Regular Job");
	// EntityType* rushJob = plugins->newInstance<EntityType>(model, "Rush Job");

	// resources
	Resource* packer = plugins->newInstance<Resource>(model, "Packer");
	// packer->setCapacity(1); // capacidade original do recurso
	packer->setCapacity(5); // alterado para 5, pois foi necessaria a alteracao no arena, para se conseguir atingir 1000 entidades saindo do sistema

	// attributes
	// Attribute* priority =  plugins->newInstance<Attribute>(model, "priority"); // precisa?

	// Create 1
	Create *create_1 = plugins->newInstance<Create>(model, "Create 1");
	create_1->setEntityTypeName("Regular Job");
	create_1->setTimeBetweenCreationsExpression("expo(3)", Util::TimeUnit::minute);
	create_1->setEntitiesPerCreation(1);

	// Assign 1
	Assign* assign_1 = plugins->newInstance<Assign>(model, "Assign 1");
	assign_1->getAssignments()->insert(new Assignment("priority", "2"));

	// Process 1
	Process* process_1 = plugins->newInstance<Process>(model, "Process 1");
	process_1->getSeizeRequests()->insert(new SeizableItem(packer));
	process_1->setPriorityExpression("priority");
	process_1->setDelayExpression("expo(5)");
	process_1->setDelayTimeUnit(Util::TimeUnit::minute);
	process_1->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "Process1.Queue")));

	// Create 2
	Create *create_2 = plugins->newInstance<Create>(model, "Create 2");
	create_2->setEntityTypeName("Rush Job");
	create_2->setTimeBetweenCreationsExpression("expo(10)", Util::TimeUnit::minute);
	create_2->setEntitiesPerCreation(1);

	// Assign 2
	Assign* assign_2 = plugins->newInstance<Assign>(model, "Assign 2");
	assign_2->getAssignments()->insert(new Assignment("priority", "1"));

	// Process 2
	Process* process_2 = plugins->newInstance<Process>(model, "Process 2");
	process_2->getSeizeRequests()->insert(new SeizableItem(packer));
	process_2->setPriorityExpression("priority");
	process_2->setDelayExpression("expo(5)");
	process_2->setDelayTimeUnit(Util::TimeUnit::minute);
	process_2->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "Process2.Queue")));

	// Dispose 1
	Dispose* dispose_1 = plugins->newInstance<Dispose>(model, "Dispose 1");

	// connecting model components to create a "workflow"
	create_1->getConnectionManager()->insert(assign_1);
	assign_1->getConnectionManager()->insert(process_1);
	process_1->getConnectionManager()->insert(dispose_1);

	create_2->getConnectionManager()->insert(assign_2);
	assign_2->getConnectionManager()->insert(process_2);
	process_2->getConnectionManager()->insert(dispose_1);

	// set options, save and simulate step-by-step (but no user interaction required)
	double replicationLength = 240;
	model->getSimulation()->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setReplicationLength(replicationLength);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setWarmUpPeriod(replicationLength * 0.05); 
	model->getSimulation()->setNumberOfReplications(3);
	model->save("./models/Smart_EntitiesProcessedByPriority.gen");

	// execute the simulation
	model->getSimulation()->start();
	
	delete genesys;
	return 0;
};

