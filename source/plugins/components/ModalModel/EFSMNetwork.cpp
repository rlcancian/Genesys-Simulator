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
#include "kernel/TraitsKernel.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "kernel/statistics/Sampler_if.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/data/Logic/Variable.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <optional>
#include <exception>
#include <functional>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
std::string stateAssociationKey(const FSMState* state) {
	return "EFSMNetwork.State." + std::to_string(state != nullptr ? state->getId() : 0);
}

struct TransitionAction {
    std::string destination;
    std::string expression;
};

// Supported GenESyS action subset: portName=expression (output) or
// existingVariableName=expression (commit), optionally separated by semicolons.
// Ptolemy II's channel indexing and refinement-qualified destinations need
// capabilities the current numeric NetworkActivationFrame does not provide.
bool parseTransitionActions(const std::string& source, std::vector<TransitionAction>* result) {
    if (result == nullptr) return false;
    result->clear();
    if (source.empty()) return true;
    const auto trim = [](std::string text) {
        const auto nonSpace = [](unsigned char c) { return !std::isspace(c); };
        auto begin = std::find_if(text.begin(), text.end(), nonSpace);
        auto end = std::find_if(text.rbegin(), text.rend(), nonSpace).base();
        return begin >= end ? std::string{} : std::string(begin, end);
    };
    std::size_t cursor = 0;
    std::unordered_set<std::string> destinations;
    while (cursor < source.size()) {
        const std::size_t separator = source.find(';', cursor);
        const std::string clause = trim(source.substr(cursor, separator == std::string::npos
            ? std::string::npos : separator - cursor));
        if (clause.empty()) return false; // reject consecutive separators
        const std::size_t equals = clause.find('=');
        if (equals == std::string::npos) return false;
        const std::string destination = trim(clause.substr(0, equals));
        const std::string expression = trim(clause.substr(equals + 1));
        if (destination.empty() || expression.empty()) return false;
        const auto isInitial = [](unsigned char ch) {
            return std::isalpha(ch) != 0 || ch == '_';
        };
        const auto isTail = [](unsigned char ch) {
            return std::isalnum(ch) != 0 || ch == '_';
        };
        if (!isInitial(static_cast<unsigned char>(destination.front()))
            || !std::all_of(destination.begin() + 1, destination.end(), isTail)
            || !destinations.insert(destination).second)
            return false;
        result->push_back({destination, expression});
        if (separator == std::string::npos) break;
        cursor = separator + 1;
    }
    return true;
}
}

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &EFSMNetwork::GetPluginInformation;
}
#endif

EFSMNetwork::EFSMNetwork(Model* model, std::string name)
	: DefaultNetwork(model, name, Util::TypeOf<EFSMNetwork>()) {
	addInputPort("input");
	addOutputPort("output");
	_sampler = new TraitsKernel<Sampler_if>::Implementation();

    auto* bound = new SimulationControlGeneric<unsigned int>(
        [this]() { return getMaxImmediateTransitionsPerActivation(); },
        [this](unsigned int count) { setMaxImmediateTransitionsPerActivation(count); },
        Util::TypeOf<EFSMNetwork>(), getName(), "Max Immediate Transitions Per Activation",
        "Positive safety bound for zero-time immediate transition chains.");
    _parentModel->getControls()->insert(bound);
    _addSimulationControl(bound);
    auto* lastError = new SimulationControlGeneric<std::string>(
        [this]() { return getLastRefinementError(); }, nullptr,
        Util::TypeOf<EFSMNetwork>(), getName(), "Last Refinement Error",
        "Read-only message of the last caught refinement fire/postfire exception.");
    _parentModel->getControls()->insert(lastError);
    _addSimulationControl(lastError);
}

EFSMNetwork::~EFSMNetwork() {
	_destroyOwnedTransitions();
	delete _states;
	_states = nullptr;
	delete _transitions;
	_transitions = nullptr;
	delete _sampler;
	_sampler = nullptr;
}

ModelDataDefinition* EFSMNetwork::NewInstance(Model* model, std::string name) {
	return new EFSMNetwork(model, name);
}

