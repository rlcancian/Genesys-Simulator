/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ProcessSet.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_ProcessSet.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ProcessSet::Smart_ProcessSet() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ProcessSet::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create *create = plugins->newInstance<Create>(model);
	create->setEntityTypeName("Client");
	create->setTimeBetweenCreationsExpression("1");
	Dispose* dispose = plugins->newInstance<Dispose>(model);
	Set* set = plugins->newInstance<Set>(model);
	set->setSetOfType("Resource");
	set->getElementSet()->insert(plugins->newInstance<Resource>(model));
	set->getElementSet()->insert(plugins->newInstance<Resource>(model));
	set->getElementSet()->insert(plugins->newInstance<Resource>(model));
	set->getElementSet()->insert(plugins->newInstance<Resource>(model));
	set->getElementSet()->insert(plugins->newInstance<Resource>(model));
	Process* process = plugins->newInstance<Process>(model);
	process->getSeizeRequests()->insert(new SeizableItem(set));
	process->setQueueableItem(new QueueableItem(model, "myqueue"));
	process->setDelayExpression("unif(0.6,1.4)");
	// connect model components to create a "workflow"
	create->getConnectionManager()->insert(process);
	process->getConnectionManager()->insert(dispose);
	// set options, save and simulate step-by-step (but no user interaction required)
	model->getSimulation()->setReplicationLength(10);
	model->save("./models/Smart_ProcessSet.gen");
	do {
		model->getSimulation()->step();
	} while (model->getSimulation()->isPaused());
	delete genesys;
	return 0;
};

