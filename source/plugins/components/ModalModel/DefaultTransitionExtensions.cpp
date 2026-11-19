#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../kernel/simulator/model/Model.h"
#include <utility>
#include <cmath>
#include <stdexcept>

EFSMTransition::EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	setTransitionKind(TransitionKind::DETERMINISTIC);
}

void EFSMTransition::setTriggerEvent(std::string triggerEvent) {
	_triggerEvent = triggerEvent;
}

std::string EFSMTransition::getTriggerEvent() const {
	return _triggerEvent;
}

void EFSMTransition::setProbabilityExpression(std::string probabilityExpression) {
	_probabilityExpression = probabilityExpression;
}

std::string EFSMTransition::getProbabilityExpression() const {
	return _probabilityExpression;
}

void EFSMTransition::setOutputActions(std::string actions) {
    _outputActions = std::move(actions);
}

std::string EFSMTransition::getOutputActions() const {
    return _outputActions;
}

void EFSMTransition::setSetActions(std::string actions) {
    _setActions = std::move(actions);
}

std::string EFSMTransition::getSetActions() const {
    return _setActions;
}

void EFSMTransition::setNondeterministic(bool enabled) {
    _nondeterministic = enabled;
}

bool EFSMTransition::isNondeterministic() const {
    return _nondeterministic;
}

void EFSMTransition::setImmediate(bool enabled) { _immediate = enabled; }
bool EFSMTransition::isImmediate() const { return _immediate; }
void EFSMTransition::setPreemptive(bool enabled) { _preemptive = enabled; }
bool EFSMTransition::isPreemptive() const { return _preemptive; }
void EFSMTransition::setHistory(bool enabled) { _history = enabled; }
bool EFSMTransition::isHistory() const { return _history; }
void EFSMTransition::setErrorTransition(bool enabled) { _errorTransition = enabled; }
bool EFSMTransition::isErrorTransition() const { return _errorTransition; }
void EFSMTransition::setTermination(bool enabled) { _termination = enabled; }
bool EFSMTransition::isTermination() const { return _termination; }

void EFSMTransition::setDefaultTransition(bool enabled) {
    _defaultTransition = enabled;
}

bool EFSMTransition::isDefaultTransition() const {
    return _defaultTransition;
}

bool EFSMTransition::canFire(Model* model, Entity* entity) const {
    // Guard-only predicate. The parent EFSMNetwork owns both the short-lived
    // input-event frame and its seeded sampler, so it alone may apply event
    // presence and optional Bernoulli probability after a successful guard.
    // An unbound transition cannot invent an activation frame or RNG source.
    (void)entity;
    if (model == nullptr) return false;
    if (getGuardExpression().empty()) return true;
    const double value = model->parseExpression(getGuardExpression());
    if (!std::isfinite(value))
        throw std::domain_error("EFSM transition guard evaluates to a non-finite value: " + getName());
    return value != 0.0;
}

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}
