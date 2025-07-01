/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_DefiningControlLogic.cpp
 * Author: João Janini
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_DefiningControlLogic.h"

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



Smart_DefiningControlLogic::Smart_DefiningControlLogic() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */


int Smart_DefiningControlLogic::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create2 = plugins->newInstance<Create>(model);
	create2->setEntityTypeName("Entity 2");
	create2->setTimeBetweenCreationsExpression("EXPO(1)"); // Random(Expo), 9
	create2->setTimeUnit(Util::TimeUnit::minute);
	create2->setFirstCreation(120);
	create2->setMaxCreations(1);

	Variable* var = plugins->newInstance<Variable>(model, "process_time");
	var->setInitialValue(2);

	Assign* var_assign1 = plugins->newInstance<Assign>(model, "var_assign1");
	var_assign1->getAssignments()->insert(new Assignment(model, "process_time", "2", false));
	
	Delay* delay2 = plugins->newInstance<Delay>(model);
	delay2->setDelayTimeUnit(Util::TimeUnit::minute);
	delay2->setDelay(240);

	Assign* var_assign2 = plugins->newInstance<Assign>(model, "var_assign2");
	var_assign2->getAssignments()->insert(new Assignment(model, "process_time", "1", false));

	Dispose* dispose2 = plugins->newInstance<Dispose>(model, "Dispose_1");

	// Model Logic
	Create* create1 = plugins->newInstance<Create>(model, "Mail Arrival");
	create1->setEntityTypeName("Entity 1");
	create1->setTimeBetweenCreationsExpression("EXPO(2)"); // Random(Expo), 9
	create1->setTimeUnit(Util::TimeUnit::minute);

    Resource* resource1 = plugins->newInstance<Resource>(model, "Resource_1");
	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Seize_1.Queue");
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(resource1, "1"));
	seize1->setQueue(queueSeize1);

	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayTimeUnit(Util::TimeUnit::minute);
	delay1->setDelayExpression("process_time");

	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(resource1, "1"));

    Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Dispose_1");

	// connect model components to create a "workflow"
	create1->getConnections()->insert(seize1);
	seize1->getConnections()->insert(delay1);
	delay1->getConnections()->insert(release1);
	release1->getConnections()->insert(dispose1);

	create2->getConnections()->insert(var_assign1);
	var_assign1->getConnections()->insert(delay2);
	delay2->getConnections()->insert(var_assign2);
	var_assign2->getConnections()->insert(dispose2); 

	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::minute);
	
	double replication_length = model->getSimulation()->getReplicationLength();
	model->getSimulation()->setWarmUpPeriod(replication_length * 0.05); 
	
	model->save("./models/Smart_DefiningControlLogic.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
