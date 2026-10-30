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
