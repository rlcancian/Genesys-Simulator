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
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Sequence::GetPluginInformation;
}
#endif

ModelDataDefinition* Sequence::NewInstance(Model* model, std::string name) {
	return new Sequence(model, name);
}

Sequence::Sequence(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Sequence>(), name) {
}

std::string Sequence::show() {
	std::string msg = ModelDataDefinition::show();
	return msg;
}

PluginInformation* Sequence::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Sequence>(), &Sequence::LoadInstance, &Sequence::NewInstance);
	return info;
}

ModelDataDefinition* Sequence::LoadInstance(Model* model, PersistenceRecord *fields) {
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

bool Sequence::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			unsigned short numSteps = fields->loadField("steps", 0);
			for (unsigned short i = 0; i < numSteps; i++) {
				SequenceStep* step = new SequenceStep((Station*)nullptr);
				step->setElementManager(_parentModel->getDataManager());
				step->_loadInstance(fields, i);
				this->_steps->insert(step);
			}
		} catch (...) {
		}
	}
	return res;
}

void Sequence::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("steps", _steps->size(), 0u, saveDefaultValues);
	int i = 0;
	for (SequenceStep* step : *_steps->list()) {
		step->_saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

bool Sequence::_check(std::string* errorMessage) {
	_attachedAttributesInsert({"Entity.Sequence", "Entity.SequenceStep"});
	int i = 0;
	for (SequenceStep* step : *_steps->list()) {
		_attachedDataInsert("StepStation" + strIndex(i), step->getStation());
		_attachedDataInsert("StepLabel" + strIndex(i), step->getLabel());
		i++;
	}
	*errorMessage += "";
	return true;
}

SequenceStep::SequenceStep(Station* station, std::list<Assignment*>* assignments) {
	this->_station = station;
	if (assignments != nullptr)
		_assignments = assignments;
	else
		_assignments = new std::list<Assignment*>();
}

SequenceStep::SequenceStep(Label* label, std::list<Assignment*>* assignments) {
	this->_label = label;
	if (assignments != nullptr)
		_assignments = assignments;
	else
		_assignments = new std::list<Assignment*>();
}

SequenceStep::SequenceStep(Model* model, std::string stationOrLabelName, bool isStation, std::list<Assignment*>* assignments) {
	this->_modeldataManager = model->getDataManager();
	if (isStation) {
		Station* station;
		ModelDataDefinition* data = model->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationOrLabelName);
		if (data != nullptr) {
			station = dynamic_cast<Station*> (data);
		} else {
			station = model->getParentSimulator()->getPlugins()->newInstance<Station>(model, stationOrLabelName);
		}
		this->_station = station;
	} else {//isLabel
		Label* label;
		ModelDataDefinition* data = model->getDataManager()->getDataDefinition(Util::TypeOf<Label>(), stationOrLabelName);
		if (data != nullptr) {
			label = dynamic_cast<Label*> (data);
		} else {
			label = model->getParentSimulator()->getPlugins()->newInstance<Label>(model, stationOrLabelName);
		}
		this->_label = label;
	}
	if (assignments != nullptr)
		_assignments = assignments;
	else
		_assignments = new std::list<Assignment*>();
}

bool SequenceStep::_loadInstance(PersistenceRecord *fields, unsigned int parentIndex) {
	bool res = true;
	std::string num = strIndex(parentIndex);
	std::string destination, expression;
	try {
		std::string stationName = fields->loadField("stepStation" + num, "");
		if (_modeldataManager != nullptr) {
			_station = static_cast<Station*> (_modeldataManager->getDataDefinition(Util::TypeOf<Station>(), stationName));
		}
		std::string labelName = fields->loadField("stepLabel" + num, "");
		if (_modeldataManager != nullptr) {
			_label = static_cast<Label*> (_modeldataManager->getDataDefinition(Util::TypeOf<Label>(), labelName));
		}
		unsigned int assignmentsSize = fields->loadField("stepAssignments" + num, DEFAULT.assignmentsSize);
		for (unsigned short i = 0; i < assignmentsSize; i++) {
			Assignment* assm = new Assignment("", "");
			assm->loadInstance(fields, i);
			_assignments->insert(_assignments->end(), assm);
		}
	} catch (...) {
		res = false;
	}
	return res;
}

void SequenceStep::_saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaultValues) {
	std::string num = strIndex(parentIndex);
	if (_station != nullptr) {
		fields->saveField("stepStation" + num, _station->getName());
	}
	if (_label != nullptr) {
		fields->saveField("stepLabel" + num, _label->getName());
	}
	fields->saveField("stepAssignments" + num, _assignments->size(), DEFAULT.assignmentsSize);
	unsigned short i = 0;
	for (Assignment* assm : *_assignments) {
		assm->saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

bool SequenceStep::_loadInstance(PersistenceRecord *fields) {
	bool res = true;
	try {
	} catch (...) {
		res = false;
	}
	return res;
}

void SequenceStep::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
}

std::list<Assignment*>* SequenceStep::getAssignments() const {
	return _assignments;
}

void SequenceStep::setStation(Station* _station) {
	this->_station = _station;
}

Station* SequenceStep::getStation() const {
	return _station;
}

void SequenceStep::setElementManager(ModelDataManager* _modeldataManager) {
	this->_modeldataManager = _modeldataManager;
}

void SequenceStep::setLabel(Label* _label) {
	this->_label = _label;
}

Label* SequenceStep::getLabel() const {
	return _label;
}
