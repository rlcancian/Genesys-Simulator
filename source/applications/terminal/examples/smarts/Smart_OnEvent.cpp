/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_OnEvent.cpp
 * Author: rlcancian
 * 
 * Created on 5 de Janeiro de 2021, 18:00
 */

#include "Smart_OnEvent.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_OnEvent::Smart_OnEvent() {
}

void Smart_OnEvent::onBreakpointHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Breakpoint occured at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void Smart_OnEvent::onEntityCreateHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getEntityCreated()->getName() << " was created." << std::endl;
}

void Smart_OnEvent::onEntityMoveHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getCurrentEvent()->getEntity()->getName() << " was moved to component" << re->getDestinationComponent()->getName() << std::endl;
}

void Smart_OnEvent::onEntityRemoveHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getCurrentEvent()->getEntity()->getName() << " was removed." << std::endl;
}

void Smart_OnEvent::onProcessEventHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Processing event " << re->getCurrentEvent()->show() << std::endl;
}

void Smart_OnEvent::onReplicationEndHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication " << re->getCurrentReplicationNumber() << " ending." << std::endl;
}

void Smart_OnEvent::onReplicationStartHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication " << re->getCurrentReplicationNumber() << " starting." << std::endl;
}

void Smart_OnEvent::onReplicationStepHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication step at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void Smart_OnEvent::onSimulationStartHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation is starting" << std::endl;
}

void Smart_OnEvent::onSimulationEndHandler(SimulationEvent* re) {
	std::cout << "Event (Handler) " << "Replication " << re->getCurrentReplicationNumber() << " ending." << std::endl;
}

void Smart_OnEvent::onSimulationPausedHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation was paused at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void Smart_OnEvent::onSimulationResumeHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation was resumed." << std::endl;
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_OnEvent::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	model->getTracer()->setTraceLevel(TraceManager::Level::L0_noTraces); // NO TRACES. Genesys will show anything!
	// set event handler to the previous methods. All outputs will come from these handlers
	OnEventManager* oem = model->getOnEventManager();
	oem->addOnBreakpointHandler(this, &Smart_OnEvent::onBreakpointHandler);
	oem->addOnEntityCreateHandler(this, &Smart_OnEvent::onEntityCreateHandler);
	oem->addOnEntityMoveHandler(this, &Smart_OnEvent::onEntityMoveHandler);
	oem->addOnEntityRemoveHandler(this, &Smart_OnEvent::onEntityRemoveHandler);
	oem->addOnProcessEventHandler(this, &Smart_OnEvent::onProcessEventHandler);
	oem->addOnReplicationEndHandler(this, &Smart_OnEvent::onReplicationEndHandler);
	oem->addOnReplicationStartHandler(this, &Smart_OnEvent::onReplicationStartHandler);
	oem->addOnReplicationStepHandler(this, &Smart_OnEvent::onReplicationStepHandler);
	oem->addOnSimulationEndHandler(this, &Smart_OnEvent::onSimulationEndHandler);
	oem->addOnSimulationPausedHandler(this, &Smart_OnEvent::onSimulationPausedHandler);
	oem->addOnSimulationResumeHandler(this, &Smart_OnEvent::onSimulationResumeHandler);
	oem->addOnSimulationStartHandler(this, &Smart_OnEvent::onSimulationStartHandler);
	// create model 
	Create* create1 = plugins->newInstance<Create>(model);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate. It will pause every step and require to press ENTER
	model->getSimulation()->setReplicationLength(10);
	model->save("./models/Smart_OnEvent.gen");
	do {
		model->getSimulation()->step();
		std::cout << "Press ENTER to continue...";
		//std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	} while (model->getSimulation()->isPaused());
	delete genesys;
	return 0;
};

