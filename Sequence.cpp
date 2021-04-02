/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sequence.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:12
 */

#include "Sequence.h"
#include "Attribute.h"
#include "Model.h"

Sequence::Sequence(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Sequence>(), name) {
}

std::string Sequence::show() {
	std::string msg = ModelElement::show();
	return msg;
}

PluginInformation* Sequence::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Sequence>(), &Sequence::LoadInstance);
	return info;
}

ModelElement* Sequence::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Sequence* newElement = new Sequence(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

List<SequenceStep*>* Sequence::getSteps() const {
	return _steps;
}

bool Sequence::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelElement::_loadInstance(fields);
	if (res) {
		try {
		} catch (...) {
		}
	}
	return res;
}

std::map<std::string, std::string>* Sequence::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance(); //Util::TypeOf<Sequence>());
	return fields;
}

bool Sequence::_check(std::string* errorMessage) {
	/* include attributes needed */
	std::vector<std::string> neededNames = {"Entity.Sequence", "Entity.SequenceStep"};
	std::string neededName;
	for (unsigned int i = 0; i < neededNames.size(); i++) {
		neededName = neededNames[i];
		if (_parentModel->getElements()->getElement(Util::TypeOf<Attribute>(), neededName) == nullptr) {
			Attribute* attr1 = new Attribute(_parentModel, neededName);
			//_parentModel->insert(attr1);
		}
	}
	//
	return true;
}

