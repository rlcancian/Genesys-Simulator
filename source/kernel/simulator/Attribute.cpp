/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Attribute.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 25 de Setembro de 2018, 16:37
 */

#include "Attribute.h"

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC 

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Attribute::GetPluginInformation;
}
#endif

ModelDataDefinition* Attribute::NewInstance(Model* model, std::string name) {
	return new Attribute(model, name);
}

Attribute::Attribute(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Attribute>(), name) {
}

std::string Attribute::show() {
	return ModelDataDefinition::show();
}

bool Attribute::_loadInstance(PersistenceRecord *fields) {
	return ModelDataDefinition::_loadInstance(fields);
}

PluginInformation* Attribute::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Attribute>(), &Attribute::LoadInstance, &Attribute::NewInstance);
	info->setDescriptionHelp("//@TODO");
	return info;

}

ModelDataDefinition* Attribute::LoadInstance(Model* model, PersistenceRecord *fields) {
	Attribute* newElement = new Attribute(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

void Attribute::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	bool saveDefaults = this->_getSaveDefaultsOption();
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
}

bool Attribute::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}