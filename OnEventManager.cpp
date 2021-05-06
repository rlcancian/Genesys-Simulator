/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   OnEventManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 7 de Novembro de 2018, 12:28
 */

#include "OnEventManager.h"

//using namespace GenesysKernel;

OnEventManager::OnEventManager() {
}

void OnEventManager::_addOnHandler(List<simulationEventHandler>* list, simulationEventHandler EventHandler) {
	if (list->find(EventHandler) == list->list()->end())
		list->insert(EventHandler);
}

void OnEventManager::addOnReplicationStartHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationStartHandlers, EventHandler);
}

void OnEventManager::addOnReplicationStepHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationStepHandlers, EventHandler);
}

void OnEventManager::addOnProcessEventHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onEntityMoveHandlers, EventHandler);
}

void OnEventManager::addOnEntityMoveHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onEntityMoveHandlers, EventHandler);
}

void OnEventManager::addOnReplicationEndHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationEndHandlers, EventHandler);
}

void OnEventManager::addOnSimulationStartHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationStartHandlers, EventHandler);
}

void OnEventManager::addOnSimulationPausedStartHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationPausedStartHandlers, EventHandler);
}

void OnEventManager::addOnSimulationEndHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationEndHandlers, EventHandler);
}

void OnEventManager::addOnBreakpointHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onBreakpointHandlers, EventHandler);
}

void OnEventManager::_NotifyHandlers(List<simulationEventHandler>* list, SimulationEvent* se) {
	for (std::list<simulationEventHandler>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		(*it)(se);
	}
}

void OnEventManager::_NotifyHandlerMethods(List<simulationEventHandlerMethod>* list, SimulationEvent* se) {
	for (std::list<simulationEventHandlerMethod>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		(*it)(se);
	}
}

void OnEventManager::NotifyReplicationStartHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationStartHandlers, se);

}

void OnEventManager::NotifyReplicationStepHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationStepHandlers, se);
}

void OnEventManager::NotifyReplicationEndHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationEndHandlers, se);
}

void OnEventManager::NotifyEntityMoveHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onEntityMoveHandlers, se);
}

void OnEventManager::NotifyProcessEventHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onProcessEventHandlers, se);
	this->_NotifyHandlerMethods(this->_onProcessEventHandlerMethods, se);
}

void OnEventManager::NotifySimulationStartHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationStartHandlers, se);
}

void OnEventManager::NotifySimulationPausedStartHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationPausedStartHandlers, se);
}

void OnEventManager::NotifySimulationEndHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationEndHandlers, se);
}

void OnEventManager::NotifyBreakpointHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onBreakpointHandlers, se);
}