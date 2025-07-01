/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   Smart_WaitScanCondition.cpp
 * Author: rlcancian
 *
 * Created on 14 de junho de 2022, 13:00
 */

#include "Smart_WaitScanCondition.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

// Model data definitions

Smart_WaitScanCondition::Smart_WaitScanCondition() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_WaitScanCondition::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setTimeBetweenCreationsExpression("1");
	Create* create2 = plugins->newInstance<Create>(model);
	create2->setTimeBetweenCreationsExpression("2");
	Wait* wait1 = plugins->newInstance<Wait>(model);
	wait1->setWaitType(Wait::WaitType::ScanForCondition);
	wait1->setCondition("mod(var1,3)==0");
	Assign* assign1 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "att1", "var1", true));
	assign1->getAssignments()->insert(new Assignment(model, "var1", "var1+1", false));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	//
	create1->getConnections()->insert(wait1);
	wait1->getConnections()->insert(dispose1);
	create2->getConnections()->insert(assign1);
	assign1->getConnections()->insert(dispose1);
	//
	ModelSimulation* simulation = model->getSimulation();
	simulation->setReplicationLength(20);
	//
	model->getTracer()->setTraceLevel(TraceManager::Level::L8_detailed);
	model->save("./models/Smart_WaitScanCondition.gen");
	simulation->start();
	delete genesys;
	return 0;
};

