/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Exit.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:15
 */

#include "plugins/components/MaterialHandling/Exit.h"

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Exit::GetPluginInformation;
}
#endif

ModelDataDefinition* Exit::NewInstance(Model* model, std::string name) {
	return new Exit(model, name);
}

Exit::Exit(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Exit>(), name) {
	auto* propConveyor = new SimulationControlGenericClass<Conveyor*, Model*, Conveyor>(
			_parentModel,
			std::bind(&Exit::getConveyor, this),
			std::bind(&Exit::setConveyor, this, std::placeholders::_1),
			Util::TypeOf<Exit>(), getName(), "Conveyor", "");
	auto* propQuantityExpression = new SimulationControlGeneric<std::string>(
			std::bind(&Exit::getQuantityExpression, this),
			std::bind(&Exit::setQuantityExpression, this, std::placeholders::_1),
			Util::TypeOf<Exit>(), getName(), "QuantityExpression", "");
	_parentModel->getControls()->insert(propConveyor);
	_parentModel->getControls()->insert(propQuantityExpression);
	_addSimulationControl(propConveyor);
	_addSimulationControl(propQuantityExpression);
}

std::string Exit::show() {
	return ModelComponent::show() + ", conveyor=\"" + (_conveyor != nullptr ? _conveyor->getName() : std::string()) + "\", quantityExpression=\"" + _quantityExpression + "\"";
}

void Exit::setConveyor(Conveyor* conveyor) {
	_conveyor = conveyor;
}

Conveyor* Exit::getConveyor() const {
	return _conveyor;
}

void Exit::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string Exit::getQuantityExpression() const {
	return _quantityExpression;
}

ModelComponent* Exit::LoadInstance(Model* model, PersistenceRecord *fields) {
	Exit* newComponent = new Exit(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Exit instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Exit::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	const unsigned int quantity = static_cast<unsigned int>(_parentModel->parseExpression(_quantityExpression));
	if (_conveyor == nullptr || quantity == 0 || !_conveyor->exit(quantity)) {
		traceError("Exit \"" + getName() + "\" could not release Conveyor capacity");
		return;
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Exit::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_quantityExpression = fields->loadField("quantityExpression", std::string("1"));
		_conveyor = dynamic_cast<Conveyor*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Conveyor>(), fields->loadField("conveyor", std::string(""))));
	}
	return res;
}

void Exit::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("quantityExpression", _quantityExpression, std::string("1"), saveDefaultValues);
	fields->saveField("conveyor", _conveyor != nullptr ? _conveyor->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Exit::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Conveyor>(), _conveyor, "Conveyor", errorMessage);
	resultAll &= _parentModel->checkExpression(_quantityExpression, "QuantityExpression", errorMessage);
	if (this->getConnectionManager()->size() == 0) {
		errorMessage += "Exit \"" + getName() + "\" must forward to a downstream component. ";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Exit::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Exit>(), &Exit::LoadInstance, &Exit::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("conveyor.so");
	info->setDescriptionHelp("Releases simplified Conveyor capacity and forwards the entity when the configured quantity was previously allocated.");
	return info;
}



// void Exit::_createInternalStatisticReporters() { }

// void Exit::_createEditableDataDefinitions() { }

// void Exit::_createAttachedAttributes() { }
