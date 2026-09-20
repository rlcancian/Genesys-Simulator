#include "plugins/data/ModalModel/CPNTransition.h"

#include "kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CPNTransition::GetPluginInformation;
}
#endif

CPNTransition::CPNTransition(Model* model, std::string name)
	: DefaultNode(model, Util::TypeOf<CPNTransition>(), name) {
}

void CPNTransition::setGuardExpression(std::string guardExpression) {
	_guardExpression = guardExpression;
}

std::string CPNTransition::getGuardExpression() const {
	return _guardExpression;
}

void CPNTransition::setPriority(unsigned int priority) {
	_priority = priority;
}

unsigned int CPNTransition::getPriority() const {
	return _priority;
}

std::string CPNTransition::show() {
	return DefaultNode::show() +
	       ", guard=\"" + _guardExpression + "\"" +
	       ", priority=" + std::to_string(_priority);
}

PluginInformation* CPNTransition::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CPNTransition>(), &CPNTransition::LoadInstance, &CPNTransition::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Transition node used by ColoredPetriNetNetwork bipartite CPN topology.");
	return info;
}

ModelDataDefinition* CPNTransition::LoadInstance(Model* model, PersistenceRecord* fields) {
	// Prefer the canonical ModelDataManager instance when nested network fields and
	// top-level serialization both refer to the same named transition.
	const std::string name = fields->loadField("name", std::string(""));
	if (model != nullptr && !name.empty()) {
		if (CPNTransition* existing = dynamic_cast<CPNTransition*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<CPNTransition>(), name))) {
			// Do not re-apply fields: a later top-level load must not wipe
			// associations already resolved by a nested network load.
			return existing;
		}
	}
	CPNTransition* transition = new CPNTransition(model);
	try {
		transition->_loadInstance(fields);
	} catch (const std::exception& e) {
		transition->traceError("Failed to load CPNTransition instance: " + std::string(e.what()));
	}
	return transition;
}

ModelDataDefinition* CPNTransition::NewInstance(Model* model, std::string name) {
	return new CPNTransition(model, name);
}

bool CPNTransition::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNode::_loadInstance(fields);
	if (res) {
		_guardExpression = fields->loadField("guardExpression", std::string(""));
		_priority = fields->loadField("priority", 0u);
	}
	return res;
}

void CPNTransition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNode::_saveInstance(fields, saveDefaultValues);
	fields->saveField("guardExpression", _guardExpression, std::string(""), saveDefaultValues);
	fields->saveField("priority", _priority, 0u, saveDefaultValues);
}

bool CPNTransition::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_guardExpression != "" && !_parentModel->checkExpression(_guardExpression, "CPN transition guard[" + getName() + "]", errorMessage)) {
		resultAll = false;
	}
	return resultAll;
}
