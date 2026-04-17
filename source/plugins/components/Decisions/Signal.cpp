/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Signal.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#include "plugins/components/Decisions/Signal.h"

#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Signal::GetPluginInformation;
}
#endif

// constructor

ModelDataDefinition* Signal::NewInstance(Model* model, std::string name) {
	return new Signal(model, name);
}

Signal::Signal(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Signal>(), name) {
	SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Signal::limitExpression, this), std::bind(&Signal::setLimitExpression, this, std::placeholders::_1),
									Util::TypeOf<Signal>(), getName(), "LimitExpression", "");

	_parentModel->getControls()->insert(propExpression);

	// setting properties
	_addProperty(propExpression);
}

// public virtual

std::string Signal::show() {
	return ModelComponent::show() + "";
}

//public

void Signal::setSignalData(SignalData* signal) {
	_signalData = signal;
}

// public static

ModelComponent* Signal::LoadInstance(Model* model, PersistenceRecord *fields) {
	Signal* newComponent = new Signal(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* Signal::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Signal>(), &Signal::LoadInstance, &Signal::NewInstance);
	info->setCategory("Decisions");
	// ...
	return info;
}

// protected must override

void Signal::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	unsigned int limit = _parentModel->parseExpression(_limitExpression);
	traceSimulation(this, "Triggering signal \""+_signalData->getName()+"\" with limit \""+_limitExpression+"\"="+std::to_string(limit));
	unsigned int freed = _signalData->generateSignal(_signalData->getId(),limit);
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Signal::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_limitExpression = fields->loadField("limitExpression", DEFAULT.limitExpression);
		std::string signalDataName = fields->loadField("signalData", "");
		if (signalDataName != "") {
			// Persisted coupling is restored by name to keep Wait/Signal sharing the same SignalData.
			_signalData = dynamic_cast<SignalData*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<SignalData>(), signalDataName));
		}
	}
	return res;
}

void Signal::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("limitExpression", _limitExpression, DEFAULT.limitExpression);
	if (_signalData != nullptr) {
		// Save associated SignalData name so load can reconnect shared state with Wait components.
		fields->saveField("signalData", _signalData->getName(), "", saveDefaultValues);
	}
}

// protected should override

bool Signal::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (!_parentModel->checkExpression(_limitExpression, "LimitExpression", errorMessage)) {
		resultAll = false;
	}
	// Current operational contract requires Signal to reference shared SignalData.
	if (_signalData == nullptr) {
		errorMessage += "SignalData is null in Signal \"" + this->getName() + "\"";
		resultAll = false;
	} else {
		SignalData* signalDataByName = dynamic_cast<SignalData*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<SignalData>(), _signalData->getName()));
		if (signalDataByName == nullptr) {
			errorMessage += "SignalData \"" + _signalData->getName() + "\" was not found in model data manager for Signal \"" + this->getName() + "\"";
			resultAll = false;
		} else if (signalDataByName != _signalData) {
			errorMessage += "SignalData \"" + _signalData->getName() + "\" reference mismatch in Signal \"" + this->getName() + "\"";
			resultAll = false;
		}
	}
	return resultAll;
}

void Signal::_createInternalAndAttachedData() {
	PluginManager* pm = _parentModel->getParentSimulator()->getPluginManager();
	// Preserve loaded/configured association; only create SignalData if none is already associated.
	if (_signalData == nullptr) {
		_signalData = pm->newInstance<SignalData>(_parentModel);
	}
	if (_signalData != nullptr) {
		_attachedDataInsert("SignalData", _signalData);
	} else {
		_attachedDataRemove("SignalData");
	}
}

const std::string&Signal::limitExpression() const
{
	return _limitExpression;
}

void Signal::setLimitExpression(const std::string&newLimitExpression)
{
	_limitExpression = newLimitExpression;
}

void Signal::_initBetweenReplications() {

}
