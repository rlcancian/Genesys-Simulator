/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Assign.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 31 de Agosto de 2018, 10:10
 */

#include "Assign.h"
#include <string>
#include "Model.h"
#include "Variable.h"
#include "Attribute.h"
#include "Resource.h"

Assign::Assign(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Assign>(), name) {
}

std::string Assign::show() {
	std::string txt = ModelComponent::show() + ",assignments=[";
	for (std::list<Assignment*>::iterator it = _assignments->list()->begin(); it != _assignments->list()->end(); it++) {
		txt += (*it)->getDestination() + "=" + (*it)->getExpression() + ",";
	}
	txt = txt.substr(0, txt.length() - 1) + "]";
	return txt;
}

List<Assign::Assignment*>* Assign::getAssignments() const {
	return _assignments;
}

PluginInformation* Assign::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Assign>(), &Assign::LoadInstance);
	info->insertDynamicLibFileDependence("attribute.so");
	info->insertDynamicLibFileDependence("variable.so");
	return info;
}

ModelComponent* Assign::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Assign* newComponent = new Assign(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Assign::_execute(Entity* entity) {
	Assignment* let;
	std::list<Assignment*>* lets = this->_assignments->list();
	for (std::list<Assignment*>::iterator it = lets->begin(); it != lets->end(); it++) {
		let = (*it);
		double value = _parentModel->parseExpression(let->getExpression());
		_parentModel->parseExpression(let->getDestination() + "=" + std::to_string(value));
		_parentModel->getTracer()->trace("Let \"" + let->getDestination() + "\" = " + std::to_string(value) + "  // " + let->getExpression());
	}

	this->_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

void Assign::_initBetweenReplications() {
}

bool Assign::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		unsigned int nv = LoadField(fields, "assignments", DEFAULT.assignmentsSize);
		for (unsigned int i = 0; i < nv; i++) {
			std::string dest = LoadField(fields, "destination" + std::to_string(i), "");
			std::string exp = LoadField(fields, "expression" + std::to_string(i), "");
			Assignment* assmt = new Assignment(dest, exp);
			this->_assignments->insert(assmt);
		}
	}
	return res;
}

std::map<std::string, std::string>* Assign::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance(); //Util::TypeOf<Assign>());
	Assignment* let;
	SaveField(fields, "assignments", _assignments->size(), DEFAULT.assignmentsSize);
	unsigned short i = 0;
	for (std::list<Assignment*>::iterator it = _assignments->list()->begin(); it != _assignments->list()->end(); it++, i++) {
		let = (*it);
		SaveField(fields, "destination" + std::to_string(i), let->getDestination(), "");
		SaveField(fields, "expression" + std::to_string(i), let->getExpression(), "");
	}
	return fields;
}

bool Assign::_check(std::string* errorMessage) {
	bool resultAll = true;
	// \todo: Reimplement it. Since 201910, attributes may have index, just like "atrrib1[2]" or "att[10,1]". Because of that, the string may contain not only the name of the attribute, but also its index and therefore, fails on the test bellow.
	for (Assignment* let : *_assignments->list()) {
		resultAll &= _parentModel->checkExpression(let->getExpression(), "assignment", errorMessage);
	}
	return resultAll;
}