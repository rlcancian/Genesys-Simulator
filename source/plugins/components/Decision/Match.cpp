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

#include "plugins/components/Decision/Match.h"

#include <algorithm>
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

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
	if (_queues == nullptr || _queues->size() != _numberOfQueues) {
		_createInternalAndAttachedData();
	}
	if (_queues == nullptr || _queues->size() != _numberOfQueues) {
		traceError("Match internal queues are not initialized.", TraceManager::Level::L1_errorFatal);
		return;
	}
	if (inputPortNumber >= _queues->size()) {
		traceError("Match received entity on invalid input port " + std::to_string(inputPortNumber) + ".", TraceManager::Level::L3_errorRecover);
		return;
	}

	unsigned int matchSize = static_cast<unsigned int>(_parentModel->parseExpression(_matchSize));
	if (matchSize == 0) {
		traceError("MatchSize evaluated to zero. Entity will remain waiting.", TraceManager::Level::L3_errorRecover);
		return;
	}

	double tnow = 0.0;
	if (_parentModel->getSimulation() != nullptr) {
		tnow = _parentModel->getSimulation()->getSimulatedTime();
	}
	Waiting* waiting = new Waiting(entity, tnow, this);
	_queues->getAtRank(inputPortNumber)->insertElement(waiting);

	if (_rule == Match::Rule::Any) {
		bool foundAll = true;
		for (unsigned int i = 0; foundAll && i < _queues->size(); ++i) {
			foundAll = foundAll && (_queues->getAtRank(i)->size() >= matchSize);
		}
		if (foundAll) {
			// release entities
			Entity* waitingEntity = nullptr;
			for (Queue* queue : *_queues->list()) {
				for (unsigned int i = 0; i < matchSize; i++) {
					waiting = queue->first();
					if (waiting == nullptr) {
						break;
					}
					waitingEntity = waiting->getEntity();
					queue->removeElement(waiting);
					_parentModel->sendEntityToComponent(waitingEntity, this->getConnectionManager()->getFrontConnection(), 0.0);
				}
			}
		}
	} else if (_rule == Match::Rule::ByAttribute) {
		std::list<double> candidateValues;
		Queue* firstQueue = _queues->size() > 0 ? _queues->getAtRank(0) : nullptr;
		if (firstQueue != nullptr) {
			for (unsigned int i = 0; i < firstQueue->size(); ++i) {
				Waiting* queueWaiting = firstQueue->getAtRank(i);
				if (queueWaiting != nullptr) {
					double candidate = queueWaiting->getEntity()->getAttributeValue(_attributeName);
					if (std::find(candidateValues.begin(), candidateValues.end(), candidate) == candidateValues.end()) {
						candidateValues.push_back(candidate);
					}
				}
			}
		}
		double selectedValue = 0.0;
		bool foundMatch = false;
		for (double candidate : candidateValues) {
			bool candidateInAllQueues = true;
			for (unsigned int q = 0; candidateInAllQueues && q < _queues->size(); ++q) {
				Queue* queue = _queues->getAtRank(q);
				unsigned int countInQueue = 0;
				for (unsigned int i = 0; i < queue->size(); ++i) {
					Waiting* queueWaiting = queue->getAtRank(i);
					if (queueWaiting != nullptr && queueWaiting->getEntity()->getAttributeValue(_attributeName) == candidate) {
						++countInQueue;
						if (countInQueue >= matchSize) {
							break;
						}
					}
				}
				candidateInAllQueues = countInQueue >= matchSize;
			}
			if (candidateInAllQueues) {
				selectedValue = candidate;
				foundMatch = true;
				break;
			}
		}

		if (foundMatch) {
			for (unsigned int q = 0; q < _queues->size(); ++q) {
				Queue* queue = _queues->getAtRank(q);
				unsigned int released = 0;
				for (unsigned int i = 0; i < queue->size() && released < matchSize; /* no increment */) {
					Waiting* queueWaiting = queue->getAtRank(i);
					if (queueWaiting != nullptr && queueWaiting->getEntity()->getAttributeValue(_attributeName) == selectedValue) {
						Entity* waitingEntity = queueWaiting->getEntity();
						queue->removeElement(queueWaiting);
						_parentModel->sendEntityToComponent(waitingEntity, this->getConnectionManager()->getFrontConnection(), 0.0);
						++released;
						continue;
					}
					++i;
				}
			}
		}
	}
}

bool Match::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_rule =  static_cast<Rule>(fields->loadField("rule", static_cast<int>(DEFAULT.rule)));
		_numberOfQueues = fields->loadField("numberOfQueues", DEFAULT.numberOfQueues);
		_matchSize = fields->loadField("matchSize", DEFAULT.matchSize);
		_attributeName = fields->loadField("attributeName", DEFAULT.attributeName);
	}
	return res;
}

void Match::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("rule", static_cast<int> (_rule), static_cast<int> (DEFAULT.rule), saveDefaultValues);
	fields->saveField("numberOfQueues", _numberOfQueues, DEFAULT.numberOfQueues, saveDefaultValues);
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

bool Match::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_numberOfQueues < 2) {
		errorMessage += "NumberOfQueues must be at least 2. ";
		resultAll = false;
	}
	resultAll &= _parentModel->checkExpression(_matchSize, "MatchSize", errorMessage);
	const bool attributeMandatory = _rule == Match::Rule::ByAttribute;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), _attributeName, "AttributeName", attributeMandatory, errorMessage);
	return resultAll;
}

void Match::_createInternalAndAttachedData() {
	while (_queues->size() > _numberOfQueues) {
		Queue* obsoleteQueue = _queues->last();
		_internalDataRemove(obsoleteQueue->getName());
		_queues->remove(_queues->last());
		_entitiesByAttrib->erase(obsoleteQueue);
	}
	while (_queues->size() < _numberOfQueues) {
		Queue* newQueue = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Queue>(_parentModel, getName() + ".Queue" + std::to_string(_queues->size()));
		if (newQueue == nullptr) {
			newQueue = new Queue(_parentModel, getName() + ".Queue" + std::to_string(_queues->size()));
		}
		ModelDataDefinition::CreateInternalData(newQueue);
		_queues->insert(newQueue);
		_internalDataInsert(newQueue->getName(), newQueue);
	}
	for (Queue* queue : *_queues->list()) {
		if (queue != nullptr) {
			_internalDataInsert(queue->getName(), queue);
		}
	}
}

PluginInformation * Match::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Match>(), &Match::LoadInstance, &Match::NewInstance);
	info->setCategory("Decision");
	info->setMaximumInputs(99);
	//info->getDynamicLibFilenameDependencies()->insert("queue.so");
	// ...
	return info;
}
