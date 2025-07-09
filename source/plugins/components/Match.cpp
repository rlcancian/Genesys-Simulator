/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Match.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#include "Match.h"

#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Match::GetPluginInformation;
}
#endif

ModelDataDefinition* Match::NewInstance(Model* model, std::string name) {
	return new Match(model, name);
}

std::string Match::convertEnumToStr(Rule rule) {
	switch (static_cast<int> (rule)) {
		case 0: return "Any";
		case 1: return "ByAttribute";
	}
	return "Unknown";
}

Match::Match(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Match>(), name) {
    SimulationControlGenericEnum<Match::Rule, Match>* propRule = new SimulationControlGenericEnum<Match::Rule, Match>(
                                    std::bind(&Match::getRule, this), std::bind(&Match::setRule, this, std::placeholders::_1),
                                    Util::TypeOf<Match>(), getName(), "Rule", "");
	SimulationControlGeneric<unsigned int>* propNumberQueues = new SimulationControlGeneric<unsigned int>(
									std::bind(&Match::getNumberOfQueues, this), std::bind(&Match::setNumberOfQueues, this, std::placeholders::_1),
									Util::TypeOf<Match>(), getName(), "NumberOfQueues", "");
	SimulationControlGeneric<std::string>* propMatchSize = new SimulationControlGeneric<std::string>(
									std::bind(&Match::getMatchSize, this), std::bind(&Match::setMatchSize, this, std::placeholders::_1),
									Util::TypeOf<Match>(), getName(), "MatchSize", "");
	SimulationControlGeneric<std::string>* propAttributeName = new SimulationControlGeneric<std::string>(
									std::bind(&Match::getAttributeName, this), std::bind(&Match::setAttributeName, this, std::placeholders::_1),
									Util::TypeOf<Match>(), getName(), "AttributeName", "");

    _parentModel->getControls()->insert(propRule);
	_parentModel->getControls()->insert(propNumberQueues);
	_parentModel->getControls()->insert(propMatchSize);
	_parentModel->getControls()->insert(propAttributeName);

	// setting properties
    _addProperty(propRule);
	_addProperty(propNumberQueues);
	_addProperty(propMatchSize);
	_addProperty(propAttributeName);
}

std::string Match::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Match::LoadInstance(Model* model, PersistenceRecord *fields) {
	Match* newComponent = new Match(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Match::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	Waiting* waiting = new Waiting(entity, _parentModel->getSimulation()->getSimulatedTime(), this);
	if (_rule == Match::Rule::ByAttribute) {
		double value = entity->getAttributeValue(_attributeName);
		// std::map<Queue*, std::map<double, unsigned int>*>*
		std::pair<Queue*, std::map<double, unsigned int>*> pair1;
		while (_entitiesByAttrib->find(_queues->getAtRank(inputPortNumber)) == _entitiesByAttrib->end()) {
			_entitiesByAttrib->insert({_queues->getAtRank(inputPortNumber), new std::map<double, unsigned int>()});
		}
		pair1 = *(_entitiesByAttrib->find(_queues->getAtRank(inputPortNumber)));
		std::pair<double, unsigned int> pair2;
		while (pair1.second->find(value) == pair1.second->end()) {
			pair1.second->insert({value, 0});
		}
		pair2 = *(pair1.second->find(value));
		pair2.second++;
	}
	_queues->getAtRank(inputPortNumber)->insertElement(waiting);
	unsigned int matchSize = _parentModel->parseExpression(_matchSize);
	unsigned int i = 0;
	if (_rule == Match::Rule::Any) {
		bool foundAll = true;
		while (foundAll && i < _queues->size()) {
			foundAll &= _queues->getAtRank(i++)->size() >= matchSize;
		}
		if (foundAll) {
			// release entities
			Entity* waitingEntity;
			for (Queue* queue : *_queues->list()) {
				for (i = 0; i < matchSize; i++) {
					waiting = queue->first();
					queue->removeElement(waiting);
					// @TODO: Actualize STATISTICS about queue/wait time
					waitingEntity = waiting->getEntity();
					_parentModel->sendEntityToComponent(waitingEntity, this->getConnectionManager()->getFrontConnection(), 0.0);
				}
			}
		}
	} else {
		// by attribute
		bool foundAll = true;
		while (foundAll && i < _queues->size()) {
		}
	}
}

bool Match::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_rule =  static_cast<Rule>(fields->loadField("rule", static_cast<int>(DEFAULT.rule)));
		_matchSize = fields->loadField("matchSize", DEFAULT.matchSize);
		_attributeName = fields->loadField("attributeName", DEFAULT.attributeName);
		//@TODO _queues
	}
	return res;
}

void Match::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("rule", static_cast<int> (_rule), static_cast<int> (DEFAULT.rule), saveDefaultValues);
	fields->saveField("matchSize", _matchSize, DEFAULT.matchSize, saveDefaultValues);
	fields->saveField("attributeName", _attributeName, DEFAULT.attributeName, saveDefaultValues);
	fields->saveField("queues", _queues->size(), 0u, saveDefaultValues);
}

void Match::setRule(Match::Rule _rule) {
	this->_rule = _rule;
}

Match::Rule Match::getRule() const {
	return _rule;
}

void Match::setAttributeName(std::string _attributeName) {
	this->_attributeName = _attributeName;
}

std::string Match::getAttributeName() const {
	return _attributeName;
}

void Match::setMatchSize(std::string _matchSize) {
	this->_matchSize = _matchSize;
}

std::string Match::getMatchSize() const {
	return _matchSize;
}

void Match::setNumberOfQueues(unsigned int _numberOfQueues) {
	this->_numberOfQueues = _numberOfQueues;
}

unsigned int Match::getNumberOfQueues() const {
	return _numberOfQueues;
}

bool Match::_check(std::string * errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

void Match::_createInternalAndAttachedData() {
	while (_queues->size() > _numberOfQueues) {
		this->_internalDataRemove(_queues->last()->getName());
		_internalDataRemove(_queues->last()->getName());
		_queues->remove(_queues->last());
	}
	while (_queues->size() < _numberOfQueues) {
		Queue* newQueue = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Queue>(_parentModel, getName() + ".Queue" + std::to_string(_queues->size()));
		_queues->insert(newQueue);
		_internalDataInsert(newQueue->getName(), newQueue);
	}
}

PluginInformation * Match::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Match>(), &Match::LoadInstance, &Match::NewInstance);
	info->setCategory("Decision");
	info->setMaximumInputs(2);
	info->setMaximumInputs(99);
	//info->getDynamicLibFilenameDependencies()->insert("queue.so");
	// ...
	return info;
}
