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

Attribute::Attribute(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Attribute>(), name) {
}

std::string Attribute::show() {
	return ModelElement::show();
}

bool Attribute::_loadInstance(std::map<std::string, std::string>* fields) {
	return ModelElement::_loadInstance(fields);
}

PluginInformation* Attribute::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Attribute>(), &Attribute::LoadInstance);
	return info;

}

ModelElement* Attribute::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Attribute* newElement = new Attribute(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

std::map<std::string, std::string>* Attribute::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance(); //Util::TypeOf<Attribute>());
	return fields;
}

bool Attribute::_check(std::string* errorMessage) {
	return true;
}