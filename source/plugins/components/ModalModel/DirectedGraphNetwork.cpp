#include "plugins/data/ModalModel/DirectedGraphNetwork.h"

#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/GraphNode.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DirectedGraphNetwork::GetPluginInformation;
}
#endif

DirectedGraphNetwork::DirectedGraphNetwork(Model* model, std::string name, std::string dataDefinitionTypename)
	: GraphNetwork(model, name, dataDefinitionTypename) {
}

ModelDataDefinition* DirectedGraphNetwork::NewInstance(Model* model, std::string name) {
	return new DirectedGraphNetwork(model, name);
}

PluginInformation* DirectedGraphNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DirectedGraphNetwork>(), &DirectedGraphNetwork::LoadInstance, &DirectedGraphNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Directed mathematical graph network with predecessor/successor and directed reachability semantics.");
	return info;
}

ModelDataDefinition* DirectedGraphNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	DirectedGraphNetwork* graph = new DirectedGraphNetwork(model);
	try {
		graph->_loadInstance(fields);
	} catch (const std::exception& e) {
		graph->traceError("Failed to load DirectedGraphNetwork instance: " + std::string(e.what()));
	}
	return graph;
}

bool DirectedGraphNetwork::isDirected() const {
	return true;
}

std::vector<GraphNode*> DirectedGraphNetwork::getPredecessors(GraphNode* node) const {
	std::vector<GraphNode*> result;
	for (GraphEdge* edge : getIncomingEdges(node)) {
		if (edge != nullptr && edge->getSource() != nullptr &&
		    std::find(result.begin(), result.end(), edge->getSource()) == result.end()) {
			result.push_back(edge->getSource());
		}
	}
	return result;
}

std::vector<GraphNode*> DirectedGraphNetwork::getSuccessors(GraphNode* node) const {
	std::vector<GraphNode*> result;
	for (GraphEdge* edge : getOutgoingEdges(node)) {
		if (edge != nullptr && edge->getDestination() != nullptr &&
		    std::find(result.begin(), result.end(), edge->getDestination()) == result.end()) {
			result.push_back(edge->getDestination());
		}
	}
	return result;
}

std::vector<GraphEdge*> DirectedGraphNetwork::getIncomingEdges(GraphNode* node) const {
	std::vector<GraphEdge*> result;
	for (GraphEdge* edge : getEdges()) {
		if (edge != nullptr && edge->getDestination() == node) {
			result.push_back(edge);
		}
	}
	return result;
}

std::vector<GraphEdge*> DirectedGraphNetwork::getOutgoingEdges(GraphNode* node) const {
	std::vector<GraphEdge*> result;
	for (GraphEdge* edge : getEdges()) {
		if (edge != nullptr && edge->getSource() == node) {
			result.push_back(edge);
		}
	}
	return result;
}

unsigned int DirectedGraphNetwork::inDegree(GraphNode* node) const {
	return static_cast<unsigned int>(getIncomingEdges(node).size());
}

unsigned int DirectedGraphNetwork::outDegree(GraphNode* node) const {
	return static_cast<unsigned int>(getOutgoingEdges(node).size());
}

std::vector<std::vector<GraphNode*>> DirectedGraphNetwork::stronglyConnectedComponents() const {
	std::vector<std::vector<GraphNode*>> components;
	std::map<GraphNode*, int> index;
	std::map<GraphNode*, int> lowLink;
	std::vector<GraphNode*> stack;
	std::set<GraphNode*> onStack;
	int nextIndex = 0;

	std::function<void(GraphNode*)> strongConnect = [&](GraphNode* node) {
		index[node] = nextIndex;
		lowLink[node] = nextIndex;
		nextIndex++;
		stack.push_back(node);
		onStack.insert(node);

		for (GraphNode* successor : getSuccessors(node)) {
			if (index.find(successor) == index.end()) {
				strongConnect(successor);
				lowLink[node] = std::min(lowLink[node], lowLink[successor]);
			} else if (onStack.find(successor) != onStack.end()) {
				lowLink[node] = std::min(lowLink[node], index[successor]);
			}
		}

		if (lowLink[node] == index[node]) {
			std::vector<GraphNode*> component;
			GraphNode* current = nullptr;
			do {
				current = stack.back();
				stack.pop_back();
				onStack.erase(current);
				component.push_back(current);
			} while (current != node);
			components.push_back(component);
		}
	};

	for (GraphNode* node : getNodes()) {
		if (index.find(node) == index.end()) {
			strongConnect(node);
		}
	}
	return components;
}

bool DirectedGraphNetwork::hasCycle() const {
	std::map<GraphNode*, int> color;
	std::function<bool(GraphNode*)> visit = [&](GraphNode* node) {
		color[node] = 1;
		for (GraphNode* successor : getSuccessors(node)) {
			if (color[successor] == 1) {
				return true;
			}
			if (color[successor] == 0 && visit(successor)) {
				return true;
			}
		}
		color[node] = 2;
		return false;
	};

	for (GraphNode* node : getNodes()) {
		if (color[node] == 0 && visit(node)) {
			return true;
		}
	}
	return false;
}

GraphTopologicalOrderResult DirectedGraphNetwork::topologicalOrder() const {
	GraphTopologicalOrderResult result;
	std::map<GraphNode*, unsigned int> indegree;
	for (GraphNode* node : getNodes()) {
		indegree[node] = 0;
	}
	for (GraphEdge* edge : getEdges()) {
		if (edge != nullptr && edge->getDestination() != nullptr) {
			indegree[edge->getDestination()]++;
		}
	}

	std::vector<GraphNode*> ready;
	for (GraphNode* node : getNodes()) {
		if (indegree[node] == 0) {
			ready.push_back(node);
		}
	}

	while (!ready.empty()) {
		GraphNode* node = ready.front();
		ready.erase(ready.begin());
		result.order.push_back(node);
		for (GraphEdge* edge : getOutgoingEdges(node)) {
			GraphNode* successor = edge->getDestination();
			if (successor == nullptr) {
				continue;
			}
			indegree[successor]--;
			if (indegree[successor] == 0) {
				ready.push_back(successor);
			}
		}
	}

	result.acyclic = result.order.size() == getNodes().size();
	if (!result.acyclic) {
		result.order.clear();
		result.errorMessage = "Directed graph contains a cycle; no topological order exists.";
	}
	return result;
}

bool DirectedGraphNetwork::_edgeConnects(GraphEdge* edge, GraphNode* source, GraphNode* destination) const {
	return edge != nullptr && edge->getSource() == source && edge->getDestination() == destination;
}

GraphNode* DirectedGraphNetwork::_oppositeNode(GraphEdge* edge, GraphNode* node) const {
	if (edge == nullptr || node == nullptr || edge->getSource() != node) {
		return nullptr;
	}
	return edge->getDestination();
}
