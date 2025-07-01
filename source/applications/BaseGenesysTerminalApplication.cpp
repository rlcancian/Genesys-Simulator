/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   BaseGenesysTerminalApplication.cpp
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 16:25
 */

#include "BaseGenesysTerminalApplication.h"
#include "../kernel/simulator/Simulator.h"
//#include "../kernel/simulator/Model.h"
//#include "../kernel/simulator/PluginManager.h"
#include "TraitsApp.h"

#include <iostream>


BaseGenesysTerminalApplication::BaseGenesysTerminalApplication() {
}

int BaseGenesysTerminalApplication::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	Simulate(genesys, model, plugins);
	delete genesys;
	return 0;
}

void BaseGenesysTerminalApplication::Simulate(Simulator* genesys, Model* model, PluginManager *plugins) {
	// in case it's not overriden
}


// default Trace Handlers

void BaseGenesysTerminalApplication::traceHandler(TraceEvent e) {
	std::cout << e.getText() << std::endl;
}

void BaseGenesysTerminalApplication::traceErrorHandler(TraceErrorEvent e) {
	std::cout << e.getText() << std::endl;
}

void BaseGenesysTerminalApplication::traceReportHandler(TraceEvent e) {
	std::cout << "" << e.getText() << "" << std::endl;
}

void BaseGenesysTerminalApplication::traceSimulationHandler(TraceSimulationEvent e) {
	std::cout << e.getText() << std::endl;
}


// default Event Handlers

void BaseGenesysTerminalApplication::onAfterProcessEventHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Event processed" << re->getCurrentEvent()->show() << std::endl;
}

void BaseGenesysTerminalApplication::onBreakpointHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Breakpoint occured at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void BaseGenesysTerminalApplication::onEntityCreateHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getEntityCreated()->getName() << " was created." << std::endl;
}

void BaseGenesysTerminalApplication::onEntityMoveHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getCurrentEvent()->getEntity()->getName() << " was moved to component" << re->getDestinationComponent()->getName() << std::endl;
}

void BaseGenesysTerminalApplication::onEntityRemoveHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Entity " << re->getCurrentEvent()->getEntity()->getName() << " was removed." << std::endl;
}

void BaseGenesysTerminalApplication::onProcessEventHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Processing event " << re->getCurrentEvent()->show() << std::endl;
}

void BaseGenesysTerminalApplication::onReplicationEndHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication " << re->getCurrentReplicationNumber() << " ending." << std::endl;
}

void BaseGenesysTerminalApplication::onReplicationStartHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication " << re->getCurrentReplicationNumber() << " starting." << std::endl;
}

void BaseGenesysTerminalApplication::onReplicationStepHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Replication step at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void BaseGenesysTerminalApplication::onSimulationStartHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation is starting" << std::endl;
}

void BaseGenesysTerminalApplication::onSimulationEndHandler(SimulationEvent* re) {
	std::cout << "Event (Handler) " << "Replication " << re->getCurrentReplicationNumber() << " ending." << std::endl;
}

void BaseGenesysTerminalApplication::onSimulationPausedHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation was paused at time " << std::to_string(re->getSimulatedTime()) << std::endl;
}

void BaseGenesysTerminalApplication::onSimulationResumeHandler(SimulationEvent* re) {
	std::cout << "(Event Handler) " << "Simulation was resumed." << std::endl;
}

void BaseGenesysTerminalApplication::setDefaultEventHandlers(OnEventManager* oem) {
	oem->addOnBreakpointHandler(this, &BaseGenesysTerminalApplication::onBreakpointHandler);
	oem->addOnEntityCreateHandler(this, &BaseGenesysTerminalApplication::onEntityCreateHandler);
	oem->addOnEntityMoveHandler(this, &BaseGenesysTerminalApplication::onEntityMoveHandler);
	oem->addOnEntityRemoveHandler(this, &BaseGenesysTerminalApplication::onEntityRemoveHandler);
	oem->addOnProcessEventHandler(this, &BaseGenesysTerminalApplication::onProcessEventHandler);
	oem->addOnReplicationEndHandler(this, &BaseGenesysTerminalApplication::onReplicationEndHandler);
	oem->addOnReplicationStartHandler(this, &BaseGenesysTerminalApplication::onReplicationStartHandler);
	oem->addOnReplicationStepHandler(this, &BaseGenesysTerminalApplication::onReplicationStepHandler);
	oem->addOnSimulationEndHandler(this, &BaseGenesysTerminalApplication::onSimulationEndHandler);
	oem->addOnSimulationPausedHandler(this, &BaseGenesysTerminalApplication::onSimulationPausedHandler);
	oem->addOnSimulationResumeHandler(this, &BaseGenesysTerminalApplication::onSimulationResumeHandler);
	oem->addOnSimulationStartHandler(this, &BaseGenesysTerminalApplication::onSimulationStartHandler);
}

void BaseGenesysTerminalApplication::setDefaultTraceHandlers(TraceManager* tm) {
	tm->addTraceHandler<BaseGenesysTerminalApplication>(this, &BaseGenesysTerminalApplication::traceHandler);
	tm->addTraceErrorHandler<BaseGenesysTerminalApplication>(this, &BaseGenesysTerminalApplication::traceErrorHandler);
	tm->addTraceReportHandler<BaseGenesysTerminalApplication>(this, &BaseGenesysTerminalApplication::traceReportHandler);
	tm->addTraceSimulationHandler<BaseGenesysTerminalApplication>(this, &BaseGenesysTerminalApplication::traceSimulationHandler);
}
