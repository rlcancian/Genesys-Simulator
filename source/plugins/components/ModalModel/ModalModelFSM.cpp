#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelFSM::GetPluginInformation;
}
#endif

ModalModelFSM::ModalModelFSM(Model* model, std::string name) : ModalModelDefault(model, name) {
	_fsmInitialState = nullptr;
}

PluginInformation* ModalModelFSM::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelFSM>(), &ModalModelFSM::LoadInstance, &ModalModelFSM::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Specialization of ModalModelDefault for finite-state machine style models.");
	return info;
}

ModelComponent* ModalModelFSM::LoadInstance(Model* model, PersistenceRecord *fields) {
	ModalModelFSM* component = new ModalModelFSM(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* ModalModelFSM::NewInstance(Model* model, std::string name) {
	return new ModalModelFSM(model, name);
}

bool ModalModelFSM::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= ModalModelDefault::_check(errorMessage);
	if (getNodes()->size() == 0) {
		errorMessage += "ModalModelFSM requires at least one state node. ";
		resultAll = false;
	}

	bool hasInitialState = false;
	for (DefaultNode* node : *getNodes()->list()) {
		FSMState* state = dynamic_cast<FSMState*>(node);
		if (state == nullptr) {
			errorMessage += "ModalModelFSM node \"" + (node != nullptr ? node->getName() : "<null>") + "\" is not an FSMState. ";
			resultAll = false;
			continue;
		}
		if (state->isInitialNode()) {
			hasInitialState = true;
		}
	}
	if (!hasInitialState) {
		errorMessage += "ModalModelFSM requires at least one initial state. ";
		resultAll = false;
	}
	return resultAll;
}

void ModalModelFSM::_initBetweenReplications() {

}