PluginInformation* EFSMNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<EFSMNetwork>(), &EFSMNetwork::LoadInstance, &EFSMNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDataDefinitionBaseTypeName(Util::TypeOf<DefaultNetwork>());
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
		// States remain owned by ModelDataManager; this is a non-owning
		// association used by orphan detection and model validation.
		_attachedDataInsert(stateAssociationKey(state), state);
		// The containing Network is also this state's hierarchical editing context.
		state->setModelLevel(static_cast<unsigned int>(getId()));
		if (_initialState == nullptr || state->isInitialNode()) {
			if (_initialState != nullptr && _initialState != state)
				_initialState->setInitialNode(false);
			_initialState = state;
			_currentState = state;
			state->setInitialNode(true);
		}
	}
}

void EFSMNetwork::removeState(FSMState* state) {
	if (state != nullptr) {
		_attachedDataRemove(stateAssociationKey(state));
	}
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
	(void)detachTransition(transition);
}

std::unique_ptr<EFSMTransition> EFSMNetwork::detachTransition(EFSMTransition* transition, unsigned int* ordinal) {
	if (transition == nullptr || _transitions == nullptr) {
		return {};
	}
	auto* entries = _transitions->list();
	auto it = std::find(entries->begin(), entries->end(), transition);
	if (it == entries->end()) {
		return {};
	}
	if (_lastTransition == transition) _lastTransition = nullptr;
	const auto position = static_cast<unsigned int>(std::distance(entries->begin(), it));
	if (ordinal != nullptr) {
		*ordinal = position;
	}
	entries->erase(it);
	if (transition->getSource() != nullptr) {
		transition->getSource()->removeTransition(transition);
	}
	return std::unique_ptr<EFSMTransition>(transition);
}

bool EFSMNetwork::restoreTransition(EFSMTransition* transition, unsigned int ordinal) {
	if (transition == nullptr || _transitions == nullptr
		|| _transitions->find(transition) != _transitions->list()->end()) {
		return false;
	}
	auto* source = dynamic_cast<FSMState*>(transition->getSource());
	auto* destination = dynamic_cast<FSMState*>(transition->getDestination());
	if (source == nullptr || destination == nullptr
		|| _states->find(source) == _states->list()->end()
		|| _states->find(destination) == _states->list()->end()) {
		return false;
	}
	auto* entries = _transitions->list();
	auto position = entries->begin();
	const auto boundedOrdinal = std::min<std::size_t>(ordinal, entries->size());
	std::advance(position, static_cast<std::ptrdiff_t>(boundedOrdinal));
	entries->insert(position, transition);
	source->addTransition(transition);
	return true;
}

List<EFSMTransition*>* EFSMNetwork::getTransitions() const {
	return _transitions;
}

void EFSMNetwork::setMaxImmediateTransitionsPerActivation(unsigned int maximum) {
    if (maximum == 0) throw std::invalid_argument("EFSM immediate transition budget must be positive");
    _maxImmediateTransitionsPerActivation = maximum;
}

unsigned int EFSMNetwork::getMaxImmediateTransitionsPerActivation() const {
    return _maxImmediateTransitionsPerActivation;
}

const std::string& EFSMNetwork::getLastRefinementError() const {
    return _lastRefinementError;
}

bool EFSMNetwork::postfireRefinement() {
    return _currentState == nullptr || !_currentState->isFinalNode();
}

void EFSMNetwork::setInitialState(FSMState* state) {
	if (state != nullptr)
		addState(state);
	_initialState = state;
	_currentState = state;
    _stateJustEntered = true;
	// The network pointer is authoritative: exactly the designated state
	// is initial, including after a GUI edit, an undo, or clearing the state.
	for (FSMState* candidate : *_states->list())
		if (candidate != nullptr) candidate->setInitialNode(candidate == state);
}

FSMState* EFSMNetwork::getInitialState() const {
	return _initialState;
}

FSMState* EFSMNetwork::getCurrentState() const {
	return _currentState;
}

EFSMTransition* EFSMNetwork::getLastTransition() const {
	return _lastTransition;
}

void EFSMNetwork::setCurrentState(FSMState* state) {
	_currentState = state;
    _stateJustEntered = true;
	if (state != nullptr) {
		addState(state);
	}
}

void EFSMNetwork::setConflictPolicy(ConflictPolicy policy) {
	_conflictPolicy = policy;
}

EFSMNetwork::ConflictPolicy EFSMNetwork::getConflictPolicy() const {
	return _conflictPolicy;
}

