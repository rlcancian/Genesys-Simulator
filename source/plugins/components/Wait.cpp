/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Wait.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#include "Wait.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"
#include "../../plugins/data/Queue.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Wait::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition* Wait::NewInstance(Model* model, std::string name) {
	return new Wait(model, name);
}

Wait::Wait(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Wait>(), name) {
}

// public

std::string Wait::show() {
	return ModelComponent::show() + "";
}

void Wait::setSignalData(SignalData* signal) {
	_signalData = signal;
}

void Wait::setCondition(std::string _condition) {
	this->_condition = _condition;
}

std::string Wait::getCondition() const {
	return _condition;
}

void Wait::setWaitType(WaitType _watType) {
	this->_waitType = _watType;
}

Wait::WaitType Wait::getWaitType() const {
	return _waitType;
}

//public static

ModelComponent* Wait::LoadInstance(Model* model, PersistenceRecord *fields) {
	Wait* newComponent = new Wait(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* Wait::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Wait>(), &Wait::LoadInstance, &Wait::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("signal.so");
	return info;
}

// protected virtual must override

void Wait::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	std::string message = "Entity is waiting in the queue \"" + _queue->getName() + "\"";
	if (_waitType == Wait::WaitType::WaitForSignal) {
		message += " for signal \"" + _signalData->getName() + "\"";
	} else if (_waitType == Wait::WaitType::ScanForCondition) {
		message += " until codition \"" + _condition + "\" is true";
	} else if (_waitType == Wait::WaitType::ScanForCondition) {
		message += " indefinitely";
	}
	_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, message);
	Waiting* waiting = new Waiting(entity, _parentModel->getSimulation()->getSimulatedTime(), this);
	_queue->insertElement(waiting);
}

bool Wait::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void Wait::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

// protected virtual could override

bool Wait::_check(std::string * errorMessage) {
	bool resultAll = true;
	if (_waitType == Wait::WaitType::ScanForCondition) {
		resultAll = _parentModel->checkExpression(_condition, "Condition", errorMessage);
		if (resultAll) { // add handler to event AfterProcessEvent 
			_parentModel->getOnEvents()->addOnAfterProcessEventHandler(this, &Wait::_handlerForAfterProcessEventEvent);
		}
	}
	return resultAll;
}

void Wait::_createInternalAndAttachedData() {
	// internal
	PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
	if (_queue == nullptr) {
		_queue = pm->newInstance<Queue>(_parentModel, getName() + ".Queue");
		_internalDataInsert("Queue", _queue);
	}
	//attached
	if (_waitType == Wait::WaitType::WaitForSignal) {
		if (_signalData == nullptr) {
			if (_parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				_signalData = pm->newInstance<SignalData>(_parentModel);
			}
		}
		SignalData::SignalDataEventHandler handler = SignalData::SetSignalDataEventHandler<Wait>(&Wait::_handlerForSignalDataEvent, this);
		_signalData->addSignalDataEventHandler(handler, this);
		_attachedDataInsert("SignalData", _signalData);
	} else {
		_attachedDataRemove("SignalData");
	}
}

void Wait::_initBetweenReplications() {

}

// private

unsigned int Wait::_handlerForSignalDataEvent(SignalData* signalData) {
	unsigned int freed = 0;
	while (_queue->size() > 0 && signalData->remainsToLimit() > 0) {
		Waiting* w = _queue->getAtRank(0);
		_queue->removeElement(w);
		freed++;
		signalData->decreaseRemainLimit();
		Entity* ent = w->getEntity();
		std::string message = getName() + " received " + signalData->getName() + ". " + ent->getName() + " removed from " + _queue->getName() + ". " + std::to_string(freed) + " freed, " + std::to_string(signalData->remainsToLimit()) + " remaining";
		_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, _parentModel->getSimulation()->getSimulatedTime(), ent, this, message);
		_parentModel->sendEntityToComponent(w->getEntity(), w->geComponent()->getConnections()->getFrontConnection());
	}
	return freed;
}

void Wait::_handlerForAfterProcessEventEvent(SimulationEvent* event) {
	double result = _parentModel->parseExpression(_condition);
	//std::string message = "Condition \"" + _condition + "\" evaluates to " + std::to_string(result);
	//_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, _parentModel->getSimulation()->getSimulatedTime(), event->getCurrentEvent()->getEntity(), this, message);
	if (result) { // condition is true. Remove entities from the queue
		while (_queue->size() > 0) {
			Waiting* w = _queue->getAtRank(0);
			_queue->removeElement(w);
			Entity* ent = w->getEntity();
			std::string message = getName() + " evaluated condition " + _condition + " as true. " + ent->getName() + " removed from " + _queue->getName();
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, _parentModel->getSimulation()->getSimulatedTime(), ent, this, message);
			_parentModel->sendEntityToComponent(w->getEntity(), w->geComponent()->getConnections()->getFrontConnection());
		}

	}
}