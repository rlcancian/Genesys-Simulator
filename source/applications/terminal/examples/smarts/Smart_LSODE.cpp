/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_LSODE.cpp
 * Author: rlcancian
 * 
 * Created on 11 de janeiro de 2022, 19:29
 */

#include "Smart_LSODE.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/LSODE.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/data/Variable.h"
#include "../../../TraitsApp.h"

Smart_LSODE::Smart_LSODE() {
}

int Smart_LSODE::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setTimeBetweenCreationsExpression("0.5");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// define variables for the ordinary differential equations
	Variable* varx = plugins->newInstance<Variable>(model, "x");
	varx->insertDimentionSize(2);
	varx->setInitialValue(1.0, "0"); //x[0] = 1.0
	varx->setInitialValue(0.0, "1"); //x[1] = 0.0
	Variable* vart = plugins->newInstance<Variable>(model, "t");
	LSODE* ode1 = plugins->newInstance<LSODE>(model);
	ode1->setVariable(varx);
	ode1->setTimeVariable(vart);
	ode1->getDiffEquations()->insert("x[1]");
	ode1->getDiffEquations()->insert("x[0] + exp(t)");
	ode1->setStep(0.1);
	ode1->setFilename("./temp/Smart_LSODE.outputdatafile.txt"); // ODE results in a text tabular format
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(ode1);
	ode1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(2.0);
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(false);
	model->save("./models/Smart_LSODE.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;

}
