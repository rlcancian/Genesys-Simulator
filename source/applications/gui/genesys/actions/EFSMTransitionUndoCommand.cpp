#include "EFSMTransitionUndoCommand.h"

#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

#include <algorithm>
#include <iterator>
#include <utility>

EFSMTransitionUndoCommand::EFSMTransitionUndoCommand(EFSMNetwork* network,
		std::unique_ptr<EFSMTransition> transition, QUndoCommand* parent)
	: QUndoCommand(parent), _network(network), _transition(transition.get()),
	  _detachedTransition(std::move(transition)), _insertOnRedo(true) {
	setText(QObject::tr("Create EFSM transition"));
	if (_network != nullptr && _network->getTransitions() != nullptr) {
		_ordinal = _network->getTransitions()->size();
	} else {
		setObsolete(true);
	}
}

EFSMTransitionUndoCommand::EFSMTransitionUndoCommand(EFSMNetwork* network,
		EFSMTransition* transition, QUndoCommand* parent)
	: QUndoCommand(parent), _network(network), _transition(transition), _insertOnRedo(false) {
	setText(QObject::tr("Delete EFSM transition"));
	if (_network == nullptr || _network->getTransitions() == nullptr || _transition == nullptr) {
		setObsolete(true);
		return;
	}
	auto* transitions = _network->getTransitions()->list();
	auto it = std::find(transitions->begin(), transitions->end(), _transition);
	if (it == transitions->end()) {
		setObsolete(true);
		return;
	}
	_ordinal = static_cast<unsigned int>(std::distance(transitions->begin(), it));
}

bool EFSMTransitionUndoCommand::_attach() {
	if (_network == nullptr || _detachedTransition == nullptr
		|| !_network->restoreTransition(_detachedTransition.get(), _ordinal)) {
		return false;
	}
	_detachedTransition.release();
	return true;
}

bool EFSMTransitionUndoCommand::_detach() {
	if (_network == nullptr || _transition == nullptr) {
		return false;
	}
	_detachedTransition = _network->detachTransition(_transition, &_ordinal);
	return _detachedTransition != nullptr;
}

void EFSMTransitionUndoCommand::undo() {
	const bool succeeded = _insertOnRedo ? _detach() : _attach();
	if (!succeeded) {
		setObsolete(true);
	}
}

void EFSMTransitionUndoCommand::redo() {
	const bool succeeded = _insertOnRedo ? _attach() : _detach();
	if (!succeeded) {
		setObsolete(true);
	}
}
