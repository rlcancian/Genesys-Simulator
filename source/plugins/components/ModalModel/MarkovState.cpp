#include "plugins/data/ModalModel/MarkovState.h"

#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MarkovState::GetPluginInformation;
}
#endif

MarkovState::MarkovState(Model* model, std::string name)
	: DefaultNode(model, Util::TypeOf<MarkovState>(), name) {
}

PluginInformation* MarkovState::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MarkovState>(), &MarkovState::LoadInstance, &MarkovState::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Finite state used by MarkovChainNetwork DTMC models; not a process-flow ModelComponent.");
	return info;
}

ModelDataDefinition* MarkovState::LoadInstance(Model* model, PersistenceRecord* fields) {
	// Prefer the canonical ModelDataManager instance when nested network fields and
	// top-level serialization both refer to the same named state.
	const std::string name = fields->loadField("name", std::string(""));
	if (model != nullptr && !name.empty()) {
		if (MarkovState* existing = dynamic_cast<MarkovState*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<MarkovState>(), name))) {
			// Do not re-apply fields: a later top-level load must not wipe
			// associations already resolved by a nested network load.
			return existing;
		}
	}
	MarkovState* state = new MarkovState(model);
	try {
		state->_loadInstance(fields);
	} catch (const std::exception& e) {
		state->traceError("Failed to load MarkovState instance: " + std::string(e.what()));
	}
	return state;
}

ModelDataDefinition* MarkovState::NewInstance(Model* model, std::string name) {
	return new MarkovState(model, name);
}
