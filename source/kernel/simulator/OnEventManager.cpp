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

// Releases all heap-allocated listener containers while preserving handler registration semantics.
OnEventManager::~OnEventManager() {
	delete _onModelCheckSuccessHandlers;
	delete _onModelLoadHandlers;
	delete _onModelSaveHandlers;
	delete _onReplicationStartHandlers;
	delete _onReplicationStepHandlers;
	delete _onReplicationEndHandlers;
	delete _onProcessEventHandlers;
	delete _onAfterProcessEventHandlers;
	delete _onEntityCreateHandlers;
	delete _onEntityMoveHandlers;
	delete _onEntityRemoveHandlers;
	delete _onSimulationStartHandlers;
	delete _onSimulationPausedHandlers;
	delete _onSimulationResumeHandlers;
	delete _onSimulationEndHandlers;
	delete _onBreakpointHandlers;

	delete _onModelCheckSuccessHandlerMethods;
	delete _onModelLoadHandlerMethods;
	delete _onModelSaveHandlerMethods;
	delete _onReplicationStartHandlerMethods;
	delete _onReplicationStepHandlerMethods;
	delete _onReplicationEndHandlerMethods;
	delete _onProcessEventHandlerMethods;
	delete _onAfterProcessEventHandlerMethods;
	delete _onEntityCreateHandlerMethods;
	delete _onEntityMoveHandlerMethods;
	delete _onEntityRemoveHandlerMethods;
	delete _onSimulationStartHandlerMethods;
	delete _onSimulationPausedHandlerMethods;
	delete _onSimulationResumeHandlerMethods;
	delete _onSimulationEndHandlerMethods;
	delete _onBreakpointHandlerMethods;
}

void OnEventManager::_addOnHandler(List<simulationEventHandler>* list, simulationEventHandler EventHandler) {
	if (list->find(EventHandler) == list->list()->end())
		list->insert(EventHandler);
}

void OnEventManager::_addOnHandler(List<modelEventHandler>* list, modelEventHandler EventHandler) {
	if (list->find(EventHandler) == list->list()->end())
		list->insert(EventHandler);
}

void OnEventManager::addOnModelCheckSucessHandler(modelEventHandler EventHandler) {
	_addOnHandler(_onModelCheckSuccessHandlers, EventHandler);
}

void OnEventManager::addOnModelLoadHandler(modelEventHandler EventHandler) {
	_addOnHandler(_onModelLoadHandlers, EventHandler);
}

void OnEventManager::addOnModelSaveHandler(modelEventHandler EventHandler) {
	_addOnHandler(_onModelSaveHandlers, EventHandler);
}


void OnEventManager::addOnReplicationStartHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationStartHandlers, EventHandler);
}

void OnEventManager::addOnReplicationStepHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationStepHandlers, EventHandler);
}

void OnEventManager::addOnProcessEventHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onProcessEventHandlers, EventHandler);
}

void OnEventManager::addOnAfterProcessEventHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onAfterProcessEventHandlers, EventHandler);
}

void OnEventManager::addOnEntityCreateHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onEntityCreateHandlers, EventHandler);
}

void OnEventManager::addOnEntityMoveHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onEntityMoveHandlers, EventHandler);
}

void OnEventManager::addOnEntityRemoveHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onEntityRemoveHandlers, EventHandler);
}

void OnEventManager::addOnReplicationEndHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onReplicationEndHandlers, EventHandler);
}

void OnEventManager::addOnSimulationStartHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationStartHandlers, EventHandler);
}

void OnEventManager::addOnSimulationPausedHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationPausedHandlers, EventHandler);
}

void OnEventManager::addOnSimulationResumeHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationResumeHandlers, EventHandler);
}

void OnEventManager::addOnSimulationEndHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onSimulationEndHandlers, EventHandler);
}

void OnEventManager::addOnBreakpointHandler(simulationEventHandler EventHandler) {
	_addOnHandler(_onBreakpointHandlers, EventHandler);
}

