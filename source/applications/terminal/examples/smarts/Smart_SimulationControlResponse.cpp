/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_SimulationControlResponse.cpp
 * Author: rlcancian
 * 
 * Created on 29 de novembro de 2022, 21:07
 */

#include "Smart_SimulationControlResponse.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_SimulationControlResponse::Smart_SimulationControlResponse() {
}

int Smart_SimulationControlResponse::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Process* process1 = plugins->newInstance<Process>(model);
	process1->setQueueableItem(new QueueableItem(model, "Queue_1"));
	process1->getSeizeRequests()->insert(new SeizableItem(model, "Resource_1"));
	//Delay* process1 = plugins->newInstance<Delay>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(process1);
	process1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	//model->save("./models/Smart_SimulationControlResponse.gen");
	model->check();
	model->show();
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
	model->getSimulation()->setShowSimulationControlsInReport(true);
	model->getSimulation()->setShowSimulationResposesInReport(true);
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
