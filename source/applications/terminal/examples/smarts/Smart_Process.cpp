/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_Process.cpp
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Process.h"
#include <thread>
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Process::Smart_Process() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Process::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create *create = plugins->newInstance<Create>(model);
	Process* process = plugins->newInstance<Process>(model);
	process->getSeizeRequests()->insert(new SeizableItem(plugins->newInstance<Resource>(model)));
	process->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	process->setDelayExpression("unif(0.7,1.5)");
	Dispose* dispose = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create->getConnectionManager()->insert(process);
	process->getConnectionManager()->insert(dispose);
	// set options, save and simulate step-by-step (but no user interaction required)
	model->getSimulation()->setReplicationLength(10);
	model->getSimulation()->setNumberOfReplications(5);
	model->save("./models/Smart_Process.gen");
	// execute the simulation
	do {
		model->getSimulation()->step();
	} while (model->getSimulation()->isPaused());
	delete genesys;
	return 0;
};

