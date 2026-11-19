#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../kernel/simulator/model/Model.h"
#include <utility>

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
	bool parentCanFire = DefaultNodeTransition::canFire(model, entity);
	if (!parentCanFire) {
		return false;
	}
	if (_probabilityExpression != "") {
		double p = model->parseExpression(_probabilityExpression);
		return p > 0.0;
	}
	return true;
}

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}
