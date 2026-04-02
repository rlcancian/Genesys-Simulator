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
	void onBreakpointHandler(SimulationEvent* re);
	void onEntityCreateHandler(SimulationEvent* re);
	void onEntityMoveHandler(SimulationEvent* re);
	void onSimulationStartHandler(SimulationEvent* re);
	void onReplicationStartHandler(SimulationEvent* re);
	void onReplicationStepHandler(SimulationEvent* re);
	void onProcessEventHandler(SimulationEvent* re);
	void onReplicationEndHandler(SimulationEvent* re);
	void onSimulationEndHandler(SimulationEvent* re);
	void onSimulationPausedHandler(SimulationEvent* re);
	void onSimulationResumeHandler(SimulationEvent* re);
	void onEntityRemoveHandler(SimulationEvent* re);
};

#endif /* SMART_ONEVENT_H */

