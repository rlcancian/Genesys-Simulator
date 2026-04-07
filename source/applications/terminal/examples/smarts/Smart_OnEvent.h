/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_OnEvent.h
 * Author: rlcancian
 *
 * Created on 5 de Janeiro de 2021, 18:00
 */

#ifndef SMART_ONEVENT_H
#define SMART_ONEVENT_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_OnEvent : public BaseGenesysTerminalApplication {
public:
	Smart_OnEvent();
public:
	virtual int main(int argc, char** argv) override;
public:
	virtual void onBreakpointHandler(SimulationEvent* re) override;
	virtual void onEntityCreateHandler(SimulationEvent* re) override;
	virtual void onEntityMoveHandler(SimulationEvent* re) override;
	virtual void onSimulationStartHandler(SimulationEvent* re) override;
	virtual void onReplicationStartHandler(SimulationEvent* re) override;
	virtual void onReplicationStepHandler(SimulationEvent* re) override;
	virtual void onProcessEventHandler(SimulationEvent* re) override;
	virtual void onReplicationEndHandler(SimulationEvent* re) override;
	virtual void onSimulationEndHandler(SimulationEvent* re) override;
	virtual void onSimulationPausedHandler(SimulationEvent* re) override;
	virtual void onSimulationResumeHandler(SimulationEvent* re) override;
	virtual void onEntityRemoveHandler(SimulationEvent* re) override;
};

#endif /* SMART_ONEVENT_H */
