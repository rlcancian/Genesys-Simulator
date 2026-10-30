#include "EFSMStatePropertiesUndoCommand.h"

#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <utility>

EFSMStatePropertiesUndoCommand::EFSMStatePropertiesUndoCommand(
        EFSMNetwork* network, FSMState* state, Properties updated, QUndoCommand* parent)
    : QUndoCommand(parent), _network(network), _state(state), _before(_read(state)),
      _after(std::move(updated)) {
    setText(QObject::tr("Edit EFSM state"));
    if (_network == nullptr || _network->getStates() == nullptr || _state == nullptr
        || _network->getStates()->find(_state) == _network->getStates()->list()->end()) {
        setObsolete(true);
    }
}

EFSMStatePropertiesUndoCommand::Properties EFSMStatePropertiesUndoCommand::_read(const FSMState* state) {
    Properties properties;
    if (state != nullptr) {
        properties.name = state->getName();
        properties.entryActionExpression = state->getEntryActionExpression();
        properties.exitActionExpression = state->getExitActionExpression();
        properties.finalNode = state->isFinalNode();
    }
    return properties;
}

bool EFSMStatePropertiesUndoCommand::_apply(const Properties& properties) {
    if (_network == nullptr || _network->getStates() == nullptr || _state == nullptr
        || _network->getStates()->find(_state) == _network->getStates()->list()->end()) {
        return false;
    }
    _state->setName(properties.name);
    _state->setEntryActionExpression(properties.entryActionExpression);
    _state->setExitActionExpression(properties.exitActionExpression);
    _state->setFinalNode(properties.finalNode);
    return true;
}

void EFSMStatePropertiesUndoCommand::undo() {
    if (!_apply(_before)) {
        setObsolete(true);
    }
}

void EFSMStatePropertiesUndoCommand::redo() {
    if (!_apply(_after)) {
        setObsolete(true);
    }
}
