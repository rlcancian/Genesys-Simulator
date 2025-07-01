/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_MarkovChain.cpp
 * Author: rlcancian
 * 
 * Created on 02/11/2023
 */

#include "Smart_MarkovChain.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/MarkovChain.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_MarkovChain::Smart_MarkovChain() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_MarkovChain::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Variable* probTransition = plugins->newInstance<Variable>(model);
	probTransition->setInitialValues({	{"1,1",0.3}, {"1,2",0.6}, {"1,3",0.1},
										{"2,1",0.1}, {"2,2",0.1}, {"2,2",0.8},
										{"3,1",0.1}, {"3,2",0.1}, {"3,2",0.8},
									 });
	Variable* initialDistribution = plugins->newInstance<Variable>(model);
	initialDistribution->setInitialValues({ {"1",0.33}, {"2", 0.33}, {"3", 0.33} });
	MarkovChain* markov1 = plugins->newInstance<MarkovChain>(model);
	markov1->setTransitionProbabilityMatrix(probTransition);
	markov1->setInitialDistribution(initialDistribution);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnections()->insert(markov1);
	markov1->getConnections()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->save("./models/Smart_MarkovChain.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

