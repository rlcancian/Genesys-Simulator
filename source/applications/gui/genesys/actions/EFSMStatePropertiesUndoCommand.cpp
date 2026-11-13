#include "EFSMStatePropertiesUndoCommand.h"

#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/TraceManager.h"
#include "kernel/util/Util.h"

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
        properties.label = state->getLabel();
        properties.entryActionExpression = state->getEntryActionExpression();
        properties.exitActionExpression = state->getExitActionExpression();
        properties.traceLevel = static_cast<std::uint32_t>(state->getTraceLevelSpecific());
        properties.reportStatistics = state->isReportStatistics();
        properties.finalNode = state->isFinalNode();
        properties.initialNode = state->isInitialNode();
    }
    return properties;
}

bool EFSMStatePropertiesUndoCommand::_apply(const Properties& properties) {
    if (_network == nullptr || _network->getStates() == nullptr || _state == nullptr
        || _network->getStates()->find(_state) == _network->getStates()->list()->end()) {
        return false;
    }
    const std::string normalizedName = Util::StrReplace(properties.name, " ", "_");
    Model* model = _state->getParentModel();
    ModelDataManager* dataManager = model != nullptr ? model->getDataManager() : nullptr;
    ModelDataDefinition* nameConflict = dataManager != nullptr
        ? dataManager->getDataDefinition(Util::TypeOf<FSMState>(), normalizedName) : nullptr;
    if (normalizedName.empty() || (nameConflict != nullptr && nameConflict != _state)) {
        return false;
    }
    _state->setName(properties.name);
    _state->setLabel(properties.label);
    _state->setEntryActionExpression(properties.entryActionExpression);
    _state->setExitActionExpression(properties.exitActionExpression);
    _state->setTraceLevelSpecific(static_cast<TraceManager::Level>(properties.traceLevel));
    _state->setReportStatistics(properties.reportStatistics);
    _state->setFinalNode(properties.finalNode);
    for (FSMState* state : *_network->getStates()->list()) {
        if (state != nullptr) state->setInitialNode(state == _state && properties.initialNode);
    }
    if (properties.initialNode) {
        _network->setInitialState(_state);
    } else if (_network->getInitialState() == _state) {
        _network->setInitialState(nullptr);
    }
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
