/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Start.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:15
 */

#include "plugins/components/MaterialHandling/Start.h"

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Start::GetPluginInformation;
}
#endif

ModelDataDefinition* Start::NewInstance(Model* model, std::string name) {
	return new Start(model, name);
}

Start::Start(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Start>(), name) {
	auto* propConveyor = new SimulationControlGenericClass<Conveyor*, Model*, Conveyor>(
			_parentModel,
			std::bind(&Start::getConveyor, this),
			std::bind(&Start::setConveyor, this, std::placeholders::_1),
			Util::TypeOf<Start>(), getName(), "Conveyor", "");
	auto* propVelocityExpression = new SimulationControlGeneric<std::string>(
			std::bind(&Start::getVelocityExpression, this),
			std::bind(&Start::setVelocityExpression, this, std::placeholders::_1),
			Util::TypeOf<Start>(), getName(), "VelocityExpression", "");
	_parentModel->getControls()->insert(propConveyor);
	_parentModel->getControls()->insert(propVelocityExpression);
	_addSimulationControl(propConveyor);
	_addSimulationControl(propVelocityExpression);
}

std::string Start::show() {
	return ModelComponent::show() + ", conveyor=\"" + (_conveyor != nullptr ? _conveyor->getName() : std::string()) + "\", velocityExpression=\"" + _velocityExpression + "\"";
}

void Start::setConveyor(Conveyor* conveyor) {
	_conveyor = conveyor;
}

Conveyor* Start::getConveyor() const {
	return _conveyor;
}

void Start::setVelocityExpression(std::string velocityExpression) {
	_velocityExpression = velocityExpression;
}

std::string Start::getVelocityExpression() const {
	return _velocityExpression;
}

ModelComponent* Start::LoadInstance(Model* model, PersistenceRecord *fields) {
	Start* newComponent = new Start(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Start instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Start::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_conveyor == nullptr) {
		traceError("Start \"" + getName() + "\" has no Conveyor configured");
		return;
	}
	if (!_velocityExpression.empty()) {
		const double velocity = _parentModel->parseExpression(_velocityExpression);
		if (velocity <= 0.0) {
			traceError("Start \"" + getName() + "\" evaluated a non-positive velocity");
			return;
		}
		_conveyor->setVelocity(velocity);
	}
	_conveyor->setActive(true);
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Start::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_velocityExpression = fields->loadField("velocityExpression", std::string(""));
		_conveyor = dynamic_cast<Conveyor*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Conveyor>(), fields->loadField("conveyor", std::string(""))));
	}
	return res;
}

void Start::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("velocityExpression", _velocityExpression, std::string(""), saveDefaultValues);
	fields->saveField("conveyor", _conveyor != nullptr ? _conveyor->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Start::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Conveyor>(), _conveyor, "Conveyor", errorMessage);
	if (!_velocityExpression.empty()) {
		resultAll &= _parentModel->checkExpression(_velocityExpression, "VelocityExpression", errorMessage);
	}
	if (this->getConnectionManager()->size() == 0) {
		errorMessage += "Start \"" + getName() + "\" must forward to a downstream component. ";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Start::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Start>(), &Start::LoadInstance, &Start::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("conveyor.so");
	info->setDescriptionHelp("Activates a Conveyor and optionally updates its velocity before forwarding the entity.");
	return info;
}



// void Start::_createInternalStatisticReporters() { }

// void Start::_createEditableDataDefinitions() { }

// void Start::_createAttachedAttributes() { }
