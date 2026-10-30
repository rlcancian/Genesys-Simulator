#include "EFSMInitialStateUndoCommand.h"

#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

EFSMInitialStateUndoCommand::EFSMInitialStateUndoCommand(EFSMNetwork* network, FSMState* state,
                                                         QUndoCommand* parent)
    : QUndoCommand(parent), _network(network), _before(network != nullptr ? network->getInitialState() : nullptr),
      _after(state) {
    setText(QObject::tr("Set EFSM initial state"));
    if (!_stateBelongsToNetwork(_after)) {
        setObsolete(true);
        return;
    }
    for (FSMState* existing : *_network->getStates()->list()) {
        if (existing != nullptr) {
            _beforeFlags.emplace_back(existing, existing->isInitialNode());
        }
    }
}

bool EFSMInitialStateUndoCommand::_stateBelongsToNetwork(FSMState* state) const {
    return _network != nullptr && _network->getStates() != nullptr && state != nullptr
        && _network->getStates()->find(state) != _network->getStates()->list()->end();
}

bool EFSMInitialStateUndoCommand::_select(FSMState* state) {
    if (!_stateBelongsToNetwork(state)) {
        return false;
    }
    for (FSMState* existing : *_network->getStates()->list()) {
        if (existing != nullptr) {
            existing->setInitialNode(existing == state);
        }
    }
    _network->setInitialState(state);
    return true;
}

void EFSMInitialStateUndoCommand::undo() {
    if (_network == nullptr || (_before != nullptr && !_stateBelongsToNetwork(_before))) {
        setObsolete(true);
        return;
    }
    if (_before != nullptr) {
        _network->setInitialState(_before);
    } else {
        _network->setInitialState(nullptr);
    }
    for (const auto& [state, wasInitial] : _beforeFlags) {
        if (_stateBelongsToNetwork(state)) {
            state->setInitialNode(wasInitial);
        }
    }
}

void EFSMInitialStateUndoCommand::redo() {
    if (!_select(_after)) {
        setObsolete(true);
    }
}
