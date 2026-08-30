/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Unstore.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:08
 */

#include "plugins/components/MaterialHandling/Unstore.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Unstore::GetPluginInformation;
}
#endif

ModelDataDefinition* Unstore::NewInstance(Model* model, std::string name) {
	return new Unstore(model, name);
}

Unstore::Unstore(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Unstore>(), name) {
	SimulationControlGenericClass<Storage*, Model*, Storage>* propStorage = new SimulationControlGenericClass<Storage*, Model*, Storage>(
			_parentModel,
			std::bind(&Unstore::getStorage, this), std::bind(&Unstore::setStorage, this, std::placeholders::_1),
			Util::TypeOf<Unstore>(), getName(), "Storage", "");
	SimulationControlGeneric<std::string>* propQuantity = new SimulationControlGeneric<std::string>(
			std::bind(&Unstore::getQuantityExpression, this), std::bind(&Unstore::setQuantityExpression, this, std::placeholders::_1),
			Util::TypeOf<Unstore>(), getName(), "QuantityExpression", "");
	_parentModel->getControls()->insert(propStorage);
	_parentModel->getControls()->insert(propQuantity);
	_addSimulationControl(propStorage);
	_addSimulationControl(propQuantity);
}

std::string Unstore::show() {
	std::string result = ModelComponent::show();
	if (_storage != nullptr) {
		result += ",storage=" + _storage->getName();
	}
	result += ",quantityExpression=" + _quantityExpression;
	return result;
}

void Unstore::setStorage(Storage* storage) {
	_storage = storage;
}

Storage* Unstore::getStorage() const {
	return _storage;
}

void Unstore::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string Unstore::getQuantityExpression() const {
	return _quantityExpression;
}

ModelComponent* Unstore::LoadInstance(Model* model, PersistenceRecord *fields) {
	Unstore* newComponent = new Unstore(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Unstore instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Unstore::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	const double parsedQuantity = _parentModel->parseExpression(_quantityExpression);
	const unsigned int quantity = parsedQuantity <= 0.0 ? 0u : static_cast<unsigned int>(parsedQuantity);
	if (quantity == 0u) {
		traceError("Unstore quantity evaluated to zero. Entity was not forwarded.", TraceManager::Level::L3_errorRecover);
		return;
	}
	if (!_storage->unstore(quantity)) {
		traceError("Storage \"" + _storage->getName() + "\" does not contain " + std::to_string(quantity) + " units to unstore.", TraceManager::Level::L3_errorRecover);
		return;
	}
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Unstore::_loadInstance(PersistenceRecord *fields) {
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

void Unstore::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("quantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaultValues);
	if (_storage != nullptr) {
		fields->saveField("storage", _storage->getName(), "", saveDefaultValues);
	}
}

bool Unstore::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Storage>(), _storage, "Storage", errorMessage);
	resultAll &= _parentModel->checkExpression(_quantityExpression, "QuantityExpression", errorMessage);
	if (this->getConnectionManager()->size() < 1) {
		errorMessage += "Unstore must have at least one output connection.";
		resultAll = false;
	}
	return resultAll;
}

PluginInformation* Unstore::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Unstore>(), &Unstore::LoadInstance, &Unstore::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("storage.so");
	info->setDescriptionHelp("Subtracts a parsed quantity from a Storage's current occupation and forwards the entity when enough units are available.");
	return info;
}



// void Unstore::_createInternalStatisticReporters() { }

void Unstore::_createEditableDataDefinitions() {
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

// void Unstore::_createAttachedAttributes() { }
