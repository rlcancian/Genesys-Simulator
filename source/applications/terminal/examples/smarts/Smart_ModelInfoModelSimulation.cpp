/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ModelInfoModelSimulation.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:59
 */

#include "Smart_ModelInfoModelSimulation.h"
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ModelInfoModelSimulation::Smart_ModelInfoModelSimulation() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ModelInfoModelSimulation::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// set general info about the model
	ModelInfo* infos = model->getInfos();
	infos->setAnalystName("Your name");
	infos->setProjectTitle("The title of the project");
	infos->setDescription("This simulation model tests one of the most basic models possible.");
	infos->setVersion("1.0");
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setTimeUnit(Util::TimeUnit::minute);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("NORM(1,0.2)", Util::TimeUnit::minute);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(dispose1);
	// set model simulation
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(15, Util::TimeUnit::minute); // each replication will last 15 minutes (simulated time)
	sim->setNumberOfReplications(2); // replicates the simulation 2 times
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	sim->setWarmUpPeriod(1);
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setPauseOnReplication(true);
	sim->setPauseOnEvent(false);
	sim->setShowReportsAfterReplication(false);
	sim->setShowReportsAfterSimulation(true);
	sim->setStepByStep(true);
	sim ->setTerminatingCondition("");
	sim->getBreakpointsOnComponent()->insert(delay1);
	sim->getBreakpointsOnTime()->insert(6.5);
	sim->getBreakpointsOnTime()->insert(14.0);
	// save and simulate. Several pauses will occour due to previous settings
	model->save("./models/Smart_ModelInfoModelSimulation.gen");
	do {
		sim->start();
		//std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	} while (sim->isPaused());
	delete genesys;
	return 0;
};

