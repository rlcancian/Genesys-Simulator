/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_ModuleDisplayVariables.cpp
 * Author: bepasar
 *
 * Created on 08/12/22
 */

#include "Smart_ModuleDisplayVariables.h"
#include <thread>
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ModuleDisplayVariables::Smart_ModuleDisplayVariables() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ModuleDisplayVariables::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	// Create 1
	Create *create_1 = plugins->newInstance<Create>(model, "Create 1");
	create_1->setEntityTypeName("Entity 1");
	create_1->setTimeBetweenCreationsExpression("expo(1)", Util::TimeUnit::minute);
	create_1->setEntitiesPerCreation(1);

	// Process 1
	Process* process_1 = plugins->newInstance<Process>(model, "Process 1");
	process_1->setDelayExpression("tria(0.5,1,1.5)"); // * verificar se eh assim msm q define a dist triangular
	process_1->setDelayTimeUnit(Util::TimeUnit::minute);
	process_1->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "Process1.Queue")));

	// Dispose 1
	Dispose* dispose_1 = plugins->newInstance<Dispose>(model, "Dispose 1");

	// connecting model components to create a "workflow"
	create_1->getConnectionManager()->insert(process_1);
	process_1->getConnectionManager()->insert(dispose_1);

	// set options, save and simulate step-by-step (but no user interaction required)
	double replicationLength = 1300; // duracao para alcancar ~1000 entidades saindo do sistema
	model->getSimulation()->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setReplicationLength(replicationLength);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setWarmUpPeriod(replicationLength * 0.05); // = 1100*0.05 = 55
	// model->getSimulation()->setReplicationLength(240); // durecao original
	model->getSimulation()->setNumberOfReplications(300);
	model->save("./models/Smart_ModuleDisplayVariables.gen");

	// execute the simulation
	do {
		model->getSimulation()->step();
	} while (model->getSimulation()->isPaused());
	
	delete genesys;
	return 0;
};

