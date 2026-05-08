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
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/Logic/Create.h"
#include "plugins/components/AnalyticalModeling/MarkovChain.h"
#include "plugins/components/Logic/Dispose.h"
#include "../../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "plugins/data/Logic/Variable.h"
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
	plugins->autoInsertPlugins();
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Variable* probTransition = plugins->newInstance<Variable>(model, "MarkovTransitionMatrix");
	probTransition->setInitialValuesText("[0.1 0.7 0.1 0.1; 0.1 0.1 0.7 0.1; 0.1 0.1 0.1 0.7; 0.7 0.1 0.1 0.1]");
	Variable* initialDistribution = plugins->newInstance<Variable>(model, "MarkovInitialDistribution");
	initialDistribution->setInitialValuesText("[0.25 0.25 0.25 0.25]");
	Variable* currentState = plugins->newInstance<Variable>(model, "CurrentMarkovState");
	currentState->setInitialValue(0.0);
	MarkovChain* markov1 = plugins->newInstance<MarkovChain>(model);
	markov1->setTransitionProbabilityMatrix(probTransition);
	markov1->setInitialDistribution(initialDistribution);
	markov1->setCurrentState(currentState);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(markov1);
	markov1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->save("./models/Smart_MarkovChain.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
