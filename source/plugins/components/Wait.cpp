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
#include "../../kernel/simulator/SimulationControlAndResponse.h"
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

std::string Wait::convertEnumToStr(WaitType type) {
	switch (static_cast<int> (type)) {
		case 0: return "WaitForSignal";
		case 1: return "InfiniteHold";
		case 2: return "ScanForCondition";
	}
	return "Unknown";
}

Wait::Wait(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Wait>(), name) {
	SimulationControlGeneric<std::string>* propCondition = new SimulationControlGeneric<std::string>(
									std::bind(&Wait::getCondition, this), std::bind(&Wait::setCondition, this, std::placeholders::_1),
									Util::TypeOf<Wait>(), getName(), "Condition", "");
	SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Wait::getlimitExpression, this), std::bind(&Wait::setLimitExpression, this, std::placeholders::_1),
									Util::TypeOf<Wait>(), getName(), "LimitExpression", "");
    SimulationControlGenericEnum<Wait::WaitType, Wait>* propWaitType = new SimulationControlGenericEnum<Wait::WaitType, Wait>(
                                    std::bind(&Wait::getWaitType, this), std::bind(&Wait::setWaitType, this, std::placeholders::_1),
                                    Util::TypeOf<Wait>(), getName(), "WaitType", "");
	SimulationControlGenericClass<Queue*, Model*, Queue>* propQueue = new SimulationControlGenericClass<Queue*, Model*, Queue>(
									_parentModel,
									std::bind(&Wait::getQueue, this), std::bind(&Wait::setQueue, this, std::placeholders::_1),
									Util::TypeOf<Wait>(), getName(), "Queue", "");																			

	_parentModel->getControls()->insert(propQueue);
    _parentModel->getControls()->insert(propWaitType);
	_parentModel->getControls()->insert(propCondition);
	_parentModel->getControls()->insert(propExpression);

	// setting properties
	_addProperty(propQueue);
    _addProperty(propWaitType);
	_addProperty(propCondition);
	_addProperty(propExpression);
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

Queue* Wait::getQueue() const {
	return _queue;
}

void Wait::setQueue(Queue* queue) {
	_queue = queue;
}

std::string Wait::getlimitExpression() const {
	return limitExpression;
}

void Wait::setLimitExpression(const std::string &newLimitExpression){
	limitExpression = newLimitExpression;
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
		message += " until condition \"" + _condition + "\" is true";
	} else if (_waitType == Wait::WaitType::InfiniteHold) {
		message += " indefinitely";
	}
	traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, message);
	Waiting* waiting = new Waiting(entity, _parentModel->getSimulation()->getSimulatedTime(), this);
	_queue->insertElement(waiting);
}

bool Wait::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_waitType = static_cast<Wait::WaitType>(fields->loadField("waitType", static_cast<int>(DEFAULT.waitType)));
		_condition = fields->loadField("condition", DEFAULT.condition);
		limitExpression = fields->loadField("limitExpression", DEFAULT.limitExpression);
		std::string queueName = fields->loadField("queue", "");
		if (queueName != "") {
			_queue = dynamic_cast<Queue*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), queueName));
		}
		std::string signalName = fields->loadField("signalData", "");
		if (signalName != "") {
			_signalData = dynamic_cast<SignalData*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<SignalData>(), signalName));
		}
	}
	return res;
}

void Wait::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("waitType", static_cast<int>(_waitType), static_cast<int>(DEFAULT.waitType), saveDefaultValues);
	fields->saveField("condition", _condition, DEFAULT.condition, saveDefaultValues);
	fields->saveField("limitExpression", limitExpression, DEFAULT.limitExpression, saveDefaultValues);
	if (_queue != nullptr) {
		fields->saveField("queue", _queue->getName(), "", saveDefaultValues);
	}
	if (_waitType == WaitType::WaitForSignal && _signalData != nullptr) {
		fields->saveField("signalData", _signalData->getName(), "", saveDefaultValues);
	}
}

// protected virtual could override

