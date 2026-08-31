#include "plugins/data/ModalModel/MarkovChainNetwork.h"

#include "kernel/TraitsKernel.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "kernel/statistics/Sampler_if.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <unordered_map>

namespace {

std::string doubleToPersistentString(double value) {
	std::ostringstream stream;
	stream << std::setprecision(17) << value;
	return stream.str();
}

}

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MarkovChainNetwork::GetPluginInformation;
}
#endif

MarkovChainNetwork::MarkovTransition::MarkovTransition(MarkovState* source, MarkovState* destination, double probability, std::string name)
	: _source(source), _destination(destination), _probability(probability), _name(name) {
}

void MarkovChainNetwork::MarkovTransition::setSource(MarkovState* source) {
	_source = source;
}

MarkovState* MarkovChainNetwork::MarkovTransition::getSource() const {
	return _source;
}

void MarkovChainNetwork::MarkovTransition::setDestination(MarkovState* destination) {
	_destination = destination;
}

MarkovState* MarkovChainNetwork::MarkovTransition::getDestination() const {
	return _destination;
}

void MarkovChainNetwork::MarkovTransition::setProbability(double probability) {
	_probability = probability;
}

double MarkovChainNetwork::MarkovTransition::getProbability() const {
	return _probability;
}

void MarkovChainNetwork::MarkovTransition::setName(std::string name) {
	_name = name;
}

std::string MarkovChainNetwork::MarkovTransition::getName() const {
	return _name;
}

MarkovChainNetwork::MarkovChainNetwork(Model* model, std::string name)
	: DefaultNetwork(model, name, Util::TypeOf<MarkovChainNetwork>()) {
	addInputPort("step");
	addOutputPort("state");
	_sampler = new TraitsKernel<Sampler_if>::Implementation();
}

MarkovChainNetwork::~MarkovChainNetwork() {
	delete _states;
	delete _transitions;
	delete _sampler;
	_sampler = nullptr;
}

ModelDataDefinition* MarkovChainNetwork::NewInstance(Model* model, std::string name) {
	return new MarkovChainNetwork(model, name);
}

PluginInformation* MarkovChainNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MarkovChainNetwork>(), &MarkovChainNetwork::LoadInstance, &MarkovChainNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Finite time-homogeneous DTMC network. One activation samples exactly one transition from the current state.");
	return info;
}

ModelDataDefinition* MarkovChainNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	MarkovChainNetwork* network = new MarkovChainNetwork(model);
	try {
		network->_loadInstance(fields);
	} catch (const std::exception& e) {
		network->traceError("Failed to load MarkovChainNetwork instance: " + std::string(e.what()));
	}
	return network;
}

void MarkovChainNetwork::addState(MarkovState* state) {
	if (state != nullptr && _states->find(state) == _states->list()->end()) {
		_states->insert(state);
		if (_initialState == nullptr || state->isInitialNode()) {
			_initialState = state;
			_currentState = state;
		}
	}
}

void MarkovChainNetwork::removeState(MarkovState* state) {
	if (state == nullptr) {
		return;
	}
	for (auto it = _transitions->list()->begin(); it != _transitions->list()->end();) {
		MarkovTransition* transition = *it;
		if (transition != nullptr && (transition->getSource() == state || transition->getDestination() == state)) {
			it = _transitions->list()->erase(it);
		} else {
			++it;
		}
	}
	_states->remove(state);
	if (_initialState == state) {
		_initialState = nullptr;
	}
	if (_currentState == state) {
		_currentState = nullptr;
	}
}

List<MarkovState*>* MarkovChainNetwork::getStates() const {
	return _states;
}

void MarkovChainNetwork::addTransition(MarkovTransition* transition) {
	if (transition != nullptr && _transitions->find(transition) == _transitions->list()->end()) {
		_transitions->insert(transition);
	}
}

void MarkovChainNetwork::removeTransition(MarkovTransition* transition) {
	_transitions->remove(transition);
}

