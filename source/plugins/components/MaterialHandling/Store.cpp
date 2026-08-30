/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Store.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:07
 */

#include "plugins/components/MaterialHandling/Store.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Store::GetPluginInformation;
}
#endif

ModelDataDefinition* Store::NewInstance(Model* model, std::string name) {
	return new Store(model, name);
}

Store::Store(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Store>(), name) {
	SimulationControlGenericClass<Storage*, Model*, Storage>* propStorage = new SimulationControlGenericClass<Storage*, Model*, Storage>(
			_parentModel,
			std::bind(&Store::getStorage, this), std::bind(&Store::setStorage, this, std::placeholders::_1),
			Util::TypeOf<Store>(), getName(), "Storage", "");
	SimulationControlGeneric<std::string>* propQuantity = new SimulationControlGeneric<std::string>(
			std::bind(&Store::getQuantityExpression, this), std::bind(&Store::setQuantityExpression, this, std::placeholders::_1),
			Util::TypeOf<Store>(), getName(), "QuantityExpression", "");
	_parentModel->getControls()->insert(propStorage);
	_parentModel->getControls()->insert(propQuantity);
	_addSimulationControl(propStorage);
	_addSimulationControl(propQuantity);
}

std::string Store::show() {
	std::string result = ModelComponent::show();
	if (_storage != nullptr) {
		result += ",storage=" + _storage->getName();
	}
	result += ",quantityExpression=" + _quantityExpression;
	return result;
}

void Store::setStorage(Storage* storage) {
	_storage = storage;
}

Storage* Store::getStorage() const {
	return _storage;
}

void Store::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string Store::getQuantityExpression() const {
	return _quantityExpression;
}

ModelComponent* Store::LoadInstance(Model* model, PersistenceRecord *fields) {
	Store* newComponent = new Store(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Store instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Store::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	const double parsedQuantity = _parentModel->parseExpression(_quantityExpression);
	const unsigned int quantity = parsedQuantity <= 0.0 ? 0u : static_cast<unsigned int>(parsedQuantity);
	if (quantity == 0u) {
		traceError("Store quantity evaluated to zero. Entity was not forwarded.", TraceManager::Level::L3_errorRecover);
		return;
	}
	if (!_storage->store(quantity)) {
		traceError("Storage \"" + _storage->getName() + "\" capacity exceeded while storing " + std::to_string(quantity) + " units.", TraceManager::Level::L3_errorRecover);
		return;
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Store::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_quantityExpression = fields->loadField("quantityExpression", DEFAULT.quantityExpression);
		std::string storageName = fields->loadField("storage", "");
		if (!storageName.empty()) {
			_storage = dynamic_cast<Storage*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Storage>(), storageName));
		}
	}
	return res;
}

void Store::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("quantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaultValues);
	if (_storage != nullptr) {
		fields->saveField("storage", _storage->getName(), "", saveDefaultValues);
	}
}

bool Store::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Storage>(), _storage, "Storage", errorMessage);
	resultAll &= _parentModel->checkExpression(_quantityExpression, "QuantityExpression", errorMessage);
	if (this->getConnectionManager()->size() < 1) {
		errorMessage += "Store must have at least one output connection.";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Store::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Store>(), &Store::LoadInstance, &Store::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("storage.so");
	info->setDescriptionHelp("Adds a parsed quantity to a Storage's current occupation and forwards the entity when capacity allows.");
	return info;
}


// void Store::_createInternalStatisticReporters() { }

void Store::_createEditableDataDefinitions() {
	if (_storage == nullptr) {
		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		_storage = plugins->newInstance<Storage>(_parentModel, getName() + ".Storage");
		if (_storage == nullptr) {
			_storage = new Storage(_parentModel, getName() + ".Storage");
		}
	}
	if (_storage != nullptr) {
		_optionalEditableDataDefinitionInsert("Storage", _storage);
	} else {
		_optionalEditableDataDefinitionRemove("Storage");
	}
}

// void Store::_createAttachedAttributes() { }
