#include "EFSMStateMembershipUndoCommand.h"

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/util/Util.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <algorithm>
#include <iterator>

EFSMStateMembershipUndoCommand::EFSMStateMembershipUndoCommand(
        Model* model, EFSMNetwork* network, std::unique_ptr<FSMState> state, QUndoCommand* parent)
    : QUndoCommand(parent), _model(model), _network(network), _state(state.get()),
      _ownedState(std::move(state)), _insertionCommand(true) {
    setText(QObject::tr("Create EFSM state"));
    if (_model == nullptr || _model->getDataManager() == nullptr || _network == nullptr || _state == nullptr) {
        setObsolete(true);
        return;
    }
    _beforeInitial = _network->getInitialState();
    _beforeCurrent = _network->getCurrentState();
    _captureFlags();
}

EFSMStateMembershipUndoCommand::EFSMStateMembershipUndoCommand(
        Model* model, EFSMNetwork* network, FSMState* state, QUndoCommand* parent)
    : QUndoCommand(parent), _model(model), _network(network), _state(state) {
    setText(QObject::tr("Delete EFSM state"));
    if (_model == nullptr || _model->getDataManager() == nullptr || !_belongs(_state)) {
        setObsolete(true);
        return;
    }
    if (_network->getStates()->size() <= 1u) {
        setObsolete(true);
        return;
    }
    _beforeInitial = _network->getInitialState();
    _beforeCurrent = _network->getCurrentState();
    _captureFlags();
}

bool EFSMStateMembershipUndoCommand::_belongs(FSMState* state) const {
    return _network != nullptr && _network->getStates() != nullptr && state != nullptr
        && _network->getStates()->find(state) != _network->getStates()->list()->end();
}

void EFSMStateMembershipUndoCommand::_captureFlags() {
    if (_network == nullptr || _network->getStates() == nullptr) {
        return;
    }
    _initialFlags.clear();
    for (FSMState* state : *_network->getStates()->list()) {
        if (state != nullptr) {
            _initialFlags.emplace_back(state, state->isInitialNode());
        }
    }
}

void EFSMStateMembershipUndoCommand::_restoreFlags() {
    for (const auto& [state, initial] : _initialFlags) {
        if (_belongs(state)) {
            state->setInitialNode(initial);
        }
    }
}

bool EFSMStateMembershipUndoCommand::_insert() {
    if (_ownedState == nullptr || _network == nullptr || _model == nullptr
        || _model->getDataManager() == nullptr || _network->getStates() == nullptr) {
        return false;
    }
    _state = _ownedState.get();
    auto* manager = _model->getDataManager();
    const auto type = Util::TypeOf<FSMState>();
    ModelDataDefinition* registered = manager->getDataDefinition(type, _state->getId());
    if (registered != nullptr && registered != _state) {
        return false;
    }
    if (registered == nullptr && !manager->insert(_state)) {
        return false;
    }
    _network->addState(_state);
    if (!_belongs(_state)) {
        manager->remove(_state);
        return false;
    }
    _ownedState.release();
    return true;
}

bool EFSMStateMembershipUndoCommand::_remove() {
    if (!_belongs(_state) || _model == nullptr || _model->getDataManager() == nullptr) {
        return false;
    }
    if (_insertionCommand) {
        for (EFSMTransition* transition : *_network->getTransitions()->list()) {
            if (transition != nullptr && (transition->getSource() == _state || transition->getDestination() == _state)) {
                return false;
            }
        }
    } else {
        _detachedTransitions.clear();
        auto* transitions = _network->getTransitions()->list();
        std::vector<std::pair<EFSMTransition*, unsigned int>> incidentTransitions;
        for (std::size_t ordinal = 0; ordinal < transitions->size(); ++ordinal) {
            auto iterator = transitions->begin();
            std::advance(iterator, static_cast<std::ptrdiff_t>(ordinal));
            EFSMTransition* transition = *iterator;
            if (transition != nullptr && (transition->getSource() == _state || transition->getDestination() == _state)) {
                incidentTransitions.emplace_back(transition, static_cast<unsigned int>(ordinal));
            }
        }
        for (auto it = incidentTransitions.rbegin(); it != incidentTransitions.rend(); ++it) {
            DetachedTransition detached;
            detached.ordinal = it->second;
            detached.transition = _network->detachTransition(it->first);
            if (detached.transition == nullptr) {
                return false;
            }
            _detachedTransitions.push_back(std::move(detached));
        }
    }

    _network->removeState(_state);
    _state->setInitialNode(false);
    _model->getDataManager()->remove(_state);
    _ownedState.reset(_state);

    if (!_insertionCommand && _beforeInitial == _state) {
        FSMState* replacement = nullptr;
        if (!_network->getStates()->empty()) {
            replacement = _network->getStates()->front();
        }
        for (FSMState* remaining : *_network->getStates()->list()) {
            if (remaining != nullptr) {
                remaining->setInitialNode(remaining == replacement);
            }
        }
        _network->setInitialState(replacement);
        _network->setCurrentState(_beforeCurrent == _state ? replacement : _beforeCurrent);
    } else if (!_insertionCommand && _beforeCurrent == _state) {
        _network->setCurrentState(_network->getInitialState());
    }
    return true;
}

bool EFSMStateMembershipUndoCommand::_restoreRemovedState() {
    if (_ownedState == nullptr || _model == nullptr || _network == nullptr) {
        return false;
    }
    if ((_beforeInitial != nullptr && _beforeInitial != _state && !_belongs(_beforeInitial))
        || (_beforeCurrent != nullptr && _beforeCurrent != _state && !_belongs(_beforeCurrent))) {
        return false;
    }
    if (!_insert()) {
        return false;
    }
    if (_beforeInitial != nullptr) {
        _network->setInitialState(_beforeInitial);
    } else {
        _network->setInitialState(nullptr);
    }
    _network->setCurrentState(_beforeCurrent);
    _restoreFlags();

    std::sort(_detachedTransitions.begin(), _detachedTransitions.end(),
              [](const DetachedTransition& left, const DetachedTransition& right) {
                  return left.ordinal < right.ordinal;
              });
    for (DetachedTransition& detached : _detachedTransitions) {
        if (detached.transition == nullptr
            || !_network->restoreTransition(detached.transition.get(), detached.ordinal)) {
            return false;
        }
        detached.transition.release();
    }
    return true;
}

void EFSMStateMembershipUndoCommand::undo() {
    const bool success = _insertionCommand ? _remove() : _restoreRemovedState();
    if (!success) {
        setObsolete(true);
    }
}

void EFSMStateMembershipUndoCommand::redo() {
    const bool success = _insertionCommand ? _insert() : _remove();
    if (!success) {
        setObsolete(true);
    }
}
