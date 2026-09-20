#include "plugins/data/ModalModel/GraphNode.h"

#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GraphNode::GetPluginInformation;
}
#endif

GraphNode::GraphNode(Model* model, std::string name)
	: DefaultNode(model, Util::TypeOf<GraphNode>(), name) {
}

PluginInformation* GraphNode::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GraphNode>(), &GraphNode::LoadInstance, &GraphNode::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Mathematical graph vertex used by GraphNetwork; not a process-flow ModelComponent.");
	return info;
}

ModelDataDefinition* GraphNode::LoadInstance(Model* model, PersistenceRecord* fields) {
	// Prefer the canonical ModelDataManager instance when nested network fields and
	// top-level serialization both refer to the same named node.
	const std::string name = fields->loadField("name", std::string(""));
	if (model != nullptr && !name.empty()) {
		if (GraphNode* existing = dynamic_cast<GraphNode*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<GraphNode>(), name))) {
			try {
				existing->_loadInstance(fields);
			} catch (const std::exception& e) {
				existing->traceError("Failed to load GraphNode instance: " + std::string(e.what()));
			}
			return existing;
		}
	}
	GraphNode* node = new GraphNode(model);
	try {
		node->_loadInstance(fields);
	} catch (const std::exception& e) {
		node->traceError("Failed to load GraphNode instance: " + std::string(e.what()));
	}
	return node;
}

ModelDataDefinition* GraphNode::NewInstance(Model* model, std::string name) {
	return new GraphNode(model, name);
}
