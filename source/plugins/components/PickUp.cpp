/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickUp.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:15
 */

#include "PickUp.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PickUp::GetPluginInformation;
}
#endif

ModelDataDefinition* PickUp::NewInstance(Model* model, std::string name) {
	return new PickUp(model, name);
}

PickUp::PickUp(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<PickUp>(), name) {
}

std::string PickUp::show() {
	return ModelComponent::show() + "";
}

ModelComponent* PickUp::LoadInstance(Model* model, PersistenceRecord *fields) {
	PickUp* newComponent = new PickUp(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void PickUp::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool PickUp::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void PickUp::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool PickUp::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* PickUp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PickUp>(), &PickUp::LoadInstance, &PickUp::NewInstance);
	info->setCategory("Decisions");
	std::string text = "The Pickup module removes a number of consecutive entities from a given queue starting at a specified rank in the queue.";
	text += "The entities that are picked up are added to the end of the incoming entity’s group.";
	info->setDescriptionHelp(text);
	return info;
}


