/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Submodel.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:04
 */

#include "Submodel.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC 

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Submodel::GetPluginInformation;
}
#endif

ModelDataDefinition* Submodel::NewInstance(Model* model, std::string name) {
	return new Submodel(model, name);
}

Submodel::Submodel(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Submodel>(), name) {
}

std::string Submodel::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Submodel::LoadInstance(Model* model, PersistenceRecord *fields) {
	Submodel* newComponent = new Submodel(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Submodel::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Submodel::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void Submodel::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool Submodel::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* Submodel::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Submodel>(), &Submodel::LoadInstance, &Submodel::NewInstance);
	info->setCategory("Logic");
	// ...
	return info;
}


