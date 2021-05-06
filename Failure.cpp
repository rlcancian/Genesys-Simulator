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

Failure::Failure(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Failure>(), name) {
}

std::string Failure::show() {
	return ModelElement::show() +
			"";
}

PluginInformation* Failure::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Failure>(), &Failure::LoadInstance);
	return info;
}

ModelElement* Failure::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Failure* newElement = new Failure(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Failure::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelElement::_loadInstance(fields);
	if (res) {
		try {
			//this->_attributeName = (*fields->find("attributeName")).second;
			//this->_orderRule = static_cast<OrderRule> (std::stoi((*fields->find("orderRule")).second));
		} catch (...) {
		}
	}
	return res;
}

std::map<std::string, std::string>* Failure::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance(); //Util::TypeOf<Failure>());
	//SaveField(fields, "orderRule", std::to_string(static_cast<int> (this->_orderRule)));
	//SaveField(fields, "attributeName", "\""+this->_attributeName+"\"");
	return fields;
}

bool Failure::_check(std::string* errorMessage) {
	bool resultAll = true;
	// resultAll |= ...
	return resultAll;
}

ParserChangesInformation* Failure::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}