std::string EFSMNetwork::convertConflictPolicyToString(ConflictPolicy policy) {
	switch (policy) {
	case ConflictPolicy::MODEL_ERROR:
		return "MODEL_ERROR";
	case ConflictPolicy::NONDETERMINISTIC_CHOICE:
		return "NONDETERMINISTIC_CHOICE";
	case ConflictPolicy::DETERMINISTIC_PRIORITY:
		return "DETERMINISTIC_PRIORITY";
	}
	return "DETERMINISTIC_PRIORITY";
}

EFSMNetwork::ConflictPolicy EFSMNetwork::convertStringToConflictPolicy(const std::string& policy) {
	if (policy == "MODEL_ERROR") {
		return ConflictPolicy::MODEL_ERROR;
	}
	if (policy == "NONDETERMINISTIC_CHOICE") {
		return ConflictPolicy::NONDETERMINISTIC_CHOICE;
	}
	return ConflictPolicy::DETERMINISTIC_PRIORITY;
}

std::string EFSMNetwork::show() {
	return DefaultNetwork::show() +
	       ", states=" + std::to_string(_states->size()) +
	       ", transitions=" + std::to_string(_transitions->size()) +
	       ", initialState=\"" + (_initialState != nullptr ? _initialState->getName() : "") + "\"" +
	       ", currentState=\"" + (_currentState != nullptr ? _currentState->getName() : "") + "\"";
}

void EFSMNetwork::_destroyOwnedTransitions() {
	if (_transitions == nullptr) {
		return;
	}
	// Do not touch source-state transition lists here: during network/model
	// teardown those DefaultNode instances may already be destroyed.
	for (EFSMTransition* transition : *_transitions->list()) {
		delete transition;
	}
	_transitions->clear();
}

bool EFSMNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNetwork::_loadInstance(fields);
	if (res) {
		_states->clear();
		_destroyOwnedTransitions();
		_initialState = nullptr;
		_currentState = nullptr;
        _stateJustEntered = true;
        _lastRefinementError.clear();
        _initializedRefinements.clear();

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
			transition->setTriggerEvent(fields->loadField("transitionTriggerEvent" + suffix, std::string("")));
			transition->setProbabilityExpression(fields->loadField("transitionProbabilityExpression" + suffix, std::string("")));
			transition->setDefaultTransition(fields->loadField("transitionDefault" + suffix, 0u) != 0u);
			transition->setNondeterministic(fields->loadField("transitionNondeterministic" + suffix, 0u) != 0u);
			transition->setOutputActions(fields->loadField("transitionOutputActions" + suffix, std::string("")));
			transition->setSetActions(fields->loadField("transitionSetActions" + suffix, std::string("")));
            transition->setImmediate(fields->loadField("transitionImmediate" + suffix, 0u) != 0u);
            transition->setPreemptive(fields->loadField("transitionPreemptive" + suffix, 0u) != 0u);
            transition->setHistory(fields->loadField("transitionHistory" + suffix, 0u) != 0u);
            transition->setErrorTransition(fields->loadField("transitionError" + suffix, 0u) != 0u);
            transition->setTermination(fields->loadField("transitionTermination" + suffix, 0u) != 0u);
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
		// The serialized initial-state name is canonical; nested state flags
		// may otherwise leave more than one Initial Node marked after loading.
		for (FSMState* candidate : *_states->list())
			if (candidate != nullptr) candidate->setInitialNode(candidate == _initialState);
		_conflictPolicy = convertStringToConflictPolicy(fields->loadField("conflictPolicy", convertConflictPolicyToString(ConflictPolicy::DETERMINISTIC_PRIORITY)));
        _maxImmediateTransitionsPerActivation = fields->loadField("maxImmediateTransitionsPerActivation", 64u);
        if (_maxImmediateTransitionsPerActivation == 0) _maxImmediateTransitionsPerActivation = 64u;
	}
	return res;
}

void EFSMNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNetwork::_saveInstance(fields, saveDefaultValues);
	fields->saveField("conflictPolicy", convertConflictPolicyToString(_conflictPolicy), convertConflictPolicyToString(ConflictPolicy::DETERMINISTIC_PRIORITY), saveDefaultValues);
    fields->saveField("maxImmediateTransitionsPerActivation", _maxImmediateTransitionsPerActivation, 64u, saveDefaultValues);
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
		// The presence of this field distinguishes an explicitly unnamed
		// transition from a legacy record that omitted the field entirely.
		// The loader intentionally retains its T<ordinal> fallback for those
		// older records, while current saves preserve the exact empty name.
		fields->saveField("transitionName" + suffix, transition->getName());
		fields->saveField("transitionGuard" + suffix, transition->getGuardExpression(), std::string(""), saveDefaultValues);
		fields->saveField("transitionOutput" + suffix, transition->getOutputExpression(), std::string(""), saveDefaultValues);
		fields->saveField("transitionInputEvent" + suffix, transition->getInputEvent(), std::string(""), saveDefaultValues);
		fields->saveField("transitionPriority" + suffix, transition->getPriority(), 0u, saveDefaultValues);
		fields->saveField("transitionTriggerEvent" + suffix, transition->getTriggerEvent(), std::string(""), saveDefaultValues);
		fields->saveField("transitionProbabilityExpression" + suffix, transition->getProbabilityExpression(), std::string(""), saveDefaultValues);
		fields->saveField("transitionDefault" + suffix, transition->isDefaultTransition() ? 1u : 0u, 0u, saveDefaultValues);
		fields->saveField("transitionNondeterministic" + suffix, transition->isNondeterministic() ? 1u : 0u, 0u, saveDefaultValues);
		fields->saveField("transitionOutputActions" + suffix, transition->getOutputActions(), std::string(""), saveDefaultValues);
		fields->saveField("transitionSetActions" + suffix, transition->getSetActions(), std::string(""), saveDefaultValues);
        fields->saveField("transitionImmediate" + suffix, transition->isImmediate() ? 1u : 0u, 0u, saveDefaultValues);
        fields->saveField("transitionPreemptive" + suffix, transition->isPreemptive() ? 1u : 0u, 0u, saveDefaultValues);
        fields->saveField("transitionHistory" + suffix, transition->isHistory() ? 1u : 0u, 0u, saveDefaultValues);
        fields->saveField("transitionError" + suffix, transition->isErrorTransition() ? 1u : 0u, 0u, saveDefaultValues);
        fields->saveField("transitionTermination" + suffix, transition->isTermination() ? 1u : 0u, 0u, saveDefaultValues);
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
    // A child refinement may itself be an EFSM; detect recursive cycles
    // without traversing model-owned objects or touching simulation state.
    std::unordered_set<const DefaultNetwork*> visited, active;
    std::function<bool(const DefaultNetwork*)> cycle = [&](const DefaultNetwork* node) {
        if (node == nullptr) return false;
        if (active.contains(node)) return true;
        if (visited.contains(node)) return false;
        visited.insert(node);
        active.insert(node);
        if (auto* nested = dynamic_cast<const EFSMNetwork*>(node)) {
            for (FSMState* state : *nested->getStates()->list())
                if (state != nullptr && cycle(state->getRefinementNetwork())) return true;
        }
        active.erase(node);
        return false;
    };
    if (cycle(this)) {
        errorMessage += "EFSMNetwork \"" + getName() + "\" has a recursive refinement cycle. ";
        resultAll = false;
    }
    for (FSMState* state : *_states->list()) {
        if (state != nullptr && state->hasUnresolvedRefinementNetwork()) {
            errorMessage += "EFSM state \"" + state->getName()
                + "\" has unresolved Refinement Network " + state->getRefinementNetworkIdentity() + ". ";
            resultAll = false;
        }
    }
	for (EFSMTransition* transition : *_transitions->list()) {
        if (transition == nullptr) continue;
        auto* sourceState = dynamic_cast<FSMState*>(transition->getSource());
        if (transition->isErrorTransition() || transition->isTermination()) {
            if (sourceState == nullptr || sourceState->getRefinementNetwork() == nullptr) {
                errorMessage += "EFSM transition \"" + transition->getName()
                    + "\" requires a source state with a Refinement Network. ";
                resultAll = false;
            }
        }
        if (transition->isErrorTransition() &&
            (transition->isPreemptive() || transition->isTermination() || transition->isImmediate())) {
            errorMessage += "EFSM error transition \"" + transition->getName()
                + "\" cannot also be preemptive, immediate or termination. ";
            resultAll = false;
        }
        if (transition->isTermination() &&
            (transition->isPreemptive() || transition->isImmediate()
             || !transition->getOutputActions().empty()
             || !transition->getOutputExpression().empty())) {
            errorMessage += "EFSM termination transition \"" + transition->getName()
                + "\" is postfire-only: no preemption, immediate firing or output actions. ";
            resultAll = false;
        }
        auto* sourceEndpoint = dynamic_cast<FSMState*>(transition->getSource());
        auto* destinationEndpoint = dynamic_cast<FSMState*>(transition->getDestination());
        if (sourceEndpoint == nullptr || destinationEndpoint == nullptr) {
            errorMessage += "EFSMNetwork \"" + getName() + "\" has transition \"" + transition->getName()
                + "\" with a null or non-FSM endpoint. ";
            resultAll = false;
        } else if (_states->find(sourceEndpoint) == _states->list()->end()
                   || _states->find(destinationEndpoint) == _states->list()->end()) {
            errorMessage += "EFSM transition \"" + transition->getName()
                + "\" references a state outside its owning Network. ";
            resultAll = false;
        }
        for (const auto& [name, value] : {
            std::pair<std::string, std::string>{"Input Event", transition->getInputEvent()},
            std::pair<std::string, std::string>{"Trigger Event", transition->getTriggerEvent()}}) {
            if (!value.empty() && getInputPortIndex(value) < 0) {
                errorMessage += "EFSM transition \"" + transition->getName()
                    + "\" has unknown " + name + " input port \"" + value + "\". ";
                resultAll = false;
            }
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
        for (bool isOutput : {true, false}) {
            const std::string& actions = isOutput
                ? transition->getOutputActions() : transition->getSetActions();
            std::vector<TransitionAction> parsed;
            if (!parseTransitionActions(actions, &parsed)) {
                errorMessage += "EFSM transition \"" + transition->getName() + "\" has invalid "
                    + (isOutput ? "Output Actions" : "Set Actions")
                    + " (use destination=expression; ...). ";
                resultAll = false;
                continue;
            }
            for (const auto& action : parsed) {
                bool targetFound = false;
                if (isOutput) {
                    for (unsigned int port = 0; port < getNumOutputPorts(); ++port)
                        if (getOutputPortName(port) == action.destination) targetFound = true;
                } else if (_parentModel != nullptr && _parentModel->getDataManager() != nullptr) {
                    targetFound = dynamic_cast<Variable*>(_parentModel->getDataManager()
                        ->getDataDefinition(Util::TypeOf<Variable>(), action.destination)) != nullptr;
                }
                if (!targetFound) {
                    errorMessage += "EFSM transition \"" + transition->getName() + "\" has unknown "
                        + (isOutput ? "output port " : "Variable ")
                        + "\"" + action.destination + "\". ";
                    resultAll = false;
                }
                resultAll &= _parentModel->checkExpression(action.expression,
                    isOutput ? "EFSM Output Actions[" + transition->getName() + "]"
                             : "EFSM Set Actions[" + transition->getName() + "]",
                    errorMessage);
            }
        }
	}
	return resultAll;
}

