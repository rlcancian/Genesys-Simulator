#include "FSMState.h"

FSMState::FSMState(Model* model, std::string name) : DefaultNode(model, name) {
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
	info->setCategory("Network");
	info->setDescriptionHelp("FSM node/state with optional entry and exit action expressions.");
	return info;
}

ModelComponent* FSMState::LoadInstance(Model* model, PersistenceRecord *fields) {
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
