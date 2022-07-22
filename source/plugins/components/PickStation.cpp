/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickStation.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 13:08
 */

#include "PickStation.h"

#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PickStation::GetPluginInformation;
}
#endif

ModelDataDefinition* PickStation::NewInstance(Model* model, std::string name) {
	return new PickStation(model, name);
}

PickStation::PickStation(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<PickStation>(), name) {
}

std::string PickStation::show() {
	return ModelComponent::show() + "";
}

ModelComponent* PickStation::LoadInstance(Model* model, PersistenceRecord *fields) {
	PickStation* newComponent = new PickStation(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void PickStation::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool PickStation::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void PickStation::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool PickStation::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* PickStation::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PickStation>(), &PickStation::LoadInstance, &PickStation::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("station.so");
	std::string text = "The PickStation module allows an entity to select a station from the multiple stations specified.";
	text += " This module picks among the group of stations based on the selection logic defined with the module.";
	text += " The entity may then route, transport, convey, or connect to the station specified.";
	text += " If the method chosen is connect, the selected station is assigned to an entity attribute.";
	text += " The station selection process is based on the minimum or maximum value of a variety of system variables and expressions.";
	text += " TYPICAL USES: (1) A part sent to a processing station based on machine’s availability at each station;";
	text += " (2) A loan application sent to a set of loan officers based on the number sent to each officer;";
	text += " (3) A customer selecting among cashier lines based on the least number waiting in each line";
	info->setDescriptionHelp(text);
	return info;
}

