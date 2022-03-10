/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Failure.cpp
 * Author: rlcancian
 * 
 * Created on 20 de Failureembro de 2019, 20:07
 */

#include "Failure.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Failure::GetPluginInformation;
}
#endif

ModelDataDefinition* Failure::NewInstance(Model* model, std::string name) {
	return new Failure(model, name);
}

Failure::Failure(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Failure>(), name) {
}

std::string Failure::show() {
	return ModelDataDefinition::show() +
			"";
}

bool Failure::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			//@TODO not implemented yet
			//this->attribute = LoadField(fields, "field", DEFAULT.fields);
			
		} catch (...) {
		}
	}
	return res;
}

std::map<std::string, std::string>* Failure::_saveInstance(bool saveDefaultValues) {
	std::map<std::string, std::string>* fields = ModelDataDefinition::_saveInstance(saveDefaultValues); //Util::TypeOf<Failure>());
	//@TODO not implemented yet
	//SaveField(fields, "orderRule", std::to_string(static_cast<int> (this->_orderRule)));
	//SaveField(fields, "attributeName", "\""+this->_attributeName+"\"");
	return fields;
}

bool Failure::_check(std::string* errorMessage) {
	bool resultAll = true;
	//@TODO not implemented yet
	// resultAll |= ...
	//*errorMessage += "";
	return resultAll;
}

ParserChangesInformation* Failure::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

PluginInformation* Failure::GetPluginInformation() {
	//@TODO not implemented yet
	PluginInformation* info = new PluginInformation(Util::TypeOf<Failure>(), &Failure::LoadInstance, &Failure::NewInstance);
	return info;
}

ModelDataDefinition* Failure::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Failure* newElement = new Failure(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}