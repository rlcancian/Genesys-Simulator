#include "EFSMTransitionPropertiesUndoCommand.h"

#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

EFSMTransitionPropertiesUndoCommand::EFSMTransitionPropertiesUndoCommand(
        EFSMNetwork* network, EFSMTransition* transition, Properties updated, QUndoCommand* parent)
    : QUndoCommand(parent), _network(network), _transition(transition),
      _before(_read(transition)), _after(std::move(updated)) {
    setText(QObject::tr("Edit EFSM transition"));
    if (_network == nullptr || _network->getTransitions() == nullptr || _transition == nullptr
        || _network->getTransitions()->find(_transition) == _network->getTransitions()->list()->end()) {
        setObsolete(true);
    }
}

EFSMTransitionPropertiesUndoCommand::Properties
EFSMTransitionPropertiesUndoCommand::_read(const EFSMTransition* transition) {
    Properties properties;
    if (transition == nullptr) {
        return properties;
    }
    properties.name = transition->getName();
    properties.guardExpression = transition->getGuardExpression();
    properties.outputExpression = transition->getOutputExpression();
    properties.inputEvent = transition->getInputEvent();
    properties.priority = transition->getPriority();
    properties.triggerEvent = transition->getTriggerEvent();
    properties.probabilityExpression = transition->getProbabilityExpression();
    properties.outputActions = transition->getOutputActions();
    properties.setActions = transition->getSetActions();
    properties.nondeterministic = transition->isNondeterministic();
    properties.defaultTransition = transition->isDefaultTransition();
    properties.immediate = transition->isImmediate();
    properties.preemptive = transition->isPreemptive();
    properties.history = transition->isHistory();
    properties.errorTransition = transition->isErrorTransition();
    properties.termination = transition->isTermination();
    return properties;
}

bool EFSMTransitionPropertiesUndoCommand::_apply(const Properties& properties) {
    if (_network == nullptr || _network->getTransitions() == nullptr || _transition == nullptr
        || _network->getTransitions()->find(_transition) == _network->getTransitions()->list()->end()) {
        return false;
    }
    _transition->setName(properties.name);
    _transition->setGuardExpression(properties.guardExpression);
    _transition->setOutputExpression(properties.outputExpression);
    _transition->setInputEvent(properties.inputEvent);
    _transition->setPriority(properties.priority);
    _transition->setTriggerEvent(properties.triggerEvent);
    _transition->setProbabilityExpression(properties.probabilityExpression);
    _transition->setOutputActions(properties.outputActions);
    _transition->setSetActions(properties.setActions);
    _transition->setNondeterministic(properties.nondeterministic);
    _transition->setDefaultTransition(properties.defaultTransition);
    _transition->setImmediate(properties.immediate);
    _transition->setPreemptive(properties.preemptive);
    _transition->setHistory(properties.history);
    _transition->setErrorTransition(properties.errorTransition);
    _transition->setTermination(properties.termination);
    return true;
}

void EFSMTransitionPropertiesUndoCommand::undo() {
    if (!_apply(_before)) {
        setObsolete(true);
    }
}

void EFSMTransitionPropertiesUndoCommand::redo() {
    if (!_apply(_after)) {
        setObsolete(true);
    }
}
