#include "plugins/components/ModalModel/FSMState.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/model/Model.h"

FSMState::FSMState(Model* model, std::string name) : DefaultNode(model, Util::TypeOf<FSMState>(), name) {
	const std::string classname = Util::TypeOf<FSMState>();
	auto* entryAction = new SimulationControlGeneric<std::string>(
		[this]() { return getEntryActionExpression(); },
		[this](const std::string& value) { setEntryActionExpression(value); },
		classname, getName(), "Entry Action Expression", "Expression evaluated when entering this state.");
	auto* exitAction = new SimulationControlGeneric<std::string>(
		[this]() { return getExitActionExpression(); },
		[this](const std::string& value) { setExitActionExpression(value); },
		classname, getName(), "Exit Action Expression", "Expression evaluated when leaving this state.");
	auto* finalNode = new SimulationControlBool(
		[this]() { return isFinalNode(); },
		[this](bool value) { setFinalNode(value); },
		classname, getName(), "Final Node", "Whether this state is a final state.");
	auto* initialNode = new SimulationControlBool(
		[this]() { return isInitialNode(); },
		[this](bool value) { setInitialNode(value); },
		classname, getName(), "Initial Node", "Whether this state is the initial state of its Network.");
	_parentModel->getControls()->insert(entryAction);
	_addSimulationControl(entryAction);
	_parentModel->getControls()->insert(exitAction);
	_addSimulationControl(exitAction);
	_parentModel->getControls()->insert(finalNode);
	_addSimulationControl(finalNode);
	_parentModel->getControls()->insert(initialNode);
	_addSimulationControl(initialNode);
}

void FSMState::setEntryActionExpression(std::string expression) {
	_entryActionExpression = expression;
}

std::string FSMState::getEntryActionExpression() const {
	return _entryActionExpression;
}

void FSMState::setExitActionExpression(std::string expression) {
	_exitActionExpression = expression;
}

std::string FSMState::getExitActionExpression() const {
	return _exitActionExpression;
}

PluginInformation* FSMState::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<FSMState>(), &FSMState::LoadInstance, &FSMState::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("FSM node/state with optional entry and exit action expressions.");
	info->setReceiveTransfer(true); // FSM nodes do not need to be connected from a source to a sink
	info->setSendTransfer(true); // FSM nodes do not need to be connected in a process flow
	return info;
}

ModelDataDefinition* FSMState::LoadInstance(Model* model, PersistenceRecord *fields) {
	// Prefer the canonical ModelDataManager instance when nested network fields and
	// top-level serialization both refer to the same named state.
	const std::string name = fields->loadField("name", std::string(""));
	if (model != nullptr && !name.empty()) {
		if (FSMState* existing = dynamic_cast<FSMState*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), name))) {
			// Do not re-apply fields: a later top-level load must not wipe
			// associations already resolved by a nested network load.
			return existing;
		}
	}
	FSMState* component = new FSMState(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* FSMState::NewInstance(Model* model, std::string name) {
	return new FSMState(model, name);
}

bool FSMState::_loadInstance(PersistenceRecord *fields) {
	bool res = DefaultNode::_loadInstance(fields);
	if (res) {
		_entryActionExpression = fields->loadField("entryActionExpression", DEFAULT.entryActionExpression);
		_exitActionExpression = fields->loadField("exitActionExpression", DEFAULT.exitActionExpression);
	}
	return res;
}

void FSMState::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	DefaultNode::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entryActionExpression", _entryActionExpression, DEFAULT.entryActionExpression, saveDefaultValues);
	fields->saveField("exitActionExpression", _exitActionExpression, DEFAULT.exitActionExpression, saveDefaultValues);
}
