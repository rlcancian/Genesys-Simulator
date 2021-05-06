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

SequenceStep::SequenceStep(Station* station, std::list<Assignment*>* assignments) {
	this->_station = station;
	if (assignments != nullptr)
		_assignments = assignments;
	else
		_assignments = new std::list<Assignment*>();
}

bool SequenceStep::_loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex) {
	bool res = true;
	std::string num = std::to_string(parentIndex);
	std::string destination, expression;
	try {
		std::string stationName = LoadField(fields, "station" + num, "");
		// \todo _station =
		unsigned int assignmentsSize = LoadField(fields, "assignmentSize" + num, DEFAULT.assignmentsSize);
		for (unsigned short i = 0; i < assignmentsSize; i++) {
			destination = LoadField(fields, "destination" + num + "_" + std::to_string(i));
			expression = LoadField(fields, "expression" + num + "_" + std::to_string(i));
			_assignments->insert(_assignments->end(), new Assignment(destination, expression));
		}
	} catch (...) {
		res = false;
	}
	return res;
}

std::map<std::string, std::string>* SequenceStep::_saveInstance(unsigned int parentIndex) {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	std::string num = std::to_string(parentIndex);
	SaveField(fields, "station" + num, _station->getName());
	//SaveField(fields, "resourceId" + num, _resourceOrSet->getId());
	SaveField(fields, "assignmentSize" + num, _assignments->size(), DEFAULT.assignmentsSize);
	unsigned short i = 0;
	for (Assignment* assm : *_assignments) {
		SaveField(fields, "destination" + num + "_" + std::to_string(i), assm->getDestination());
		SaveField(fields, "expression" + num + "_" + std::to_string(i), assm->getExpression());
		i++;
	}
	return fields;
}

bool SequenceStep::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = true;
	try {
	} catch (...) {
		res = false;
	}
	return res;
}

std::map<std::string, std::string>* SequenceStep::_saveInstance() {

}

std::list<SequenceStep::Assignment*>* SequenceStep::getAssignments() const {
	return _assignments;
}

void SequenceStep::setStation(Station* _station) {
	this->_station = _station;
}

Station* SequenceStep::getStation() const {
	return _station;
}