/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_AlternatingEntityCreation.cpp
 * Author: João Janini
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_AlternatingEntityCreation.h"

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
#include "../../../TraitsApp.h"

Smart_AlternatingEntityCreation::Smart_AlternatingEntityCreation() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */

int Smart_AlternatingEntityCreation::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	// Create 1
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Entity_1");
	create1->setTimeBetweenCreationsExpression("EXPO(1)");
	create1->setTimeUnit(Util::TimeUnit::minute);
	create1->setMaxCreations(99999999999999); // Infinite

	Assign* assign = plugins->newInstance<Assign>(model);
	assign->getAssignments()->insert(new Assignment(model, "serialNumber", "serialNumber + 1", false));
	assign->getAssignments()->insert(new Assignment(model, "isEven", "MOD(serialNumber,2)"));

	Decide* decide1 = plugins->newInstance<Decide>(model, "Decide_1");
	decide1->getConditions()->insert("isEven==0");
	decide1->getConditions()->insert("isEven==1");
 
	Assign* assign1 = plugins->newInstance<Assign>(model, "Assign_1");
	EntityType* typeA = new EntityType(model, "typeA");
	assign1->getAssignments()->insert(new Assignment("Entity.Type",std::to_string(typeA->getId()), true));

	EntityType* typeB = new EntityType(model, "typeB");
	Assign* assign2 = plugins->newInstance<Assign>(model, "Assign_1");
	assign2->getAssignments()->insert(new Assignment("Entity.Type",std::to_string(typeB->getId()), true));
	
	Dispose* dipose1 = plugins->newInstance<Dispose>(model, "Dispose_1");
	Dispose* dipose2 = plugins->newInstance<Dispose>(model, "Dispose_2");

	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(assign);
	assign->getConnectionManager()->insert(decide1);
	decide1->getConnectionManager()->insert(assign1);
	decide1->getConnectionManager()->insert(assign2);
	assign1->getConnectionManager()->insert(dipose1);
	assign2->getConnectionManager()->insert(dipose2);
 
	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::minute);

	double replication_length = model->getSimulation()->getReplicationLength(); 
	model->getSimulation()->setWarmUpPeriod(replication_length * 0.05);
	
	model->save("./models/Smart_AlternatingEntityCreation.gen");
	model->getSimulation()->start();
	for (int i=0; i<1e9; i++);
	delete genesys;
	return 0;
};

