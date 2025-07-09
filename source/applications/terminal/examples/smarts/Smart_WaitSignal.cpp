/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_WaitSignal.cpp
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_WaitSignal.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Signal.h"
#include "../../../../plugins/data/SignalData.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

// Model data definitions

Smart_WaitSignal::Smart_WaitSignal() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_WaitSignal::main(int argc, char** argv) {
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
	Create* create3 = plugins->newInstance<Create>(model);
	create3->setTimeBetweenCreationsExpression("5");
	create3->setFirstCreation(5);
	Create* create4 = plugins->newInstance<Create>(model);
	create4->setTimeBetweenCreationsExpression("15");
	create4->setFirstCreation(15);

	SignalData* sigdata1 = plugins->newInstance<SignalData>(model, "sinalD1");
	SignalData* sigdata2 = plugins->newInstance<SignalData>(model, "sinalD2");
	Wait* wait1 = plugins->newInstance<Wait>(model);
	wait1->setSignalData(sigdata1);
	Wait* wait2 = plugins->newInstance<Wait>(model);
	wait2->setSignalData(sigdata2);
	Signal* signal1 = plugins->newInstance<Signal>(model);
	signal1->setSignalData(sigdata1);
	signal1->setLimitExpression("3");
	Signal* signal2 = plugins->newInstance<Signal>(model);
	signal2->setSignalData(sigdata2);
	signal2->setLimitExpression("1e3");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	Dispose* dispose2 = plugins->newInstance<Dispose>(model);
	//
	create1->getConnectionManager()->insert(wait1);
	wait1->getConnectionManager()->insert(dispose1);
	create2->getConnectionManager()->insert(wait2);
	wait2->getConnectionManager()->insert(dispose1);
	create3->getConnectionManager()->insert(signal1);
	signal1->getConnectionManager()->insert(dispose2);
	create4->getConnectionManager()->insert(signal2);
	signal2->getConnectionManager()->insert(dispose2);
	//
	ModelSimulation* simulation = model->getSimulation();
	simulation->setReplicationLength(20);
	//
	model->getTracer()->setTraceLevel(TraceManager::Level::L8_detailed);
	model->save("./models/Smart_WaitSignal.gen");
	do {
		simulation->start();
		//std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	} while (simulation->isPaused());
	delete genesys;
	return 0;
};

