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

#include "Exit.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Exit::GetPluginInformation;
}
#endif

ModelDataDefinition* Exit::NewInstance(Model* model, std::string name) {
	return new Exit(model, name);
}

Exit::Exit(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Exit>(), name) {
}

std::string Exit::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Exit::LoadInstance(Model* model, PersistenceRecord *fields) {
	Exit* newComponent = new Exit(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Exit::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Exit::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void Exit::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool Exit::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* Exit::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Exit>(), &Exit::LoadInstance, &Exit::NewInstance);
	info->setCategory("Material Handling");
	return info;
}


