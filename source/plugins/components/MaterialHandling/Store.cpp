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
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Store::GetPluginInformation;
}
#endif

ModelDataDefinition* Store::NewInstance(Model* model, std::string name) {
	return new Store(model, name);
}

Store::Store(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Store>(), name) {
}

std::string Store::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Store::LoadInstance(Model* model, PersistenceRecord *fields) {
	Store* newComponent = new Store(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Store::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	traceSimulation(this, "I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Store::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		/*!
		 * \brief Load Store-specific persistent fields.
		 *
		 * Store currently has no extra persisted attributes beyond ModelComponent.
		 * Keep the template commands below as a guide for future extensions.
		 */
		// _storageName = fields->loadField("storageName", DEFAULT.storageName);
		// _quantityExpression = fields->loadField("quantityExpression", DEFAULT.quantityExpression);
	}
	return res;
}

void Store::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Save Store-specific persistent fields.
	 */
	// fields->saveField("storageName", _storageName, DEFAULT.storageName, saveDefaultValues);
	// fields->saveField("quantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaultValues);
}

bool Store::_check(std::string& errorMessage) {
	/*!
	 * \brief Validate minimal consistency for a pass-through Store component.
	 */
	bool resultAll = true;
	resultAll &= this->getConnectionManager()->size() >= 1;
	if (!resultAll) {
		errorMessage += "Store must have at least one output connection.";
	}
	return resultAll;
}

PluginInformation* Store::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Store>(), &Store::LoadInstance, &Store::NewInstance);
	info->setCategory("MaterialHandling");
	info->setDescriptionHelp("The Store component is a placeholder for storage-oriented flows. In the current implementation it forwards the entity to the next connected component.");
	// ...
	return info;
}


void Store::_createReportStatisticsDataDefinitions() {
}

void Store::_createEditableDataDefinitions() {
}

void Store::_createOthersDataDefinitions() {
}
