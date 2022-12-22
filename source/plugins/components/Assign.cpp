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
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/Simulator.h"
#include "../data/Variable.h"
#include "../data/Resource.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Assign::GetPluginInformation;
}
#endif

ModelDataDefinition* Assign::NewInstance(Model* model, std::string name) {
	return new Assign(model, name);
}

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

List<Assignment*>* Assign::getAssignments() const {
	return _assignments;
}

PluginInformation* Assign::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Assign>(), &Assign::LoadInstance, &Assign::NewInstance);
	//info->insertDynamicLibFileDependence("attribute.so");
	info->insertDynamicLibFileDependence("variable.so");
	info->setCategory("Discrete Processing");
	std::string text = "";
	text += "This module is used for assigning new values to variables, entity attributes, entity types, entity pictures, or other system variables.";
	text += " Multiple assignments can be made with a single Assign module.";
	text += " TYPICAL USES: (1) Accumulate the number of subassemblies added to a part;";
	text += " (2) Change an entity’s type to represent the customer copy of a multi - page form;";
	text += " (3) Establish a customer’s priority";
	info->setDescriptionHelp(text);

	return info;
}

ModelComponent* Assign::LoadInstance(Model* model, PersistenceRecord *fields) {
	Assign* newComponent = new Assign(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}

	return newComponent;
}

void Assign::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	Assignment* let;
	std::list<Assignment*>* lets = this->_assignments->list();
	for (std::list<Assignment*>::iterator it = lets->begin(); it != lets->end(); it++) {

		let = (*it);
		double value = _parentModel->parseExpression(let->getExpression());
		_parentModel->parseExpression(let->getDestination() + "=" + std::to_string(value));
		_parentModel->getTracer()->traceSimulation(this, "Let \"" + let->getDestination() + "\" = " + Util::StrTruncIfInt(std::to_string(value)) + "  // " + let->getExpression());
	}

	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

//void Assign::_initBetweenReplications() {}

bool Assign::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		unsigned int nv = fields->loadField("Assignments", DEFAULT.assignmentsSize);
		for (unsigned short i = 0; i < nv; i++) {
			Assignment* item = new Assignment("", "");
			item->loadInstance(fields, i);
			this->_assignments->insert(item);
		}
	}
	return res;
}

void Assign::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	Assignment* let;
	fields->saveField("Assignments", _assignments->size(), DEFAULT.assignmentsSize, saveDefaultValues);
	unsigned short i = 0;
	for (std::list<Assignment*>::iterator it = _assignments->list()->begin(); it != _assignments->list()->end(); it++, i++) {
		let = (*it);
		let->saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

bool Assign::_check(std::string* errorMessage) {
	bool resultAll = true;
	int i = 0;
	for (Assignment* let : *_assignments->list()) {
		ModelDataDefinition* data = nullptr;
		if (let->isAttributeNotVariable()) {
			data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), let->getDestination());
			if (data == nullptr) {
				data = _parentModel->getParentSimulator()->getPlugins()->newInstance<Attribute>(_parentModel, let->getDestination());
				_parentModel->getDataManager()->insert(data);
			}
		}
		if (!let->isAttributeNotVariable()) {
			data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), let->getDestination());
			if (data == nullptr) {
				data = _parentModel->getParentSimulator()->getPlugins()->newInstance<Variable>(_parentModel, let->getDestination());
				_parentModel->getDataManager()->insert(data);
			}
		}
		_attachedDataInsert("Assignment" + Util::StrIndex(i), data);
		// @TODO: +++ Reimplement it. Since 201910, attributes may have index, just like "atrrib1[2]" or "att[10,1]". Because of that, the string may contain not only the name of the attribute, but also its index and therefore, fails on the test bellow.
		resultAll &= _parentModel->checkExpression(let->getExpression(), "assignment", errorMessage);
		i++;
	}
	return resultAll;
}

void Assign::_createInternalAndAttachedData() {
	ModelDataManager* elems = _parentModel->getDataManager();
	for (Assignment* ass : *_assignments->list()) {
		ModelDataDefinition* elem;
		std::string name;
		if (ass->isAttributeNotVariable()) {
			name = "Attribute";
			elem = elems->getDataDefinition(Util::TypeOf<Attribute>(), ass->getDestination());
		} else {
			name = "Variable";
			elem = elems->getDataDefinition(Util::TypeOf<Variable>(), ass->getDestination());
		}
		//assert elem != nullptr
		if (elem != nullptr) {
			this->_attachedDataInsert(name + "_" + ass->getDestination(), elem);
		}
	}
}
