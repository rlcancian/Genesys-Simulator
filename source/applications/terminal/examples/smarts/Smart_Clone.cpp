/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Clone.cpp
 * Author: rlcancian
 *
 * Created on 30 de Novembro de 2022, 19:25
 */

#include "Smart_Clone.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Clone.h"
#include "../../../../plugins/components/Dispose.h"

Smart_Clone::Smart_Clone() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Clone::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	//genesys->getTracer()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// crete model
	Create* create1 = plugins->newInstance<Create>(model);
	Clone* clone1 = plugins->newInstance<Clone>(model);
	clone1->setNumClonesExpression("2");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	Dispose* dispose2 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnections()->insert(clone1);
	clone1->getConnections()->insert(dispose1);
	clone1->getConnections()->insert(dispose2);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->save("./models/Smart_Clone.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