void OnEventManager::_NotifyHandlers(List<simulationEventHandler>* list, SimulationEvent* se) {
	for (auto handler : *list->list()) {
		handler(se);
	}
}

void OnEventManager::_NotifyHandlerMethods(List<simulationEventHandlerMethod>* list, SimulationEvent* se) {
	for (auto& handler : *list->list()) {
		handler(se);
	}
}

void OnEventManager::_NotifyHandlers(List<modelEventHandler>* list, ModelEvent* se) {
	for (auto handler : *list->list()) {
		handler(se);
	}
}

void OnEventManager::_NotifyHandlerMethods(List<modelEventHandlerMethod>* list, ModelEvent* se) {
	for (auto& handler : *list->list()) {
		handler(se);
	}
}



void OnEventManager::NotifyModelCheckSuccessHandlers(ModelEvent* se) {
	this->_NotifyHandlers(this->_onModelCheckSuccessHandlers, se);
	this->_NotifyHandlerMethods(this->_onModelCheckSuccessHandlerMethods, se);
}

void OnEventManager::NotifyModelLoadHandlers(ModelEvent* se) {
	this->_NotifyHandlers(this->_onModelLoadHandlers, se);
	this->_NotifyHandlerMethods(this->_onModelLoadHandlerMethods, se);
}

void OnEventManager::NotifyModelSaveHandlers(ModelEvent* se) {
	this->_NotifyHandlers(this->_onModelSaveHandlers, se);
	this->_NotifyHandlerMethods(this->_onModelSaveHandlerMethods, se);
}

void OnEventManager::NotifyReplicationStartHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationStartHandlers, se);
	this->_NotifyHandlerMethods(this->_onReplicationStartHandlerMethods, se);
}

void OnEventManager::NotifyReplicationStepHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationStepHandlers, se);
	this->_NotifyHandlerMethods(this->_onReplicationStepHandlerMethods, se);
}

void OnEventManager::NotifyReplicationEndHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onReplicationEndHandlers, se);
	this->_NotifyHandlerMethods(this->_onReplicationEndHandlerMethods, se);
}

void OnEventManager::NotifyEntityCreateHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onEntityCreateHandlers, se);
	this->_NotifyHandlerMethods(this->_onEntityCreateHandlerMethods, se);
}

void OnEventManager::NotifyEntityMoveHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onEntityMoveHandlers, se);
	this->_NotifyHandlerMethods(this->_onEntityMoveHandlerMethods, se);
}

void OnEventManager::NotifyEntityRemoveHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onEntityRemoveHandlers, se);
	this->_NotifyHandlerMethods(this->_onEntityRemoveHandlerMethods, se);
}

void OnEventManager::NotifyProcessEventHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onProcessEventHandlers, se);
	this->_NotifyHandlerMethods(this->_onProcessEventHandlerMethods, se);
}

void OnEventManager::NotifyAfterProcessEventHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onAfterProcessEventHandlers, se);
	this->_NotifyHandlerMethods(this->_onAfterProcessEventHandlerMethods, se);
}

void OnEventManager::NotifySimulationStartHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationStartHandlers, se);
	this->_NotifyHandlerMethods(this->_onSimulationStartHandlerMethods, se);
}

void OnEventManager::NotifySimulationPausedHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationPausedHandlers, se);
	this->_NotifyHandlerMethods(this->_onSimulationPausedHandlerMethods, se);
}

void OnEventManager::NotifySimulationResumeHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationResumeHandlers, se);
	this->_NotifyHandlerMethods(this->_onSimulationResumeHandlerMethods, se);
}

void OnEventManager::NotifySimulationEndHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onSimulationEndHandlers, se);
	this->_NotifyHandlerMethods(this->_onSimulationEndHandlerMethods, se);
}

void OnEventManager::NotifyBreakpointHandlers(SimulationEvent* se) {
	this->_NotifyHandlers(this->_onBreakpointHandlers, se);
	this->_NotifyHandlerMethods(this->_onBreakpointHandlerMethods, se);
}