void EFSMNetwork::_initBetweenReplications() {
	DefaultNetwork::_initBetweenReplications();
	_currentState = _resolveInitialState();
	_lastTransition = nullptr;
    _stateJustEntered = true;
    _initializedRefinements.clear();
    _lastRefinementError.clear();
	if (auto* defaultSampler = dynamic_cast<SamplerDefaultImpl1*>(_sampler); defaultSampler != nullptr) {
		defaultSampler->reset();
	}
}

NetworkActivationResult EFSMNetwork::_activate(const NetworkActivationFrame& frame) {
    _lastTransition = nullptr;
    _lastRefinementError.clear();
    NetworkActivationResult result(getNumOutputPorts());
    if (_currentState == nullptr) {
        _currentState = _resolveInitialState();
        _stateJustEntered = true;
    }
    if (_currentState == nullptr) return result;

    // Selection is a two-phase discrete-event analogue of FSMDirector:
    // preemptive guards -> fire state refinement -> non-preemptive guards
    // -> output actions -> refinement postfire -> set actions/state commit.
    // Default transitions are fallbacks in each phase. Termination occurs
    // only after postfire and loses to all normal/default transitions.
    bool selectionConflict = false;
    const auto inputEventIsPresent = [this, &frame](const std::string& eventName) {
        if (eventName.empty()) return true;
        const int port = getInputPortIndex(eventName);
        return port >= 0 && frame.isPresent(static_cast<unsigned int>(port));
    };

    const auto select = [this, &selectionConflict, &inputEventIsPresent](
        FSMState* state, bool preemptive, std::optional<bool> immediate,
        bool error, bool termination) -> EFSMTransition* {
        std::vector<EFSMTransition*> enabled;
        for (bool defaultFlag : {false, true}) {
            for (DefaultNodeTransition* base : *state->getTransitions()->list()) {
                auto* t = dynamic_cast<EFSMTransition*>(base);
                if (t == nullptr || _transitions->find(t) == _transitions->list()->end()
                    || t->isPreemptive() != preemptive
                    || (immediate.has_value() && t->isImmediate() != immediate.value())
                    || t->isErrorTransition() != error || t->isTermination() != termination
                    || t->isDefaultTransition() != defaultFlag)
                    continue;
                // In the current numeric NetworkActivationFrame, an event is the
                // presence of its named input port, not a nonzero port value.
                // Both configured event names are conjunctive requirements.
                // Short-circuit before evaluating stochastic guard expressions.
                if (!inputEventIsPresent(t->getInputEvent())
                    || !inputEventIsPresent(t->getTriggerEvent())
                    || !t->canFire(_parentModel, nullptr))
                    continue;

                // Probability Expression means a *per-transition Bernoulli
                // eligibility gate*, not a row-normalized weighted choice.
                // Existing conflict/default policies apply after this gate.
                if (!t->getProbabilityExpression().empty()) {
                    const double p = _parentModel->parseExpression(t->getProbabilityExpression());
                    if (!std::isfinite(p) || p < 0.0 || p > 1.0)
                        throw std::domain_error("EFSM transition \"" + t->getName()
                            + "\" Probability Expression must evaluate to a finite value in [0,1]");
                    if (p == 0.0) continue;
                    if (p < 1.0) {
                        if (_sampler == nullptr)
                            throw std::logic_error("EFSM probabilistic transition requires a sampler");
                        if (_sampler->random() >= p) continue;
                    }
                }
                enabled.push_back(t);
            }
            if (!enabled.empty()) break;
        }
        if (enabled.empty()) return nullptr;
        if (enabled.size() == 1) return enabled.front();
        const bool allNondeterministic = std::all_of(enabled.begin(), enabled.end(),
            [](const EFSMTransition* t) { return t->isNondeterministic(); });
        if (_conflictPolicy == ConflictPolicy::MODEL_ERROR && !allNondeterministic) {
            traceError("EFSMNetwork \"" + getName() + "\" has conflicting enabled transitions.",
                       TraceManager::Level::L3_errorRecover);
            selectionConflict = true;
            return nullptr;
        }
        if (_conflictPolicy == ConflictPolicy::NONDETERMINISTIC_CHOICE || allNondeterministic) {
            const double draw = _sampler != nullptr ? _sampler->random() : 0.0;
            const auto index = std::min(static_cast<std::size_t>(draw * enabled.size()),
                                        enabled.size() - 1);
            return enabled[index];
        }
        std::stable_sort(enabled.begin(), enabled.end(), [](const EFSMTransition* a, const EFSMTransition* b) {
            return a->getPriority() < b->getPriority();
        });
        return enabled.front();
    };

    const auto projectRefinementOutputs = [this, &result](
        DefaultNetwork* refinement, const NetworkActivationResult& outputs) {
        for (unsigned int local = 0; local < outputs.size(); ++local) {
            if (!outputs.isPresent(local)) continue;
            const int external = getOutputPortIndex(refinement->getOutputPortName(local));
            if (external >= 0)
                result.setPresent(static_cast<unsigned int>(external), outputs.getValue(local));
        }
    };

    const auto childFrame = [this, &frame](DefaultNetwork* refinement) {
        NetworkActivationFrame child(refinement->getNumInputPorts());
        for (unsigned int input = 0; input < child.size(); ++input) {
            const int parent = getInputPortIndex(refinement->getInputPortName(input));
            if (parent >= 0 && frame.isPresent(static_cast<unsigned int>(parent)))
                child.setPresent(input, frame.getValue(static_cast<unsigned int>(parent)));
        }
        return child;
    };

    // Commit validates every destination before changing any state or Variable.
    // Output actions are evaluated before refinement postfire; Set Actions are
    // evaluated and committed afterwards.
    std::exception_ptr postfireFailure;
    const auto commit = [this, &result, &postfireFailure](EFSMTransition* t, DefaultNetwork* firedRefinement,
                                        bool didFire, bool afterPostfire) -> bool {
        std::vector<TransitionAction> outputs, assignments;
        if (!parseTransitionActions(t->getOutputActions(), &outputs)
            || !parseTransitionActions(t->getSetActions(), &assignments)) {
            traceError("EFSMNetwork: malformed actions on transition \"" + t->getName() + "\".",
                       TraceManager::Level::L3_errorRecover);
            return false;
        }
        std::vector<std::pair<Variable*, std::string>> commitTargets;
        if (_parentModel == nullptr || _parentModel->getDataManager() == nullptr) return false;
        for (const auto& action : assignments) {
            auto* variable = dynamic_cast<Variable*>(_parentModel->getDataManager()
                ->getDataDefinition(Util::TypeOf<Variable>(), action.destination));
            if (variable == nullptr) {
                traceError("EFSMNetwork: unknown Variable \"" + action.destination + "\" in Set Actions.",
                           TraceManager::Level::L3_errorRecover);
                return false;
            }
            commitTargets.emplace_back(variable, action.expression);
        }
        for (const auto& action : outputs)
            if (getOutputPortIndex(action.destination) < 0) {
                traceError("EFSMNetwork: unknown output port \"" + action.destination + "\" in Output Actions.",
                           TraceManager::Level::L3_errorRecover);
                return false;
            }

        auto* source = dynamic_cast<FSMState*>(t->getSource());
        auto* destination = dynamic_cast<FSMState*>(t->getDestination());
        if (source == nullptr || destination == nullptr
            || _states->find(destination) == _states->list()->end()) return false;

        // A source exit action belongs to the *committed* transition. If a
        // refinement has fired, delay the exit action until its postfire
        // succeeds; otherwise an Error recovery could execute it twice.
        // Keep the historical order for preemptive/no-refinement transitions.
        const bool deferExitAction = didFire && firedRefinement != nullptr;
        if (!deferExitAction && !source->getExitActionExpression().empty())
            _parentModel->parseExpression(source->getExitActionExpression());
        // A postfire Error handler may still publish recovery outputs;
        // a Termination handler cannot publish outputs by model contract.
        if (!afterPostfire || t->isErrorTransition()) {
            if (!t->getOutputExpression().empty() && getNumOutputPorts() > 0)
                result.setPresent(0, _parentModel->parseExpression(t->getOutputExpression()));
            for (const auto& action : outputs) {
                const int port = getOutputPortIndex(action.destination);
                result.setPresent(static_cast<unsigned int>(port),
                                  _parentModel->parseExpression(action.expression));
            }
        }
        if (didFire && firedRefinement != nullptr) {
            try {
                (void)firedRefinement->postfireRefinement();
            } catch (const std::exception& error) {
                _lastRefinementError = error.what();
                postfireFailure = std::current_exception();
                return false;
            } catch (...) {
                _lastRefinementError = "unknown refinement postfire exception";
                postfireFailure = std::current_exception();
                return false;
            }
        }
        if (deferExitAction && !source->getExitActionExpression().empty())
            _parentModel->parseExpression(source->getExitActionExpression());
        std::vector<std::pair<Variable*, double>> values;
        for (const auto& [variable, expr] : commitTargets)
            values.emplace_back(variable, _parentModel->parseExpression(expr));
        for (const auto& [variable, value] : values) variable->setValue(value);

        DefaultNetwork* destinationRefinement = destination->getRefinementNetwork();
        if (destinationRefinement != nullptr
            && (!t->isHistory() || !_initializedRefinements.contains(destinationRefinement))) {
            destinationRefinement->initializeRefinement();
            _initializedRefinements.insert(destinationRefinement);
        }
        _lastTransition = t;
        _currentState = destination;
        _stateJustEntered = true;
        if (!destination->getEntryActionExpression().empty())
            _parentModel->parseExpression(destination->getEntryActionExpression());
        return true;
    };

    // One normal EFSM step per activation, followed by zero-time immediate
    // microsteps from newly entered states. Never execute another ordinary
    // transition merely because an immediate transition led to a new state.
    bool immediateOnly = false;
    unsigned int immediateCount = 0;
    while (_currentState != nullptr) {
        FSMState* source = _currentState;
        _stateJustEntered = false;
        if (immediateOnly) {
            // A newly entered state without an immediate outgoing relation
            // is stable for the remainder of this activation. Its refinement
            // must not fire a second ordinary step in the same dispatch.
            bool hasImmediate = false;
            for (DefaultNodeTransition* base : *source->getTransitions()->list()) {
                auto* t = dynamic_cast<EFSMTransition*>(base);
                if (t != nullptr && t->isImmediate()
                    && !t->isErrorTransition() && !t->isTermination()
                    && _transitions->find(t) != _transitions->list()->end()) {
                    hasImmediate = true; break;
                }
            }
            if (!hasImmediate) break;
        }
        DefaultNetwork* refinement = source->getRefinementNetwork();
        if (refinement != nullptr && !_initializedRefinements.contains(refinement)) {
            refinement->initializeRefinement();
            _initializedRefinements.insert(refinement);
        }

        // On every *external* activation, the current state's immediate and
        // ordinary transitions compete together. The immediate-only filter
        // applies exclusively to microsteps after a transition in this call;
        // an immediate transition remains eligible on later activations if
        // its guard was previously false.
        const std::optional<bool> immediateFilter =
            immediateOnly ? std::optional<bool>{true} : std::nullopt;
        EFSMTransition* chosen = select(source, true, immediateFilter, false, false);
        if (selectionConflict) break;
        bool hasNonpreemptiveImmediate = false;
        if (immediateOnly) {
            for (DefaultNodeTransition* base : *source->getTransitions()->list()) {
                auto* t = dynamic_cast<EFSMTransition*>(base);
                if (t != nullptr && t->isImmediate() && !t->isPreemptive()
                    && !t->isErrorTransition() && !t->isTermination()
                    && _transitions->find(t) != _transitions->list()->end()) {
                    hasNonpreemptiveImmediate = true; break;
                }
            }
            if (chosen == nullptr && !hasNonpreemptiveImmediate && immediateOnly) break;
        }
        bool didFire = false;
        bool terminated = false;
        bool refinementFailed = false;
        std::exception_ptr refinementException;

        if (chosen == nullptr) {
            if (refinement != nullptr) {
                try {
                    const auto outputs = refinement->fireRefinement(childFrame(refinement));
                    didFire = true;
                    projectRefinementOutputs(refinement, outputs);
                } catch (const std::exception& exception) {
                    refinementFailed = true;
                    _lastRefinementError = exception.what();
                    refinementException = std::current_exception();
                } catch (...) {
                    refinementFailed = true;
                    _lastRefinementError = "unknown refinement exception";
                    refinementException = std::current_exception();
                }
            }
            if (refinementFailed) {
                chosen = select(source, false, false, true, false);
                if (selectionConflict) break;
                if (chosen == nullptr) std::rethrow_exception(refinementException);
            } else {
                chosen = select(source, false, immediateFilter, false, false);
                if (selectionConflict) break;
            }
        }

        if (chosen == nullptr) {
            if (didFire && refinement != nullptr) {
                try {
                    terminated = !refinement->postfireRefinement();
                } catch (const std::exception& error) {
                    _lastRefinementError = error.what();
                    postfireFailure = std::current_exception();
                } catch (...) {
                    _lastRefinementError = "unknown refinement postfire exception";
                    postfireFailure = std::current_exception();
                }
            }
            if (postfireFailure != nullptr) {
                chosen = select(source, false, false, true, false);
                if (selectionConflict) break;
                if (chosen == nullptr) std::rethrow_exception(postfireFailure);
                result.reset(getNumOutputPorts()); // suppress incomplete refinement outputs
                postfireFailure = nullptr;
                if (!commit(chosen, nullptr, false, true)) break;
            } else {
                if (terminated && !immediateOnly)
                    chosen = select(source, false, false, false, true);
                if (selectionConflict || chosen == nullptr) break;
                // Termination is discovered in postfire: its output actions are
                // forbidden by _check(), because the output phase has elapsed.
                if (!chosen->isTermination() || !commit(chosen, nullptr, false, true)) break;
            }
        } else {
            if (!commit(chosen, refinement, didFire, false)) {
                if (postfireFailure == nullptr) break;
                chosen = select(source, false, false, true, false);
                if (selectionConflict) break;
                if (chosen == nullptr) std::rethrow_exception(postfireFailure);
                result.reset(getNumOutputPorts()); // retract uncommitted transition outputs
                postfireFailure = nullptr;
                if (!commit(chosen, nullptr, false, true)) break;
            }
        }

        if (chosen->isImmediate()) {
            if (++immediateCount > _maxImmediateTransitionsPerActivation)
                throw std::runtime_error("EFSM immediate transition cycle exceeded configured bound in \"" +
                                         getName() + "\"");
        }
        immediateOnly = true;
        // The next pass can take only an immediate transition. Do not fire
        // the destination refinement if no immediate transition is eligible.
        if (_currentState == nullptr) break;
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
