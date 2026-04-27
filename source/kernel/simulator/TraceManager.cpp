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

std::string TraceManager::convertEnumToStr(TraceManager::Level level) {
	switch (level) {
		case TraceManager::Level::L0_noTraces: return "No Traces";
		case TraceManager::Level::L1_errorFatal: return "Error Fatal";
		case TraceManager::Level::L2_results: return "Results";
		case TraceManager::Level::L3_errorRecover: return "Error Recover";
		case TraceManager::Level::L4_warning: return "Warning";
		case TraceManager::Level::L5_event: return "Event";
		case TraceManager::Level::L6_arrival: return "Arrival";
		case TraceManager::Level::L7_internal: return "Internal";
		case TraceManager::Level::L8_detailed: return "Detailed";
		case TraceManager::Level::L9_mostDetailed: return "Most Detailed";
		case TraceManager::Level::num_elements: return "";
	}
	return "";
}

TraceManager::TraceManager(Simulator* simulator) {//(Model* model) {
	_simulator = simulator;
    _traceLevel = TraitsKernel<Simulator>::traceLevel; // inherits the kernel trace leval
}

TraceManager::~TraceManager() {
	// Reuse shutdown path so callback vectors are neutralized before storage is released.
	beginShutdown();
	delete _traceHandlers;
	delete _traceErrorHandlers;
	delete _traceReportHandlers;
	delete _traceSimulationHandlers;
	delete _traceHandlersMethod;
	delete _traceErrorHandlersMethod;
	delete _traceReportHandlersMethod;
	delete _traceSimulationHandlersMethod;
	delete _traceSimulationExceptionRule;
	delete _errorMessages;
}

void TraceManager::beginShutdown() {
	// Mark tracer as shutting down and clear all callback targets to block late GUI invocations.
	_shuttingDown = true;
	_traceHandlers->clear();
	_traceErrorHandlers->clear();
	_traceReportHandlers->clear();
	_traceSimulationHandlers->clear();
	_traceHandlersMethod->clear();
	_traceErrorHandlersMethod->clear();
	_traceReportHandlersMethod->clear();
	_traceSimulationHandlersMethod->clear();
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

void TraceManager::trace(TraceManager::Level level, const std::string& text) {
	trace(text, level);
}

void TraceManager::trace(const std::string& text, TraceManager::Level level) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
	if (_traceConditionPassed(level)) {
		text = Util::Indent() + text;
		//text = "L" + std::to_string(static_cast<int> (level)) + "    " + Util::Indent() + text;
		TraceEvent e = TraceEvent(text, level);
		for (auto handler : *this->_traceHandlers->list()) {
			handler(e);
		}
		for (auto& handlerMethod : *this->_traceHandlersMethod->list()) {
			handlerMethod(e);
		}
	}
}

//void TraceManager::traceError(std::exception e, std::string text) {
//	traceError(text, e);
//}

//void TraceManager::traceError(TraceManager::Level level, std::string text) {
//	traceError(text, level);
//}

void TraceManager::traceError(const std::string& text, TraceManager::Level level) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
	text = Util::Indent() + text;
	_errorMessages->insert(text);
	TraceErrorEvent exceptEvent = TraceErrorEvent(text, level);
	for (auto handler : *this->_traceErrorHandlers->list()) {
		handler(exceptEvent);
	}
	for (auto& handlerMethod : *this->_traceErrorHandlersMethod->list()) {
		handlerMethod(exceptEvent);
	}
}

void TraceManager::traceError(const std::string& text, const std::exception& e) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
	text = Util::Indent() + text;
	_errorMessages->insert(text);
	TraceErrorEvent exceptEvent = TraceErrorEvent(text, e);
	for (auto handler : *this->_traceErrorHandlers->list()) {
		handler(exceptEvent);
	}
	for (auto& handlerMethod : *this->_traceErrorHandlersMethod->list()) {
		handlerMethod(exceptEvent);
	}
}

void TraceManager::traceSimulation(void* thisobject, TraceManager::Level level, const std::string& text) {
	traceSimulation(thisobject, text, level);
}

void TraceManager::traceSimulation(void* thisobject, const std::string& text, TraceManager::Level level, bool showAnyway) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
    if (_traceSimulationConditionPassed(level, thisobject,showAnyway)) {
		text = Util::Indent() + text;
		//text = "L" + std::to_string(static_cast<int> (level)) + "    " + Util::Indent() + text;
		TraceSimulationEvent e = TraceSimulationEvent(level, 0.0, nullptr, nullptr, text);
		for (auto handler : *this->_traceSimulationHandlers->list()) {
			handler(e);
		}
		for (auto& handlerMethod : *this->_traceSimulationHandlersMethod->list()) {
			handlerMethod(e);
		}
	}
}

void TraceManager::traceSimulation(void* thisobject, TraceManager::Level level, double time, Entity* entity, ModelComponent* component, const std::string& text) {
	traceSimulation(thisobject, time, entity, component, text, level);
}

void TraceManager::traceSimulation(void* thisobject, double time, Entity* entity, ModelComponent* component, const std::string& text, TraceManager::Level level, bool showAnyway) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
    if (_traceSimulationConditionPassed(level, thisobject, showAnyway)) {
		text = Util::Indent() + text;
		TraceSimulationEvent e = TraceSimulationEvent(level, time, entity, component, text);
		for (auto handler : *this->_traceSimulationHandlers->list()) {
			handler(e);
		}
		for (auto& handlerMethod : *this->_traceSimulationHandlersMethod->list()) {
			handlerMethod(e);
		}
	}
}

//void TraceManager::traceReport(TraceManager::Level level, std::string text) {
//	traceReport(text, level);
//}

void TraceManager::traceReport(const std::string& text, TraceManager::Level level) {
	// Ignore traces during teardown to prevent late callbacks on already-destroyed objects.
	if (_shuttingDown) {
		return;
	}
	if (_traceConditionPassed(level)) {
		text = Util::Indent() + text;
		TraceEvent e = TraceEvent(text, level);
		for (auto handler : *this->_traceReportHandlers->list()) {
			handler(e);
		}
		for (auto& handlerMethod : *this->_traceReportHandlersMethod->list()) {
			handlerMethod(e);
		}
	}
}

List<std::string>* TraceManager::errorMessages() const {
	return _errorMessages;
}

bool TraceManager::_traceConditionPassed(TraceManager::Level level) {
	return /*this->_debugged &&*/ static_cast<int> (this->_traceLevel) >= static_cast<int> (level);
}

bool TraceManager::_traceSimulationConditionPassed(TraceManager::Level level, void* thisobject, bool showAnyway) {
    if (showAnyway)
        return true;
    bool result = _traceConditionPassed(level);
    if (result) {
        bool isException = false;
        isException = (_traceSimulationExceptionRule->find(thisobject) != _traceSimulationExceptionRule->list()->end());
        result &= (_traceSimulationRuleAllAllowed && !isException) || (!_traceSimulationRuleAllAllowed && isException); // xor
    }
	return result;
}
