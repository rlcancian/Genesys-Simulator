/*
 * File:   EFSMNetwork.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#include "plugins/data/ModalModel/EFSMNetwork.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include <algorithm>
#include <memory>
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &EFSMNetwork::GetPluginInformation;
}
#endif

EFSMNetwork::EFSMNetwork(Model* model, std::string name)
	: DefaultNetwork(model, name, Util::TypeOf<EFSMNetwork>()) {
	addInputPort("input");
	addOutputPort("output");
}

ModelDataDefinition* EFSMNetwork::NewInstance(Model* model, std::string name) {
	return new EFSMNetwork(model, name);
}

PluginInformation* EFSMNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<EFSMNetwork>(), &EFSMNetwork::LoadInstance, &EFSMNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Extended finite-state-machine network with explicit DefaultNetwork activation semantics.");
	return info;
}

ModelDataDefinition* EFSMNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	EFSMNetwork* network = new EFSMNetwork(model);
	try {
		network->_loadInstance(fields);
	} catch (const std::exception& e) {
		network->traceError("Failed to load EFSMNetwork instance: " + std::string(e.what()));
	}
	return network;
}

void EFSMNetwork::addState(FSMState* state) {
	if (state != nullptr && _states->find(state) == _states->list()->end()) {
		_states->insert(state);
		if (_initialState == nullptr || state->isInitialNode()) {
			_initialState = state;
			_currentState = state;
		}
	}
}

void EFSMNetwork::removeState(FSMState* state) {
	_states->remove(state);
	if (_initialState == state) {
		_initialState = nullptr;
	}
	if (_currentState == state) {
		_currentState = nullptr;
	}
}

List<FSMState*>* EFSMNetwork::getStates() const {
	return _states;
}

void EFSMNetwork::addTransition(EFSMTransition* transition) {
	if (transition != nullptr && _transitions->find(transition) == _transitions->list()->end()) {
		_transitions->insert(transition);
		if (transition->getSource() != nullptr) {
			transition->getSource()->addTransition(transition);
		}
	}
}

void EFSMNetwork::removeTransition(EFSMTransition* transition) {
	_transitions->remove(transition);
	if (transition != nullptr && transition->getSource() != nullptr) {
		transition->getSource()->removeTransition(transition);
	}
}

List<EFSMTransition*>* EFSMNetwork::getTransitions() const {
	return _transitions;
}

void EFSMNetwork::setInitialState(FSMState* state) {
	_initialState = state;
	_currentState = state;
	if (state != nullptr) {
		state->setInitialNode(true);
		addState(state);
	}
}

FSMState* EFSMNetwork::getInitialState() const {
	return _initialState;
}

FSMState* EFSMNetwork::getCurrentState() const {
	return _currentState;
}

void EFSMNetwork::setCurrentState(FSMState* state) {
	_currentState = state;
	if (state != nullptr) {
		addState(state);
	}
}

std::string EFSMNetwork::show() {
	return DefaultNetwork::show() +
	       ", states=" + std::to_string(_states->size()) +
	       ", transitions=" + std::to_string(_transitions->size()) +
	       ", initialState=\"" + (_initialState != nullptr ? _initialState->getName() : "") + "\"" +
	       ", currentState=\"" + (_currentState != nullptr ? _currentState->getName() : "") + "\"";
}

bool EFSMNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNetwork::_loadInstance(fields);
	if (res) {
		_states->clear();
		_transitions->clear();
		_initialState = nullptr;
		_currentState = nullptr;

		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		std::unordered_map<std::string, FSMState*> statesByName;
		unsigned int statesSize = fields->loadField("statesSize", 0u);
		for (unsigned int i = 0; i < statesSize; i++) {
			const std::string prefix = "state" + Util::StrIndex(i) + ".";
			auto stateFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
			for (auto it = fields->begin(); it != fields->end(); ++it) {
				if (it->first.rfind(prefix, 0) == 0) {
					PersistenceRecord::Entry entry = it->second;
					entry.first = it->first.substr(prefix.size());
					stateFields->insert(entry);
				}
			}
			if (stateFields->size() == 0) {
				continue;
			}

			std::string stateType = stateFields->loadField("typename", Util::TypeOf<FSMState>());
			Plugin* statePlugin = plugins->find(stateType);
			if (statePlugin == nullptr) {
				traceError("Could not load EFSM state plugin \"" + stateType + "\" while loading EFSMNetwork \"" + getName() + "\"");
				continue;
			}

			ModelDataDefinition* loaded = statePlugin->loadNew(_parentModel, stateFields.get());
			FSMState* state = dynamic_cast<FSMState*>(loaded);
			if (state == nullptr) {
				traceError("Loaded EFSM state is not an FSMState for typename \"" + stateType + "\"");
				continue;
			}
			state->setModelLevel(_id);
			addState(state);
			statesByName[state->getName()] = state;
		}

		unsigned int transitionsSize = fields->loadField("transitionsSize", 0u);
		for (unsigned int i = 0; i < transitionsSize; i++) {
			const std::string suffix = Util::StrIndex(i);
			auto sourceIt = statesByName.find(fields->loadField("transitionSource" + suffix, std::string("")));
			auto destinationIt = statesByName.find(fields->loadField("transitionDestination" + suffix, std::string("")));
			if (sourceIt == statesByName.end() || destinationIt == statesByName.end()) {
				traceError("Skipping EFSM transition with unknown source/destination while loading \"" + getName() + "\"");
				continue;
			}
			EFSMTransition* transition = new EFSMTransition(sourceIt->second, destinationIt->second,
				fields->loadField("transitionName" + suffix, "T" + suffix));
			transition->setGuardExpression(fields->loadField("transitionGuard" + suffix, std::string("")));
			transition->setOutputExpression(fields->loadField("transitionOutput" + suffix, std::string("")));
			transition->setInputEvent(fields->loadField("transitionInputEvent" + suffix, std::string("")));
			transition->setPriority(fields->loadField("transitionPriority" + suffix, 0u));
			transition->setProbabilityExpression(fields->loadField("transitionProbabilityExpression" + suffix, std::string("")));
			addTransition(transition);
		}

		std::string initialStateName = fields->loadField("initialState", std::string(""));
		if (initialStateName != "" && statesByName.find(initialStateName) != statesByName.end()) {
			_initialState = statesByName[initialStateName];
		}
		std::string currentStateName = fields->loadField("currentState", std::string(""));
		if (currentStateName != "" && statesByName.find(currentStateName) != statesByName.end()) {
			_currentState = statesByName[currentStateName];
		}
		if (_initialState == nullptr) {
			_initialState = _resolveInitialState();
		}
		if (_currentState == nullptr) {
			_currentState = _initialState;
		}
	}
	return res;
}

void EFSMNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNetwork::_saveInstance(fields, saveDefaultValues);
	if (_initialState != nullptr) {
		fields->saveField("initialState", _initialState->getName(), std::string(""), saveDefaultValues);
	}
	if (_currentState != nullptr) {
		fields->saveField("currentState", _currentState->getName(), std::string(""), saveDefaultValues);
	}
	fields->saveField("statesSize", _states->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (FSMState* state : *_states->list()) {
		const std::string prefix = "state" + Util::StrIndex(i) + ".";
		auto stateFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(stateFields.get(), state);
		for (auto it = stateFields->begin(); it != stateFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	fields->saveField("transitionsSize", _transitions->size(), 0u, saveDefaultValues);
	i = 0;
	for (EFSMTransition* transition : *_transitions->list()) {
		const std::string suffix = Util::StrIndex(i);
		if (transition->getSource() != nullptr) {
			fields->saveField("transitionSource" + suffix, transition->getSource()->getName(), std::string(""), saveDefaultValues);
		}
		if (transition->getDestination() != nullptr) {
			fields->saveField("transitionDestination" + suffix, transition->getDestination()->getName(), std::string(""), saveDefaultValues);
		}
		fields->saveField("transitionName" + suffix, transition->getName(), std::string(""), saveDefaultValues);
		fields->saveField("transitionGuard" + suffix, transition->getGuardExpression(), std::string(""), saveDefaultValues);
		fields->saveField("transitionOutput" + suffix, transition->getOutputExpression(), std::string(""), saveDefaultValues);
		fields->saveField("transitionInputEvent" + suffix, transition->getInputEvent(), std::string(""), saveDefaultValues);
		fields->saveField("transitionPriority" + suffix, transition->getPriority(), 0u, saveDefaultValues);
		fields->saveField("transitionProbabilityExpression" + suffix, transition->getProbabilityExpression(), std::string(""), saveDefaultValues);
		i++;
	}
}

bool EFSMNetwork::_check(std::string& errorMessage) {
	bool resultAll = DefaultNetwork::_check(errorMessage);
	if (_states->size() == 0) {
		errorMessage += "EFSMNetwork \"" + getName() + "\" requires at least one state. ";
		resultAll = false;
	}
	if (_resolveInitialState() == nullptr) {
		errorMessage += "EFSMNetwork \"" + getName() + "\" requires an initial state. ";
		resultAll = false;
	}
	for (EFSMTransition* transition : *_transitions->list()) {
		if (transition->getSource() == nullptr || transition->getDestination() == nullptr) {
			errorMessage += "EFSMNetwork \"" + getName() + "\" has transition \"" + transition->getName() + "\" with null endpoint. ";
			resultAll = false;
		}
		if (transition->getGuardExpression() != "") {
			resultAll &= _parentModel->checkExpression(transition->getGuardExpression(), "EFSM guard[" + transition->getName() + "]", errorMessage);
		}
		if (transition->getOutputExpression() != "") {
			resultAll &= _parentModel->checkExpression(transition->getOutputExpression(), "EFSM output[" + transition->getName() + "]", errorMessage);
		}
		if (transition->getProbabilityExpression() != "") {
			resultAll &= _parentModel->checkExpression(transition->getProbabilityExpression(), "EFSM probability[" + transition->getName() + "]", errorMessage);
		}
	}
	return resultAll;
}

void EFSMNetwork::_initBetweenReplications() {
	DefaultNetwork::_initBetweenReplications();
	_currentState = _resolveInitialState();
}

NetworkActivationResult EFSMNetwork::_activate(const NetworkActivationFrame& frame) {
	(void)frame;
	NetworkActivationResult result(getNumOutputPorts());
	if (_currentState == nullptr) {
		_currentState = _resolveInitialState();
	}
	if (_currentState == nullptr) {
		return result;
	}

	std::vector<EFSMTransition*> enabled;
	for (DefaultNodeTransition* transition : *_currentState->getTransitions()->list()) {
		EFSMTransition* efsmTransition = dynamic_cast<EFSMTransition*>(transition);
		if (efsmTransition != nullptr && efsmTransition->canFire(_parentModel, nullptr)) {
			enabled.push_back(efsmTransition);
		}
	}
	if (enabled.empty()) {
		return result;
	}
	std::sort(enabled.begin(), enabled.end(), [](EFSMTransition* a, EFSMTransition* b) {
		return a->getPriority() < b->getPriority();
	});

	EFSMTransition* chosen = enabled.front();
	FSMState* source = dynamic_cast<FSMState*>(chosen->getSource());
	FSMState* destination = dynamic_cast<FSMState*>(chosen->getDestination());
	if (source != nullptr && source->getExitActionExpression() != "") {
		_parentModel->parseExpression(source->getExitActionExpression());
	}
	if (chosen->getOutputExpression() != "" && getNumOutputPorts() > 0) {
		result.setPresent(0, _parentModel->parseExpression(chosen->getOutputExpression()));
	}
	_currentState = destination;
	if (_currentState != nullptr && _currentState->getEntryActionExpression() != "") {
		_parentModel->parseExpression(_currentState->getEntryActionExpression());
	}
	return result;
}

FSMState* EFSMNetwork::_resolveInitialState() {
	if (_initialState != nullptr) {
		return _initialState;
	}
	for (FSMState* state : *_states->list()) {
		if (state != nullptr && state->isInitialNode()) {
			_initialState = state;
			return _initialState;
		}
	}
	if (_states->size() > 0) {
		_initialState = _states->front();
	}
	return _initialState;
}
