/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Access.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:14
 */

#include "plugins/components/MaterialHandling/Access.h"

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Access::GetPluginInformation;
}
#endif

ModelDataDefinition* Access::NewInstance(Model* model, std::string name) {
	return new Access(model, name);
}

Access::Access(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Access>(), name) {
	auto* propConveyor = new SimulationControlGenericClass<Conveyor*, Model*, Conveyor>(
			_parentModel,
			std::bind(&Access::getConveyor, this),
			std::bind(&Access::setConveyor, this, std::placeholders::_1),
			Util::TypeOf<Access>(), getName(), "Conveyor", "");
	auto* propQuantityExpression = new SimulationControlGeneric<std::string>(
			std::bind(&Access::getQuantityExpression, this),
			std::bind(&Access::setQuantityExpression, this, std::placeholders::_1),
			Util::TypeOf<Access>(), getName(), "QuantityExpression", "");
	_parentModel->getControls()->insert(propConveyor);
	_parentModel->getControls()->insert(propQuantityExpression);
	_addSimulationControl(propConveyor);
	_addSimulationControl(propQuantityExpression);
}

std::string Access::show() {
	return ModelComponent::show() + ", conveyor=\"" + (_conveyor != nullptr ? _conveyor->getName() : std::string()) + "\", quantityExpression=\"" + _quantityExpression + "\"";
}

void Access::setConveyor(Conveyor* conveyor) {
	_conveyor = conveyor;
}

Conveyor* Access::getConveyor() const {
	return _conveyor;
}

void Access::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string Access::getQuantityExpression() const {
	return _quantityExpression;
}

ModelComponent* Access::LoadInstance(Model* model, PersistenceRecord *fields) {
	Access* newComponent = new Access(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Access instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Access::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	const unsigned int quantity = static_cast<unsigned int>(_parentModel->parseExpression(_quantityExpression));
	if (_conveyor == nullptr || quantity == 0 || !_conveyor->access(quantity)) {
		traceError("Access \"" + getName() + "\" could not allocate Conveyor capacity");
		return;
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Access::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_quantityExpression = fields->loadField("quantityExpression", std::string("1"));
		_conveyor = dynamic_cast<Conveyor*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Conveyor>(), fields->loadField("conveyor", std::string(""))));
	}
	return res;
}

//void Access::_initBetweenReplications() {}

void Access::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("quantityExpression", _quantityExpression, std::string("1"), saveDefaultValues);
	fields->saveField("conveyor", _conveyor != nullptr ? _conveyor->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Access::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Conveyor>(), _conveyor, "Conveyor", errorMessage);
	resultAll &= _parentModel->checkExpression(_quantityExpression, "QuantityExpression", errorMessage);
	if (this->getConnectionManager()->size() == 0) {
		errorMessage += "Access \"" + getName() + "\" must forward to a downstream component. ";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Access::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Access>(), &Access::LoadInstance, &Access::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("conveyor.so");
	info->setDescriptionHelp("Allocates simplified Conveyor capacity and forwards the entity only when the configured Conveyor is active and has enough free slots.");
	return info;
}



// void Access::_createInternalStatisticReporters() { }

// void Access::_createEditableDataDefinitions() {
// 	if (_conveyor == nullptr) {
// 		_conveyor = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Conveyor>(_parentModel);
// 	}
// }

// void Access::_createAttachedAttributes() { }
