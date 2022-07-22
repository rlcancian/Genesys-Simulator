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

#include "Start.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Start::GetPluginInformation;
}
#endif

ModelDataDefinition* Start::NewInstance(Model* model, std::string name) {
	return new Start(model, name);
}

Start::Start(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Start>(), name) {
}

std::string Start::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Start::LoadInstance(Model* model, PersistenceRecord *fields) {
	Start* newComponent = new Start(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Start::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Start::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void Start::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool Start::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* Start::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Start>(), &Start::LoadInstance, &Start::NewInstance);
	info->setCategory("Material Handling");
	// ...
	return info;
}


