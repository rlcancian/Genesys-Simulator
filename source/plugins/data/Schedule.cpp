/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Schedule.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:16
 */

#include "Schedule.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Schedule::GetPluginInformation;
}
#endif

ModelDataDefinition* Schedule::NewInstance(Model* model, std::string name) {
	return new Schedule(model, name);
}

Schedule::Schedule(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Schedule>(), name) {
}

std::string Schedule::show() {
	return ModelDataDefinition::show();
}

PluginInformation* Schedule::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Schedule>(), &Schedule::LoadInstance, &Schedule::NewInstance);
	info->setDescriptionHelp("//@TODO");
	// ...
	return info;
}

ModelDataDefinition* Schedule::LoadInstance(Model* model, PersistenceRecord *fields) {
	Schedule* newElement = new Schedule(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Schedule::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			// @TODO: Not implemented
			//this->_attributeName = fields->loadField("attributeName", DEFAULT.attributeName);
			//this->_orderRule = static_cast<OrderRule> (fields->loadField("orderRule", static_cast<int> (DEFAULT.orderRule)));
		} catch (...) {
		}
	}
	return res;
}

void Schedule::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	// @TODO: Not implemented
}

bool Schedule::_check(std::string* errorMessage) {
	// @TODO: Not implemented
	*errorMessage += "";
	return true;
}
