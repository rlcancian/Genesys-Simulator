/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DropOff.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:15
 */

#include "DropOff.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DropOff::GetPluginInformation;
}
#endif

ModelDataDefinition* DropOff::NewInstance(Model* model, std::string name) {
	return new DropOff(model, name);
}

DropOff::DropOff(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DropOff>(), name) {
}

std::string DropOff::show() {
	return ModelComponent::show() + "";
}

ModelComponent* DropOff::LoadInstance(Model* model, PersistenceRecord *fields) {
	DropOff* newComponent = new DropOff(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void DropOff::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool DropOff::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

//void DropOff::_initBetweenReplications() {}

void DropOff::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool DropOff::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* DropOff::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DropOff>(), &DropOff::LoadInstance, &DropOff::NewInstance);
	info->setCategory("Decisions");
	info->setDescriptionHelp("//@TODO");
	return info;
}

