/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Decide.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 9 de Agosto de 2018, 20:39
 */

#include "Decide.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Counter.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Decide::GetPluginInformation;
}
#endif

ModelDataDefinition* Decide::NewInstance(Model* model, std::string name) {
	return new Decide(model, name);
}

Decide::Decide(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Decide>(), name) {
}

List<std::string>* Decide::getConditions() const {
	return _conditions;
}

std::string Decide::show() {
	return ModelComponent::show() + "";
}

void Decide::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double value;
	unsigned short i = 0;
	for (std::list<std::string>::iterator it = _conditions->list()->begin(); it != _conditions->list()->end(); it++) {
		value = _parentModel->parseExpression((*it));
		_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, std::to_string(i + 1) + "th condition evaluated to " + strTruncIfInt(std::to_string(value)) + "  // " + (*it));
		if (value) {
			if (_reportStatistics) {
				_numberOuts->getAtRank(i)->incCountValue();
			}
			_parentModel->sendEntityToComponent(entity, this->getConnections()->getConnectionAtPort(i));
			return;
		}
		i++;
	}
	_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "No condition has been evaluated true");
	if (_reportStatistics) {
		_numberOuts->getAtRank(i)->incCountValue();
	}
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getConnectionAtPort(i));
}

//void Decide::_initBetweenReplications() { // inherited method takes care of it
//	if (_numberOuts != nullptr) {
//		for (Counter* counter : *_numberOuts->list()) {
//			counter->clear();
//		}
//	}
//}

bool Decide::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		unsigned int nv = fields->loadField("conditions", 0);
		for (unsigned int i = 0; i < nv; i++) {
			this->_conditions->insert(fields->loadField("condition" + strIndex(i), ""));
		}
	}
	return res;
}

void Decide::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("conditions", _conditions->size(), 0u, saveDefaultValues);
	unsigned short i = 0;
	for (std::list<std::string>::iterator it = _conditions->list()->begin(); it != _conditions->list()->end(); it++, i++) {
		fields->saveField("condition" + strIndex(i), (*it), "", saveDefaultValues);
	}
}

bool Decide::_check(std::string* errorMessage) {
	bool allResult = true;
	std::string condition;
	for (std::list<std::string>::iterator it = _conditions->list()->begin(); it != _conditions->list()->end(); it++) {
		condition = (*it);
		allResult &= _parentModel->checkExpression(condition, "condition", errorMessage);
	}
	return allResult;
}

void Decide::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		if (_numberOuts == nullptr) {
			_numberOuts = new List<Counter*>();
		}
		for (unsigned int i = _numberOuts->size(); i<this->_connections->size(); i++) {
			Counter* counter = new Counter(_parentModel, getName() + "." + "CountNumberOut" + strIndex(i), this);
			_numberOuts->insert(counter);
			_internalDataInsert("CountNumberOut" + strIndex(i), counter);
		}
	} else if (!_reportStatistics && _numberOuts != nullptr) {
		this->_internalDataClear();
		_numberOuts = nullptr;
	}

}

PluginInformation* Decide::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Decide>(), &Decide::LoadInstance, &Decide::NewInstance);
	info->setCategory("Decisions");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(999);
	std::string help = "This module allows for decision-making processes in the system.";
	help += " It includes options to make decisions based on one or more conditions(for example, if entity type is Gold Card) or based on one or more probabilities(for example, 75 %, true; 25 %, false).";
	help += " Conditions can be based on attribute values(for example, Priority), variable values(for example, Number Denied), the entity type, or an expression(for example, NQ(ProcessA.Queue)).";
	help += " Multiple exit points are shown for each condition or probability and a single Else exit.";
	help += " TYPICAL USES: (1) Dispatching a faulty part for rework; (2) Branching accepted vs. rejected checks; (3) Sending priority customers to a dedicated process.";
	info->setDescriptionHelp(help);
	return info;
}

ModelComponent* Decide::LoadInstance(Model* model, PersistenceRecord *fields) {
	Decide* newComponent = new Decide(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}
