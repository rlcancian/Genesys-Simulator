/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Stop.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:15
 */

#include "plugins/components/MaterialHandling/Stop.h"

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC 

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Stop::GetPluginInformation;
}
#endif

ModelDataDefinition* Stop::NewInstance(Model* model, std::string name) {
	return new Stop(model, name);
}

Stop::Stop(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Stop>(), name) {
	auto* propConveyor = new SimulationControlGenericClass<Conveyor*, Model*, Conveyor>(
			_parentModel,
			std::bind(&Stop::getConveyor, this),
			std::bind(&Stop::setConveyor, this, std::placeholders::_1),
			Util::TypeOf<Stop>(), getName(), "Conveyor", "");
	_parentModel->getControls()->insert(propConveyor);
	_addSimulationControl(propConveyor);
}

std::string Stop::show() {
	return ModelComponent::show() + ", conveyor=\"" + (_conveyor != nullptr ? _conveyor->getName() : std::string()) + "\"";
}

void Stop::setConveyor(Conveyor* conveyor) {
	_conveyor = conveyor;
}

Conveyor* Stop::getConveyor() const {
	return _conveyor;
}

ModelComponent* Stop::LoadInstance(Model* model, PersistenceRecord *fields) {
	Stop* newComponent = new Stop(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Stop instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Stop::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_conveyor == nullptr) {
		traceError("Stop \"" + getName() + "\" has no Conveyor configured");
		return;
	}
	_conveyor->setActive(false);
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Stop::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_conveyor = dynamic_cast<Conveyor*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Conveyor>(), fields->loadField("conveyor", std::string(""))));
	}
	return res;
}

void Stop::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("conveyor", _conveyor != nullptr ? _conveyor->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Stop::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Conveyor>(), _conveyor, "Conveyor", errorMessage);
	if (this->getConnectionManager()->size() == 0) {
		errorMessage += "Stop \"" + getName() + "\" must forward to a downstream component. ";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Stop::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Stop>(), &Stop::LoadInstance, &Stop::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("conveyor.so");
	info->setDescriptionHelp("Deactivates a Conveyor and forwards the entity.");
	return info;
}



// void Stop::_createInternalStatisticReporters() { }

// void Stop::_createEditableDataDefinitions() { }

// void Stop::_createAttachedAttributes() { }
