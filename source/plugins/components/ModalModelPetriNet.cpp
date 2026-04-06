#include "ModalModelPetriNet.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelPetriNet::GetPluginInformation;
}
#endif

ModalModelPetriNet::ModalModelPetriNet(Model* model, std::string name) : ModalModelDefault(model, name) {
}

PluginInformation* ModalModelPetriNet::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelPetriNet>(), &ModalModelPetriNet::LoadInstance, &ModalModelPetriNet::NewInstance);
	info->setCategory("Network");
	info->setDescriptionHelp("Specialization of ModalModelDefault for colored Petri net style models.");
	return info;
}

ModelComponent* ModalModelPetriNet::LoadInstance(Model* model, PersistenceRecord *fields) {
	ModalModelPetriNet* component = new ModalModelPetriNet(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* ModalModelPetriNet::NewInstance(Model* model, std::string name) {
	return new ModalModelPetriNet(model, name);
}

bool ModalModelPetriNet::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= getNodes()->size() > 0;
	if (!resultAll) {
		errorMessage += "ModalModelPetriNet requires at least one place/transition node.";
	}
	return resultAll;
}
