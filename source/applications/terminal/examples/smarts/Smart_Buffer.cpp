/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Buffer.cpp
 * Author: rlcancian
 * 
 * Created on
 */

#include "Smart_Buffer.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Buffer.h"
#include "../../../../plugins/components/Signal.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Buffer::Smart_Buffer() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Buffer::main(int argc, char** argv) {

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Buffer* buffer1 = plugins->newInstance<Buffer>(model);
	buffer1->setCapacity(5);
	buffer1->setAdvanceOn(Buffer::AdvanceOn::NewArrivals);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);


	Create* create4 = plugins->newInstance<Create>(model);
	Buffer* buffer4 = plugins->newInstance<Buffer>(model);
	buffer4->setCapacity(5);
	buffer4->setAdvanceOn(Buffer::AdvanceOn::Signal);
	SignalData *signalData = plugins->newInstance<SignalData>(model);
	buffer4->setSignal(signalData);
	buffer4->setArrivalOnFullBufferRule(Buffer::ArrivalOnFullBufferRule::Dispose);

	Create* create5 = plugins->newInstance<Create>(model);
	Signal* signal5 = plugins->newInstance<Signal>(model);
	signal5->setSignalData(signalData);

	// connect model components to create a "workflow"
	create1->getConnections()->insert(buffer1);
	buffer1->getConnections()->insert(dispose1);
	create4->getConnections()->insert(buffer4);
	buffer4->getConnections()->insert(dispose1);
	create5->getConnections()->insert(signal5);
	signal5->getConnections()->insert(dispose1);

	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(1);
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>30");
	model->save("./models/Smart_Buffer.gen");
	model->getSimulation()->start();
	delete genesys;

	return 0;
};

