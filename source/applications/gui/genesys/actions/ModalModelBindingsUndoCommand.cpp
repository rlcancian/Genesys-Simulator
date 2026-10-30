#include "ModalModelBindingsUndoCommand.h"

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"

#include <utility>

ModalModelBindingsUndoCommand::ModalModelBindingsUndoCommand(
        ModalModelDefault* adapter, DefaultNetwork* network, Bindings updated, QUndoCommand* parent)
    : QUndoCommand(parent), _adapter(adapter), _network(network), _after(std::move(updated)) {
    setText(QObject::tr("Edit ModalModel Network bindings"));
    if (_adapter == nullptr || _network == nullptr || _adapter->getNetwork() != _network) {
        setObsolete(true);
        return;
    }
    _before.inputs.reserve(_network->getNumInputPorts());
    _before.outputs.reserve(_network->getNumOutputPorts());
    for (unsigned int port = 0; port < _network->getNumInputPorts(); ++port) {
        _before.inputs.push_back(_adapter->getInputBinding(port));
    }
    for (unsigned int port = 0; port < _network->getNumOutputPorts(); ++port) {
        _before.outputs.push_back(_adapter->getOutputBinding(port));
    }
}

bool ModalModelBindingsUndoCommand::_apply(const Bindings& bindings) {
    if (_adapter == nullptr || _network == nullptr || _adapter->getNetwork() != _network
        || bindings.inputs.size() != _network->getNumInputPorts()
        || bindings.outputs.size() != _network->getNumOutputPorts()) {
        return false;
    }
    for (unsigned int port = 0; port < bindings.inputs.size(); ++port) {
        _adapter->setInputBinding(port, bindings.inputs[port]);
    }
    for (unsigned int port = 0; port < bindings.outputs.size(); ++port) {
        _adapter->setOutputBinding(port, bindings.outputs[port]);
    }
    return true;
}

void ModalModelBindingsUndoCommand::undo() {
    if (!_apply(_before)) setObsolete(true);
}

void ModalModelBindingsUndoCommand::redo() {
    if (!_apply(_after)) setObsolete(true);
}
