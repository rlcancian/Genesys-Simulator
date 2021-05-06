/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TraceManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 7 de Novembro de 2018, 11:59
 */

#include "TraceManager.h"
#include "Traits.h"

//using namespace GenesysKernel;

TraceManager::TraceManager(Simulator* simulator) {//(Model* model) {
	_simulator = simulator;
	_traceLevel = Traits<Model>::traceLevel;
}

void TraceManager::setTraceLevel(Util::TraceLevel _traceLevel) {
	this->_traceLevel = _traceLevel;
}

Util::TraceLevel TraceManager::getTraceLevel() const {
	return _traceLevel;
}

Simulator* TraceManager::getParentSimulator() const {
	return _simulator;
}

void TraceManager::addTraceHandler(traceListener traceListener) {
	this->_traceHandlers->insert(traceListener);
}

void TraceManager::addTraceSimulationHandler(traceSimulationListener traceSimulationListener) {
	this->_traceSimulationHandlers->insert(traceSimulationListener);
}

void TraceManager::addTraceErrorHandler(traceErrorListener traceErrorListener) {
	this->_traceErrorHandlers->insert(traceErrorListener);
}

void TraceManager::addTraceReportHandler(traceListener traceReportListener) {
	this->_traceReportHandlers->insert(traceReportListener);
}

void TraceManager::trace(Util::TraceLevel level, std::string text) {
	trace(text, level);
}

void TraceManager::trace(std::string text, Util::TraceLevel level) {
	if (_traceConditionPassed(level)) {
		//text = std::to_string(static_cast<int> (level)) + ". " + Util::Indent() + text;
		text = Util::Indent() + text;
		TraceEvent e = TraceEvent(text, level);
		/*  \todo:--: somewhere in future it should be interesting to use "auto" and c++17 at least */
		for (std::list<traceListener>::iterator it = this->_traceHandlers->list()->begin(); it != _traceHandlers->list()->end(); it++) {
			(*it)(e);
		}
		for (std::list<traceListenerMethod>::iterator it = this->_traceHandlersMethod->list()->begin(); it != _traceHandlersMethod->list()->end(); it++) {
			(*it)(e);
		}

	}
}

void TraceManager::traceError(std::exception e, std::string text) {
	traceError(text, e);
}

void TraceManager::traceError(std::string text, std::exception e) {
	text = Util::Indent() + text;
	TraceErrorEvent exceptEvent = TraceErrorEvent(text, e);
	/*  \todo:--: somewhere in future it should be interesting to use "auto" and c++17 at least */
	for (std::list<traceErrorListener>::iterator it = this->_traceErrorHandlers->list()->begin(); it != _traceErrorHandlers->list()->end(); it++) {
		(*it)(exceptEvent);
	}
	for (std::list<traceErrorListenerMethod>::iterator it = this->_traceErrorHandlersMethod->list()->begin(); it != _traceErrorHandlersMethod->list()->end(); it++) {
		(*it)(exceptEvent);
	}
}

void TraceManager::traceSimulation(Util::TraceLevel level, double time, Entity* entity, ModelComponent* component, std::string text) {
	traceSimulation(time, entity, component, text, level);
}

void TraceManager::traceSimulation(double time, Entity* entity, ModelComponent* component, std::string text, Util::TraceLevel level) {
	if (_traceConditionPassed(level)) {
		text = Util::Indent() + text;
		TraceSimulationEvent e = TraceSimulationEvent(level, time, entity, component, text);
		for (std::list<traceSimulationListener>::iterator it = this->_traceSimulationHandlers->list()->begin(); it != _traceSimulationHandlers->list()->end(); it++) {
			(*it)(e);
		}
		for (std::list<traceSimulationListenerMethod>::iterator it = this->_traceSimulationHandlersMethod->list()->begin(); it != _traceSimulationHandlersMethod->list()->end(); it++) {
			(*it)(e);
		}
	}
}

void TraceManager::traceReport(Util::TraceLevel level, std::string text) {
	traceReport(text, level);
}

void TraceManager::traceReport(std::string text, Util::TraceLevel level) {
	if (_traceConditionPassed(level)) {
		text = Util::Indent() + text;
		TraceEvent e = TraceEvent(text, level);
		for (std::list<traceListener>::iterator it = this->_traceReportHandlers->list()->begin(); it != _traceReportHandlers->list()->end(); it++) {
			(*it)(e);
		}
		for (std::list<traceListenerMethod>::iterator it = this->_traceReportHandlersMethod->list()->begin(); it != _traceReportHandlersMethod->list()->end(); it++) {
			(*it)(e);
		}
	}
}

List<std::string>* TraceManager::errorMessages() const {
	return _errorMessages;
}

bool TraceManager::_traceConditionPassed(Util::TraceLevel level) {
	return /*this->_debugged &&*/ static_cast<int> (this->_traceLevel) >= static_cast<int> (level);
}
