#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelFSM::GetPluginInformation;
}
#endif

ModalModelFSM::ModalModelFSM(Model* model, std::string name) : ModalModelDefault(model, name) {
}

PluginInformation* ModalModelFSM::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelFSM>(), &ModalModelFSM::LoadInstance, &ModalModelFSM::NewInstance);
	info->setCategory("Network");
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
	resultAll &= getNodes()->size() > 0;
	if (!resultAll) {
		errorMessage += "ModalModelFSM requires at least one state node.";
	}
	return resultAll;
}
