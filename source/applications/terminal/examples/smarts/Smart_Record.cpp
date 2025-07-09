/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_Record.cpp
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Record.h"
#include <thread>
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Record::Smart_Record() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Record::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create *create = plugins->newInstance<Create>(model);
	Process* process = plugins->newInstance<Process>(model);
	Resource *resource1 = plugins->newInstance<Resource>(model);
	resource1->setCapacity(5);
	process->getSeizeRequests()->insert(new SeizableItem(resource1));
	process->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	process->setDelayExpression("unif(2, 6)");
	Record *record1 = plugins->newInstance<Record>(model);
	record1->setExpressionName("Number in queue");
	record1->setExpression("nq(Queue_1)");
	record1->setTimeDependent(true);
	record1->setFilename("recordNumberInQueue.txt");
	Record *record2 = plugins->newInstance<Record>(model);
	record2->setExpressionName("resource_1 number busy");
	record2->setExpression("NR(Resource_1)");
	record2->setTimeDependent(true);
	record2->setFilename("recordNumberBusy.txt");
	Record *record3 = plugins->newInstance<Record>(model);
	record3->setExpressionName("Just a Beta distribution");
	record3->setExpression("beta(2,8,0,100)");
	record3->setFilename("recordBeta.txt");
	Dispose* dispose = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create->getConnectionManager()->insert(process);
	process->getConnectionManager()->insert(record1);
	record1->getConnectionManager()->insert(record2);
	record2->getConnectionManager()->insert(record3);
	record3->getConnectionManager()->insert(dispose);
	// set options, save and simulate step-by-step (but no user interaction required)
	model->getSimulation()->setReplicationLength(1e3);
	model->getSimulation()->setNumberOfReplications(30);
	model->save("./models/Smart_Record.gen");
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L2_results);
	// execute the simulation
	//do {
	//	model->getSimulation()->step();
	//} while (model->getSimulation()->isPaused());
	model->getSimulation()->start();

	delete genesys;
	return 0;
};

