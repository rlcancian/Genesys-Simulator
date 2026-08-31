#include "plugins/data/ModalModel/DirectedAcyclicGraphNetwork.h"

#include "plugins/data/ModalModel/GraphEdge.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DirectedAcyclicGraphNetwork::GetPluginInformation;
}
#endif

DirectedAcyclicGraphNetwork::DirectedAcyclicGraphNetwork(Model* model, std::string name)
	: DirectedGraphNetwork(model, name, Util::TypeOf<DirectedAcyclicGraphNetwork>()) {
}

ModelDataDefinition* DirectedAcyclicGraphNetwork::NewInstance(Model* model, std::string name) {
	return new DirectedAcyclicGraphNetwork(model, name);
}

PluginInformation* DirectedAcyclicGraphNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DirectedAcyclicGraphNetwork>(), &DirectedAcyclicGraphNetwork::LoadInstance, &DirectedAcyclicGraphNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Directed acyclic mathematical graph network with cycle validation and topological ordering.");
	return info;
}

ModelDataDefinition* DirectedAcyclicGraphNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	DirectedAcyclicGraphNetwork* graph = new DirectedAcyclicGraphNetwork(model);
	try {
		graph->_loadInstance(fields);
	} catch (const std::exception& e) {
		graph->traceError("Failed to load DirectedAcyclicGraphNetwork instance: " + std::string(e.what()));
	}
	return graph;
}

bool DirectedAcyclicGraphNetwork::addEdge(GraphEdge* edge) {
	if (!DirectedGraphNetwork::addEdge(edge)) {
		return false;
	}
	if (hasCycle()) {
		DirectedGraphNetwork::removeEdge(edge);
		return false;
	}
	return true;
}

bool DirectedAcyclicGraphNetwork::_check(std::string& errorMessage) {
	bool resultAll = DirectedGraphNetwork::_check(errorMessage);
	if (hasCycle()) {
		errorMessage += "DirectedAcyclicGraphNetwork \"" + getName() + "\" contains a directed cycle. ";
		resultAll = false;
	}
	return resultAll;
}