List<MarkovChainNetwork::MarkovTransition*>* MarkovChainNetwork::getTransitions() const {
	return _transitions;
}

std::vector<MarkovChainNetwork::MarkovTransition*> MarkovChainNetwork::getOutgoingTransitions(MarkovState* state) const {
	std::vector<MarkovTransition*> outgoing;
	for (MarkovTransition* transition : *_transitions->list()) {
		if (transition != nullptr && transition->getSource() == state) {
			outgoing.push_back(transition);
		}
	}
	return outgoing;
}

void MarkovChainNetwork::setInitialState(MarkovState* state) {
	_initialState = state;
	_currentState = state;
	if (state != nullptr) {
		state->setInitialNode(true);
		addState(state);
	}
}

MarkovState* MarkovChainNetwork::getInitialState() const {
	return _initialState;
}

void MarkovChainNetwork::setCurrentState(MarkovState* state) {
	_currentState = state;
	if (state != nullptr) {
		addState(state);
	}
}

MarkovState* MarkovChainNetwork::getCurrentState() const {
	return _currentState;
}

unsigned int MarkovChainNetwork::getStateIndex(MarkovState* state) const {
	unsigned int index = 0;
	for (MarkovState* existing : *_states->list()) {
		if (existing == state) {
			return index;
		}
		index++;
	}
	return std::numeric_limits<unsigned int>::max();
}

MarkovState* MarkovChainNetwork::getStateAt(unsigned int index) const {
	return index < _states->size() ? _states->getAtRank(index) : nullptr;
}

double MarkovChainNetwork::getProbabilityTolerance() const {
	return _probabilityTolerance;
}

void MarkovChainNetwork::setProbabilityTolerance(double tolerance) {
	_probabilityTolerance = tolerance;
}

void MarkovChainNetwork::resetSampler() {
	auto* defaultSampler = dynamic_cast<SamplerDefaultImpl1*>(_sampler);
	if (defaultSampler != nullptr) {
		defaultSampler->reset();
	}
}

std::string MarkovChainNetwork::show() {
	return DefaultNetwork::show() +
	       ", states=" + std::to_string(_states->size()) +
	       ", transitions=" + std::to_string(_transitions->size()) +
	       ", initialState=\"" + (_initialState != nullptr ? _initialState->getName() : "") + "\"" +
	       ", currentState=\"" + (_currentState != nullptr ? _currentState->getName() : "") + "\"";
}

bool MarkovChainNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNetwork::_loadInstance(fields);
	if (res) {
		_states->clear();
		_transitions->clear();
		_initialState = nullptr;
		_currentState = nullptr;
		_probabilityTolerance = fields->loadField("probabilityTolerance", 1e-9);

		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		std::unordered_map<std::string, MarkovState*> statesByName;
		unsigned int statesSize = fields->loadField("markovStatesSize", 0u);
		for (unsigned int i = 0; i < statesSize; i++) {
			const std::string prefix = "markovState" + Util::StrIndex(i) + ".";
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

			std::string stateType = stateFields->loadField("typename", Util::TypeOf<MarkovState>());
			ModelDataDefinition* loaded = nullptr;
			Plugin* statePlugin = plugins != nullptr ? plugins->find(stateType) : nullptr;
			if (statePlugin != nullptr) {
				loaded = statePlugin->loadNew(_parentModel, stateFields.get());
			} else {
				loaded = MarkovState::LoadInstance(_parentModel, stateFields.get());
			}
			MarkovState* state = dynamic_cast<MarkovState*>(loaded);
			if (state == nullptr) {
				traceError("Loaded Markov state is not a MarkovState for typename \"" + stateType + "\"");
				continue;
			}
			state->setModelLevel(_id);
			addState(state);
			statesByName[state->getName()] = state;
		}

		unsigned int transitionsSize = fields->loadField("markovTransitionsSize", 0u);
		for (unsigned int i = 0; i < transitionsSize; i++) {
			const std::string suffix = Util::StrIndex(i);
			auto sourceIt = statesByName.find(fields->loadField("markovTransitionSource" + suffix, std::string("")));
			auto destinationIt = statesByName.find(fields->loadField("markovTransitionDestination" + suffix, std::string("")));
			if (sourceIt == statesByName.end() || destinationIt == statesByName.end()) {
				traceError("Skipping Markov transition with unknown source/destination while loading \"" + getName() + "\"");
				continue;
			}
			MarkovTransition* transition = new MarkovTransition(
				sourceIt->second,
				destinationIt->second,
				fields->loadField("markovTransitionProbability" + suffix, 0.0),
				fields->loadField("markovTransitionName" + suffix, "P" + suffix));
			addTransition(transition);
		}

		const std::string initialStateName = fields->loadField("initialState", std::string(""));
		if (initialStateName != "" && statesByName.find(initialStateName) != statesByName.end()) {
			_initialState = statesByName[initialStateName];
		}
		const std::string currentStateName = fields->loadField("currentState", std::string(""));
		if (currentStateName != "" && statesByName.find(currentStateName) != statesByName.end()) {
			_currentState = statesByName[currentStateName];
		}
		if (_initialState == nullptr) {
			_initialState = _resolveInitialState();
		}
		if (_currentState == nullptr) {
			_currentState = _initialState;
		}
		resetSampler();
	}
	return res;
}

void MarkovChainNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNetwork::_saveInstance(fields, saveDefaultValues);
	if (saveDefaultValues || _probabilityTolerance != 1e-9) {
		fields->saveField("probabilityTolerance", doubleToPersistentString(_probabilityTolerance), std::string(""), true);
	}
	if (_initialState != nullptr) {
		fields->saveField("initialState", _initialState->getName(), std::string(""), saveDefaultValues);
	}
	if (_currentState != nullptr) {
		fields->saveField("currentState", _currentState->getName(), std::string(""), saveDefaultValues);
	}

	fields->saveField("markovStatesSize", _states->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (MarkovState* state : *_states->list()) {
		const std::string prefix = "markovState" + Util::StrIndex(i) + ".";
		auto stateFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(stateFields.get(), state);
		for (auto it = stateFields->begin(); it != stateFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	fields->saveField("markovTransitionsSize", _transitions->size(), 0u, saveDefaultValues);
	i = 0;
	for (MarkovTransition* transition : *_transitions->list()) {
		const std::string suffix = Util::StrIndex(i);
		if (transition->getSource() != nullptr) {
			fields->saveField("markovTransitionSource" + suffix, transition->getSource()->getName(), std::string(""), saveDefaultValues);
		}
		if (transition->getDestination() != nullptr) {
			fields->saveField("markovTransitionDestination" + suffix, transition->getDestination()->getName(), std::string(""), saveDefaultValues);
		}
		fields->saveField("markovTransitionName" + suffix, transition->getName(), std::string(""), saveDefaultValues);
		if (saveDefaultValues || transition->getProbability() != 0.0) {
			fields->saveField("markovTransitionProbability" + suffix, doubleToPersistentString(transition->getProbability()), std::string(""), true);
		}
		i++;
	}
}

bool MarkovChainNetwork::_check(std::string& errorMessage) {
	bool resultAll = DefaultNetwork::_check(errorMessage);
	if (!std::isfinite(_probabilityTolerance) || _probabilityTolerance < 0.0) {
		errorMessage += "MarkovChainNetwork \"" + getName() + "\" requires a nonnegative finite probability tolerance. ";
		resultAll = false;
	}
	if (_states->size() == 0) {
		errorMessage += "MarkovChainNetwork \"" + getName() + "\" requires at least one state. ";
		resultAll = false;
	}
	if (_resolveInitialState() == nullptr) {
		errorMessage += "MarkovChainNetwork \"" + getName() + "\" requires an initial state. ";
		resultAll = false;
	}

	std::unordered_map<MarkovState*, double> rowSums;
	for (MarkovState* state : *_states->list()) {
		rowSums[state] = 0.0;
	}

	for (MarkovTransition* transition : *_transitions->list()) {
		if (transition == nullptr) {
			errorMessage += "MarkovChainNetwork \"" + getName() + "\" has a null transition reference. ";
			resultAll = false;
			continue;
		}
		if (transition->getSource() == nullptr || transition->getDestination() == nullptr) {
			errorMessage += "MarkovChainNetwork \"" + getName() + "\" has transition \"" + transition->getName() + "\" with null endpoint. ";
			resultAll = false;
			continue;
		}
		if (!_hasState(transition->getSource()) || !_hasState(transition->getDestination())) {
			errorMessage += "MarkovChainNetwork \"" + getName() + "\" has transition \"" + transition->getName() + "\" referencing state outside the chain. ";
			resultAll = false;
		}
		const double probability = transition->getProbability();
		if (!std::isfinite(probability) || probability < -_probabilityTolerance || probability > 1.0 + _probabilityTolerance) {
			errorMessage += "MarkovChainNetwork \"" + getName() + "\" transition \"" + transition->getName() + "\" probability must be in [0,1]. ";
			resultAll = false;
		} else if (_hasState(transition->getSource())) {
			rowSums[transition->getSource()] += probability;
		}
	}

	for (MarkovState* state : *_states->list()) {
		const double rowSum = rowSums[state];
		if (std::fabs(rowSum - 1.0) > _probabilityTolerance) {
			errorMessage += "MarkovChainNetwork \"" + getName() + "\" transition probabilities from state \"" +
			                (state != nullptr ? state->getName() : "") + "\" must sum to 1.0. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void MarkovChainNetwork::_initBetweenReplications() {
	DefaultNetwork::_initBetweenReplications();
	_currentState = _resolveInitialState();
	resetSampler();
}

NetworkActivationResult MarkovChainNetwork::_activate(const NetworkActivationFrame& frame) {
	(void)frame;
	NetworkActivationResult result(getNumOutputPorts());
	if (_currentState == nullptr) {
		_currentState = _resolveInitialState();
	}
	if (_currentState == nullptr) {
		return result;
	}

	std::vector<MarkovTransition*> outgoing = getOutgoingTransitions(_currentState);
	if (outgoing.empty()) {
		return result;
	}

	const unsigned int selectedIndex = _sampleNextStateIndex(outgoing);
	MarkovTransition* selectedTransition = selectedIndex < outgoing.size() ? outgoing[selectedIndex] : nullptr;
	if (selectedTransition == nullptr || selectedTransition->getDestination() == nullptr) {
		return result;
	}
	_currentState = selectedTransition->getDestination();
	if (getNumOutputPorts() > 0) {
		const unsigned int stateIndex = getStateIndex(_currentState);
		if (stateIndex != std::numeric_limits<unsigned int>::max()) {
			result.setPresent(0, static_cast<double>(stateIndex));
		}
	}
	return result;
}

MarkovState* MarkovChainNetwork::_resolveInitialState() {
	if (_initialState != nullptr) {
		return _initialState;
	}
	for (MarkovState* state : *_states->list()) {
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

unsigned int MarkovChainNetwork::_sampleNextStateIndex(const std::vector<MarkovTransition*>& outgoing) {
	if (outgoing.empty()) {
		return 0u;
	}
	std::vector<double> probabilities;
	std::vector<double> values;
	probabilities.reserve(outgoing.size());
	values.reserve(outgoing.size());
	for (unsigned int i = 0; i < outgoing.size(); i++) {
		probabilities.push_back(outgoing[i] != nullptr ? outgoing[i]->getProbability() : 0.0);
		values.push_back(static_cast<double>(i));
	}
	return static_cast<unsigned int>(_sampler->sampleDiscrete(probabilities.data(), values.data(), static_cast<int>(outgoing.size())));
}

bool MarkovChainNetwork::_hasState(MarkovState* state) const {
	return state != nullptr && _states->find(state) != _states->list()->end();
}
