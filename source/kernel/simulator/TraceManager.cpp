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
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

TraceManager::TraceManager(Simulator* simulator) {//(Model* model) {
	_simulator = simulator;
    _traceLevel = TraitsKernel<Simulator>::traceLevel; // inherits the kernel trace leval
	_errorMessages = new List<std::string>();
	//@TODO: Tracelevels should be based on the tracelevel of each "class" 
}

void TraceManager::setTraceLevel(TraceManager::Level _traceLevel) {
	this->_traceLevel = _traceLevel;
}

TraceManager::Level TraceManager::getTraceLevel() const {
	return _traceLevel;
}

Simulator* TraceManager::getParentSimulator() const {
	return _simulator;
}

void TraceManager::setTraceSimulationRuleAllAllowed(bool _traceSimulationRuleAllAllowed) {
	this->_traceSimulationRuleAllAllowed = _traceSimulationRuleAllAllowed;
}

bool TraceManager::isTraceSimulationRuleAllAllowed() const {
	return _traceSimulationRuleAllAllowed;
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

void TraceManager::addTraceSimulationExceptionRuleModelData(void* thisobject) {
	if (_traceSimulationExceptionRule->find(thisobject) == _traceSimulationExceptionRule->list()->end()) {
		_traceSimulationExceptionRule->insert(thisobject);
	}
}

void TraceManager::trace(TraceManager::Level level, std::string text) {
	trace(text, level);
}

void TraceManager::trace(std::string text, TraceManager::Level level) {
	if (_traceConditionPassed(level)) {
		text = Util::Indent() + text;
		//text = "L" + std::to_string(static_cast<int> (level)) + "    " + Util::Indent() + text;
		TraceEvent e = TraceEvent(text, level);
		/*  @TODO:--: somewhere in future it should be interesting to use "auto" and c++17 at least */
		for (std::list<traceListener>::iterator it = this->_traceHandlers->list()->begin(); it != _traceHandlers->list()->end(); it++) {
			(*it)(e);
		}
		for (std::list<traceListenerMethod>::iterator it = this->_traceHandlersMethod->list()->begin(); it != _traceHandlersMethod->list()->end(); it++) {
			(*it)(e);
		}
	}
}

//void TraceManager::traceError(std::exception e, std::string text) {
//	traceError(text, e);
//}

//void TraceManager::traceError(TraceManager::Level level, std::string text) {
//	traceError(text, level);
//}

void TraceManager::traceError(std::string text, TraceManager::Level level) {
	text = Util::Indent() + text;
	TraceErrorEvent exceptEvent = TraceErrorEvent(text, level);
	for (std::list<traceErrorListener>::iterator it = this->_traceErrorHandlers->list()->begin(); it != _traceErrorHandlers->list()->end(); it++) {
		(*it)(exceptEvent);
	}
	for (std::list<traceErrorListenerMethod>::iterator it = this->_traceErrorHandlersMethod->list()->begin(); it != _traceErrorHandlersMethod->list()->end(); it++) {
		(*it)(exceptEvent);
	}
}

void TraceManager::traceError(std::string text, std::exception e) {
	text = Util::Indent() + text;
	TraceErrorEvent exceptEvent = TraceErrorEvent(text, e);
	/*  @TODO:--: somewhere in future it should be interesting to use "auto" and c++17 at least */
	for (std::list<traceErrorListener>::iterator it = this->_traceErrorHandlers->list()->begin(); it != _traceErrorHandlers->list()->end(); it++) {
		(*it)(exceptEvent);
	}
	for (std::list<traceErrorListenerMethod>::iterator it = this->_traceErrorHandlersMethod->list()->begin(); it != _traceErrorHandlersMethod->list()->end(); it++) {
		(*it)(exceptEvent);
	}
}

void TraceManager::traceSimulation(void* thisobject, TraceManager::Level level, std::string text) {
	traceSimulation(thisobject, text, level);
}

void TraceManager::traceSimulation(void* thisobject, std::string text, TraceManager::Level level) {
	if (_traceSimulationConditionPassed(level, thisobject)) {
		text = Util::Indent() + text;
		//text = "L" + std::to_string(static_cast<int> (level)) + "    " + Util::Indent() + text;
		TraceSimulationEvent e = TraceSimulationEvent(level, 0.0, nullptr, nullptr, text);
		/*  @TODO:--: somewhere in future it should be interesting to use "auto" and c++17 at least */
		for (std::list<traceSimulationListener>::iterator it = this->_traceSimulationHandlers->list()->begin(); it != _traceSimulationHandlers->list()->end(); it++) {
			(*it)(e);
		}
		for (std::list<traceSimulationListenerMethod>::iterator it = this->_traceSimulationHandlersMethod->list()->begin(); it != _traceSimulationHandlersMethod->list()->end(); it++) {
			(*it)(e);
		}
	}
}

void TraceManager::traceSimulation(void* thisobject, TraceManager::Level level, double time, Entity* entity, ModelComponent* component, std::string text) {
	traceSimulation(thisobject, time, entity, component, text, level);
}

void TraceManager::traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component, std::string text, TraceManager::Level level) {
	if (_traceSimulationConditionPassed(level, thisobject)) {
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

//void TraceManager::traceReport(TraceManager::Level level, std::string text) {
//	traceReport(text, level);
//}

void TraceManager::traceReport(std::string text, TraceManager::Level level) {
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

bool TraceManager::_traceConditionPassed(TraceManager::Level level) {
	return /*this->_debugged &&*/ static_cast<int> (this->_traceLevel) >= static_cast<int> (level);
}

bool TraceManager::_traceSimulationConditionPassed(TraceManager::Level level, void* thisobject) {
	bool result = _traceConditionPassed(level);
	bool isException = false;
	if (result) {
		isException = (_traceSimulationExceptionRule->find(thisobject) != _traceSimulationExceptionRule->list()->end());
	}
	result &= (_traceSimulationRuleAllAllowed && !isException) || (!_traceSimulationRuleAllAllowed && isException); // xor
	return result;
}
