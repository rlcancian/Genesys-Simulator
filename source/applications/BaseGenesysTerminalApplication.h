/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   BaseGenesysTerminalApplication.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 16:25
 */

#ifndef BaseGenesysTerminalApplication_H
#define BaseGenesysTerminalApplication_H

#include "GenesysApplication_if.h"
#include "../kernel/simulator/TraceManager.h"
#include "../kernel/simulator/OnEventManager.h"
#include "../kernel/simulator/PluginManager.h"
#include "../kernel/simulator/Model.h"

#include <thread>

class BaseGenesysTerminalApplication : public GenesysApplication_if {
public:
	BaseGenesysTerminalApplication();
	virtual ~BaseGenesysTerminalApplication() = default;
public:
	virtual int main(int argc, char** argv) override;
	virtual void Simulate(Simulator* genesys, Model* model, PluginManager *plugins);
public:
	void setDefaultTraceHandlers(TraceManager* tm);
	void setDefaultEventHandlers(OnEventManager* oem); /*!< Add default method to handle simulator events so information is plotted to output screen */
protected:
	// default Trace Handlers
	virtual void traceHandler(TraceEvent e);
	virtual void traceErrorHandler(TraceErrorEvent e);
	virtual void traceReportHandler(TraceEvent e);
	virtual void traceSimulationHandler(TraceSimulationEvent e);
	// default Event Handlers
	virtual void onAfterProcessEventHandler(SimulationEvent* re);
	virtual void onBreakpointHandler(SimulationEvent* re);
	virtual void onEntityCreateHandler(SimulationEvent* re);
	virtual void onEntityMoveHandler(SimulationEvent* re);
	virtual void onSimulationStartHandler(SimulationEvent* re);
	virtual void onReplicationStepHandler(SimulationEvent* re);
	virtual void onReplicationStartHandler(SimulationEvent* re);
	virtual void onProcessEventHandler(SimulationEvent* re);
	virtual void onReplicationEndHandler(SimulationEvent* re);
	virtual void onSimulationEndHandler(SimulationEvent* re);
	virtual void onSimulationPausedHandler(SimulationEvent* re);
	virtual void onSimulationResumeHandler(SimulationEvent* re);
	virtual void onEntityRemoveHandler(SimulationEvent* re);
private:

};

#endif /* BaseGenesysTerminalApplication_H */

