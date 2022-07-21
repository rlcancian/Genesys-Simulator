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
#include <iostream>

#include "../kernel/simulator/Simulator.h"

BaseGenesysTerminalApplication::BaseGenesysTerminalApplication() {
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

void BaseGenesysTerminalApplication::insertFakePluginsByHand(Simulator* simulator) {
	PluginManager* pm = simulator->getPlugins();
	// TRYING SOME NEW ORGANIZATION (BASED ON ARENA 16..20)

	// DISCRETE PROCESSING

	// GROUPING

	//INPUT/OUTPUT

	// MATERIAL HANDLING

	// CONTINUOS

	// DATA DEFINITION


	//-----------------------------------------------------

	// OLD ORGANIZATION

	// model components
	// arena basic process
	pm->insert("create.so");
	pm->insert("dispose.so");
	pm->insert("decide.so");
	pm->insert("batch.so");
	pm->insert("separate.so");
	pm->insert("assign.so");
	pm->insert("record.so");
	pm->insert("process.so");
	pm->insert("submodel.so");
	pm->insert("entitygroup.so");
	pm->insert("queue.so");
	pm->insert("set.so");
	pm->insert("resource.so");
	pm->insert("variable.so");
	pm->insert("schedule.so");
	pm->insert("entitygroup.so");
	// arena advanced process
	pm->insert("delay.so");
	pm->insert("dropoff.so");
	pm->insert("hold.so");
	pm->insert("match.so");
	pm->insert("pickup.so");
	pm->insert("read.so");
	pm->insert("write.so");
	pm->insert("release.so");
	pm->insert("remove.so");
	pm->insert("seize.so");
	pm->insert("search.so");
	pm->insert("signal.so");
	pm->insert("signaldata.so");
	pm->insert("store.so");
	pm->insert("unstore.so");
	pm->insert("expression.so");
	pm->insert("failure.so");
	pm->insert("file.so");
	pm->insert("storage.so");
	// arena transfer station
	pm->insert("enter.so");
	pm->insert("leave.so");
	pm->insert("pickstation.so");
	pm->insert("route.so");
	pm->insert("sequence.so");
	pm->insert("station.so");
	pm->insert("label.so");
	// arena transfer conveyour
	pm->insert("access.so");
	pm->insert("exit.so");
	pm->insert("start.so");
	pm->insert("stop.so");
	pm->insert("conveyour.so");
	pm->insert("segment.so");
	// arena transfer transport
	pm->insert("alocate.so");
	pm->insert("free.so");
	pm->insert("halt.so");
	pm->insert("move.so");
	pm->insert("request.so");
	pm->insert("transporter.so");
	pm->insert("distance.so");
	pm->insert("network.so");
	pm->insert("networklink.so");
	// others
	pm->insert("dummy.so");
	pm->insert("lsode.so");
	pm->insert("biochemical.so");
	pm->insert("markovchain.so");
	pm->insert("cellularautomata.so");
	pm->insert("cppforg.so");
	pm->insert("fsm.so");
	pm->insert("fsmstate.so");
	pm->insert("fsmtransition.so");
	// now complete the information
	simulator->getPlugins()->completePluginsFieldsAndTemplates();
}