bool Wait::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_queue == nullptr) {
		errorMessage += "Queue is null in Wait \"" + this->getName() + "\"";
		return false;
	}
	if (_waitType == Wait::WaitType::WaitForSignal) {
		// Wait/Signal operational contract is based on a shared SignalData instance.
		if (_signalData == nullptr) {
			errorMessage += "SignalData is null for WaitForSignal in Wait \"" + this->getName() + "\"";
			resultAll = false;
		}
		if (!_parentModel->checkExpression(limitExpression, "LimitExpression", errorMessage)) {
			resultAll = false;
		}
	} else if (_waitType == Wait::WaitType::ScanForCondition) {
		resultAll = _parentModel->checkExpression(_condition, "Condition", errorMessage);
		if (resultAll && !_isScanConditionHandlerRegistered) { // local guard to avoid duplicate registration on re-checks
			_parentModel->getOnEventManager()->addOnAfterProcessEventHandler(this, &Wait::_handlerForAfterProcessEventEvent);
			_isScanConditionHandlerRegistered = true;
		}
	}
	return resultAll;
}

void Wait::_createInternalAndAttachedData() {
	SignalData* previouslyAttachedSignalData = nullptr;
	std::map<std::string, ModelDataDefinition*>* attachedData = getAttachedData();
	std::map<std::string, ModelDataDefinition*>::iterator attachedSignalDataIt = attachedData->find("SignalData");
	if (attachedSignalDataIt != attachedData->end()) {
		previouslyAttachedSignalData = dynamic_cast<SignalData*>(attachedSignalDataIt->second);
	}

	// internal
	PluginManager* pm = _parentModel->getParentSimulator()->getPluginManager();
	if (_queue == nullptr) {
		_queue = pm->newInstance<Queue>(_parentModel, getName() + ".Queue");
	}
	_internalDataInsert("Queue", _queue);
	//attached
	if (previouslyAttachedSignalData != nullptr && (_waitType != Wait::WaitType::WaitForSignal || previouslyAttachedSignalData != _signalData)) {
		previouslyAttachedSignalData->removeSignalDataEventHandler(this);
	}

	if (_waitType == Wait::WaitType::WaitForSignal) {
		if (_signalData == nullptr) {
			_signalData = pm->newInstance<SignalData>(_parentModel);
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
	unsigned int waitLimit = _parentModel->parseExpression(limitExpression);
	// Stop when either global signal limit or local wait limit is reached.
	while (_queue->size() > 0 && signalData->remainsToLimit() > 0 && freed < waitLimit) {
		Waiting* w = _queue->getAtRank(0);
		Entity* ent = w->getEntity();
		ModelComponent* sourceComponent = w->geComponent();
		_queue->removeElement(w);
		freed++;
		signalData->decreaseRemainLimit();
		std::string message = getName() + " received " + signalData->getName() + ". " + ent->getName() + " removed from " + _queue->getName() + ". " + std::to_string(freed) + " freed, " + std::to_string(signalData->remainsToLimit()) + " remaining";
        traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), ent, this, message);
		_parentModel->sendEntityToComponent(ent, sourceComponent->getConnectionManager()->getFrontConnection());
	}
	return freed;
}

void Wait::_handlerForAfterProcessEventEvent(SimulationEvent* event) {
	if (_waitType != Wait::WaitType::ScanForCondition) {
		return;
	}
	double result = _parentModel->parseExpression(_condition);
	//std::string message = "Condition \"" + _condition + "\" evaluates to " + std::to_string(result);
	//traceSimulation(this, TraceManager::Level::L7_internal, _parentModel->getSimulation()->getSimulatedTime(), event->getCurrentEvent()->getEntity(), this, message);
	if (result) { // condition is true. Remove entities from the queue
		while (_queue->size() > 0) {
			Waiting* w = _queue->getAtRank(0);
			Entity* ent = w->getEntity();
			ModelComponent* sourceComponent = w->geComponent();
			_queue->removeElement(w);
			std::string message = getName() + " evaluated condition " + _condition + " as true. " + ent->getName() + " removed from " + _queue->getName();
            traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), ent, this, message, TraceManager::Level::L8_detailed);
			_parentModel->sendEntityToComponent(ent, sourceComponent->getConnectionManager()->getFrontConnection());
		}

	}
}
