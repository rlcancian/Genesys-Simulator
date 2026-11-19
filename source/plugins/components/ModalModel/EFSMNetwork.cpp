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

#include <algorithm>
#include <cctype>
#include <memory>
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
            || !std::all_of(destination.begin() + 1, destination.end(), isTail))
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
	(void)frame;
	_lastTransition = nullptr;
	NetworkActivationResult result(getNumOutputPorts());
	if (_currentState == nullptr) {
		_currentState = _resolveInitialState();
	}
	if (_currentState == nullptr) {
		return result;
	}

	std::vector<EFSMTransition*> enabled;
    // Do not evaluate fallback guards when a normal transition is enabled:
    // guard expressions can be stochastic and must not consume RNG needlessly.
    for (bool defaults : {false, true}) {
        for (DefaultNodeTransition* transition : *_currentState->getTransitions()->list()) {
            auto* candidate = dynamic_cast<EFSMTransition*>(transition);
            if (candidate != nullptr && candidate->isDefaultTransition() == defaults
                && candidate->canFire(_parentModel, nullptr))
                enabled.push_back(candidate);
        }
        if (!enabled.empty()) break;
    }
	if (enabled.empty()) {
		return result;
	}
	EFSMTransition* chosen = nullptr;
    const bool allNondeterministic = enabled.size() > 1
        && std::all_of(enabled.begin(), enabled.end(), [](const EFSMTransition* transition) {
            return transition->isNondeterministic();
        });
	if (enabled.size() == 1) {
		chosen = enabled.front();
	} else if (_conflictPolicy == ConflictPolicy::MODEL_ERROR && !allNondeterministic) {
		traceError("EFSMNetwork \"" + getName() + "\" has multiple enabled transitions with MODEL_ERROR conflict policy.", TraceManager::Level::L3_errorRecover);
		return result;
	} else if (_conflictPolicy == ConflictPolicy::NONDETERMINISTIC_CHOICE || allNondeterministic) {
		const double sample = _sampler != nullptr ? _sampler->random() : 0.0;
		const auto index = std::min(static_cast<std::size_t>(sample * enabled.size()), enabled.size() - 1);
		chosen = enabled[index];
	} else {
		std::stable_sort(enabled.begin(), enabled.end(), [](EFSMTransition* a, EFSMTransition* b) {
			return a->getPriority() < b->getPriority();
		});
		chosen = enabled.front();
	}
	if (chosen == nullptr) {
		return result;
	}
    std::vector<TransitionAction> outputs;
    std::vector<TransitionAction> commits;
    if (!parseTransitionActions(chosen->getOutputActions(), &outputs)
        || !parseTransitionActions(chosen->getSetActions(), &commits)) {
        traceError("EFSMNetwork: invalid transition actions in \"" + chosen->getName() + "\".",
                   TraceManager::Level::L3_errorRecover);
        return result;
    }
    // Validate all destinations before causing a state transition or writes.
    std::vector<std::pair<Variable*, std::string>> commitTargets;
    for (const TransitionAction& action : commits) {
        auto* variable = dynamic_cast<Variable*>(_parentModel->getDataManager()
            ->getDataDefinition(Util::TypeOf<Variable>(), action.destination));
        if (variable == nullptr) {
            traceError("EFSMNetwork: unknown Variable \"" + action.destination
                + "\" in Set Actions.", TraceManager::Level::L3_errorRecover);
            return result;
        }
        commitTargets.emplace_back(variable, action.expression);
    }
    for (const TransitionAction& action : outputs) {
        bool found = false;
        for (unsigned int port = 0; port < getNumOutputPorts(); ++port)
            if (getOutputPortName(port) == action.destination) found = true;
        if (!found) {
            traceError("EFSMNetwork: unknown output port \"" + action.destination
                + "\" in Output Actions.", TraceManager::Level::L3_errorRecover);
            return result;
        }
    }

	_lastTransition = chosen;
	FSMState* source = dynamic_cast<FSMState*>(chosen->getSource());
	FSMState* destination = dynamic_cast<FSMState*>(chosen->getDestination());
	if (source != nullptr && source->getExitActionExpression() != "") {
		_parentModel->parseExpression(source->getExitActionExpression());
	}
	if (chosen->getOutputExpression() != "" && getNumOutputPorts() > 0) {
		result.setPresent(0, _parentModel->parseExpression(chosen->getOutputExpression()));
	}
    for (const TransitionAction& action : outputs)
        for (unsigned int port = 0; port < getNumOutputPorts(); ++port)
            if (getOutputPortName(port) == action.destination)
                result.setPresent(port, _parentModel->parseExpression(action.expression));

    // The activation is atomic in the current GenESyS network contract:
    // calculate all commit RHS values before changing any Variable.
    std::vector<std::pair<Variable*, double>> pendingCommits;
    pendingCommits.reserve(commitTargets.size());
    for (const auto& [variable, expression] : commitTargets)
        pendingCommits.emplace_back(variable, _parentModel->parseExpression(expression));
    for (const auto& [variable, value] : pendingCommits)
        variable->setValue(value);

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
