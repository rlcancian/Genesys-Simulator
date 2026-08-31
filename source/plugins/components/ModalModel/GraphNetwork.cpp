#include "plugins/data/ModalModel/GraphNetwork.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "plugins/data/ModalModel/GraphEdge.h"
#include "plugins/data/ModalModel/GraphNode.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <set>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GraphNetwork::GetPluginInformation;
}
#endif

bool GraphTraversalResult::visited(GraphNode* node) const {
	return std::any_of(visitOrder.begin(), visitOrder.end(), [node](GraphNode* current) {
		return current == node;
	});
}

GraphNode* GraphTraversalResult::predecessorOf(GraphNode* node) const {
	for (const GraphSearchTreeEntry& entry : tree) {
		if (entry.node == node) {
			return entry.predecessor;
		}
	}
	return nullptr;
}

GraphEdge* GraphTraversalResult::edgeTo(GraphNode* node) const {
	for (const GraphSearchTreeEntry& entry : tree) {
		if (entry.node == node) {
			return entry.edge;
		}
	}
	return nullptr;
}

unsigned int GraphTraversalResult::distanceTo(GraphNode* node) const {
	for (const GraphSearchTreeEntry& entry : tree) {
		if (entry.node == node) {
			return entry.distance;
		}
	}
	return std::numeric_limits<unsigned int>::max();
}

GraphNetwork::GraphNetwork(Model* model, std::string name, std::string dataDefinitionTypename)
	: DefaultNetwork(model, name, dataDefinitionTypename) {
}

ModelDataDefinition* GraphNetwork::NewInstance(Model* model, std::string name) {
	return new GraphNetwork(model, name);
}

PluginInformation* GraphNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GraphNetwork>(), &GraphNetwork::LoadInstance, &GraphNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Undirected mathematical graph network with topology and graph algorithms; not a process Connection network.");
	return info;
}

ModelDataDefinition* GraphNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	GraphNetwork* graph = new GraphNetwork(model);
	try {
		graph->_loadInstance(fields);
	} catch (const std::exception& e) {
		graph->traceError("Failed to load GraphNetwork instance: " + std::string(e.what()));
	}
	return graph;
}

bool GraphNetwork::isDirected() const {
	return false;
}

bool GraphNetwork::addNode(GraphNode* node) {
	if (node == nullptr || containsNode(node)) {
		return false;
	}
	_nodes.push_back(node);
	return true;
}

bool GraphNetwork::removeNode(GraphNode* node) {
	auto it = std::find(_nodes.begin(), _nodes.end(), node);
	if (it == _nodes.end()) {
		return false;
	}
	_removeIncidentEdges(node);
	_nodes.erase(it);
	return true;
}

bool GraphNetwork::containsNode(GraphNode* node) const {
	return std::find(_nodes.begin(), _nodes.end(), node) != _nodes.end();
}

GraphNode* GraphNetwork::findNodeByName(const std::string& name) const {
	for (GraphNode* node : _nodes) {
		if (node != nullptr && node->getName() == name) {
			return node;
		}
	}
	return nullptr;
}

std::vector<GraphNode*> GraphNetwork::getNodes() const {
	return _nodes;
}

bool GraphNetwork::addEdge(GraphEdge* edge) {
	if (edge == nullptr || containsEdge(edge)) {
		return false;
	}
	if (edge->getSource() == nullptr || edge->getDestination() == nullptr) {
		return false;
	}
	if (!containsNode(edge->getSource()) || !containsNode(edge->getDestination())) {
		return false;
	}
	edge->setDirected(isDirected());
	_edges.push_back(edge);
	return true;
}

bool GraphNetwork::removeEdge(GraphEdge* edge) {
	auto it = std::find(_edges.begin(), _edges.end(), edge);
	if (it == _edges.end()) {
		return false;
	}
	_edges.erase(it);
	return true;
}

bool GraphNetwork::containsEdge(GraphEdge* edge) const {
	return std::find(_edges.begin(), _edges.end(), edge) != _edges.end();
}

GraphEdge* GraphNetwork::findEdgeByName(const std::string& name) const {
	for (GraphEdge* edge : _edges) {
		if (edge != nullptr && edge->getName() == name) {
			return edge;
		}
	}
	return nullptr;
}

std::vector<GraphEdge*> GraphNetwork::getEdges() const {
	return _edges;
}

bool GraphNetwork::hasEdge(GraphNode* source, GraphNode* destination) const {
	return !getEdgesBetween(source, destination).empty();
}

std::vector<GraphEdge*> GraphNetwork::getEdgesBetween(GraphNode* source, GraphNode* destination) const {
	std::vector<GraphEdge*> result;
	for (GraphEdge* edge : _edges) {
		if (_edgeConnects(edge, source, destination)) {
			result.push_back(edge);
		}
	}
	return result;
}

std::vector<GraphNode*> GraphNetwork::getNeighbors(GraphNode* node) const {
	std::vector<GraphNode*> result;
	for (GraphEdge* edge : _edges) {
		GraphNode* opposite = _oppositeNode(edge, node);
		if (opposite != nullptr && std::find(result.begin(), result.end(), opposite) == result.end()) {
			result.push_back(opposite);
		}
	}
	return result;
}

std::vector<GraphEdge*> GraphNetwork::getIncidentEdges(GraphNode* node) const {
	std::vector<GraphEdge*> result;
	for (GraphEdge* edge : _edges) {
		if (edge != nullptr && (edge->getSource() == node || edge->getDestination() == node)) {
			result.push_back(edge);
		}
	}
	return result;
}

unsigned int GraphNetwork::degree(GraphNode* node) const {
	unsigned int result = 0;
	for (GraphEdge* edge : _edges) {
		if (edge == nullptr) {
			continue;
		}
		if (edge->getSource() == node && edge->getDestination() == node) {
			result += 2;
		} else if (edge->getSource() == node || edge->getDestination() == node) {
			result++;
		}
	}
	return result;
}

std::vector<GraphNode*> GraphNetwork::getPredecessors(GraphNode* node) const {
	return getNeighbors(node);
}

std::vector<GraphNode*> GraphNetwork::getSuccessors(GraphNode* node) const {
	return getNeighbors(node);
}

std::vector<GraphEdge*> GraphNetwork::getIncomingEdges(GraphNode* node) const {
	return getIncidentEdges(node);
}

std::vector<GraphEdge*> GraphNetwork::getOutgoingEdges(GraphNode* node) const {
	return getIncidentEdges(node);
}

unsigned int GraphNetwork::inDegree(GraphNode* node) const {
	return degree(node);
}

unsigned int GraphNetwork::outDegree(GraphNode* node) const {
	return degree(node);
}

GraphTraversalResult GraphNetwork::breadthFirstSearch(GraphNode* source) const {
	GraphTraversalResult result;
	if (!containsNode(source)) {
		return result;
	}

	std::set<GraphNode*> visited;
	std::queue<GraphNode*> queue;
	std::map<GraphNode*, unsigned int> distance;
	visited.insert(source);
	queue.push(source);
	distance[source] = 0;
	result.visitOrder.push_back(source);
	result.tree.push_back({source, nullptr, nullptr, 0});

	while (!queue.empty()) {
		GraphNode* current = queue.front();
		queue.pop();
		for (GraphEdge* edge : getOutgoingEdges(current)) {
			GraphNode* next = _oppositeNode(edge, current);
			if (next == nullptr || visited.find(next) != visited.end()) {
				continue;
			}
			visited.insert(next);
			distance[next] = distance[current] + 1;
			queue.push(next);
			result.visitOrder.push_back(next);
			result.tree.push_back({next, current, edge, distance[next]});
		}
	}
	return result;
}

GraphTraversalResult GraphNetwork::depthFirstSearch(GraphNode* source) const {
	GraphTraversalResult result;
	if (!containsNode(source)) {
		return result;
	}

	std::set<GraphNode*> visited;
	std::function<void(GraphNode*, GraphNode*, GraphEdge*, unsigned int)> visit =
		[&](GraphNode* current, GraphNode* predecessor, GraphEdge* edgeToCurrent, unsigned int distance) {
			visited.insert(current);
			result.visitOrder.push_back(current);
			result.tree.push_back({current, predecessor, edgeToCurrent, distance});
			for (GraphEdge* edge : getOutgoingEdges(current)) {
				GraphNode* next = _oppositeNode(edge, current);
				if (next != nullptr && visited.find(next) == visited.end()) {
					visit(next, current, edge, distance + 1);
				}
			}
		};
	visit(source, nullptr, nullptr, 0);
	return result;
}

bool GraphNetwork::hasPath(GraphNode* source, GraphNode* destination) const {
	if (source == destination && containsNode(source)) {
		return true;
	}
	return breadthFirstSearch(source).visited(destination);
}

std::vector<GraphNode*> GraphNetwork::reachableNodes(GraphNode* source) const {
	return breadthFirstSearch(source).visitOrder;
}

GraphPathResult GraphNetwork::shortestPathUnweighted(GraphNode* source, GraphNode* destination) const {
	GraphTraversalResult traversal = breadthFirstSearch(source);
	if (!traversal.visited(destination)) {
		return {};
	}
	std::map<GraphNode*, GraphNode*> predecessors;
	std::map<GraphNode*, GraphEdge*> predecessorEdges;
	for (const GraphSearchTreeEntry& entry : traversal.tree) {
		predecessors[entry.node] = entry.predecessor;
		predecessorEdges[entry.node] = entry.edge;
	}
	return _reconstructPath(source, destination, predecessors, predecessorEdges, traversal.distanceTo(destination));
}

GraphPathResult GraphNetwork::shortestPathDijkstra(GraphNode* source, GraphNode* destination) const {
	std::string errorMessage;
	if (_hasNegativeWeight(errorMessage)) {
		GraphPathResult result;
		result.errorMessage = errorMessage;
		return result;
	}
	if (!containsNode(source) || !containsNode(destination)) {
		return {};
	}

	const double infinity = std::numeric_limits<double>::infinity();
	std::map<GraphNode*, double> distance;
	std::map<GraphNode*, GraphNode*> predecessors;
	std::map<GraphNode*, GraphEdge*> predecessorEdges;
	std::set<GraphNode*> visited;
	for (GraphNode* node : _nodes) {
		distance[node] = infinity;
	}
	distance[source] = 0.0;

	while (visited.size() < _nodes.size()) {
		GraphNode* current = nullptr;
		double best = infinity;
		for (GraphNode* node : _nodes) {
			if (visited.find(node) == visited.end() && distance[node] < best) {
				current = node;
				best = distance[node];
			}
		}
		if (current == nullptr) {
			break;
		}
		if (current == destination) {
			break;
		}
		visited.insert(current);
		for (GraphEdge* edge : getOutgoingEdges(current)) {
			GraphNode* next = _oppositeNode(edge, current);
			if (next == nullptr || visited.find(next) != visited.end()) {
				continue;
			}
			double candidate = distance[current] + edge->getWeight(1.0);
			if (candidate < distance[next]) {
				distance[next] = candidate;
				predecessors[next] = current;
				predecessorEdges[next] = edge;
			}
		}
	}
	if (!std::isfinite(distance[destination])) {
		return {};
	}
	return _reconstructPath(source, destination, predecessors, predecessorEdges, distance[destination]);
}

std::vector<std::vector<GraphNode*>> GraphNetwork::connectedComponents() const {
	std::vector<std::vector<GraphNode*>> components;
	std::set<GraphNode*> visited;
	for (GraphNode* node : _nodes) {
		if (visited.find(node) != visited.end()) {
			continue;
		}
		GraphTraversalResult traversal = breadthFirstSearch(node);
		for (GraphNode* reached : traversal.visitOrder) {
			visited.insert(reached);
		}
		components.push_back(traversal.visitOrder);
	}
	return components;
}

bool GraphNetwork::hasCycle() const {
	std::set<GraphNode*> visited;
	std::function<bool(GraphNode*, GraphEdge*)> visit = [&](GraphNode* node, GraphEdge* parentEdge) {
		visited.insert(node);
		for (GraphEdge* edge : getIncidentEdges(node)) {
			if (edge == nullptr) {
				continue;
			}
			GraphNode* next = _oppositeNode(edge, node);
			if (next == nullptr) {
				continue;
			}
			if (next == node) {
				return true;
			}
			if (visited.find(next) == visited.end()) {
				if (visit(next, edge)) {
					return true;
				}
			} else if (edge != parentEdge) {
				return true;
			}
		}
		return false;
	};
	for (GraphNode* node : _nodes) {
		if (visited.find(node) == visited.end() && visit(node, nullptr)) {
			return true;
		}
	}
	return false;
}

GraphTopologicalOrderResult GraphNetwork::topologicalOrder() const {
	GraphTopologicalOrderResult result;
	result.errorMessage = "Topological order is defined only for directed acyclic graphs.";
	return result;
}

std::string GraphNetwork::show() {
	return DefaultNetwork::show() +
	       ", directed=" + std::string(isDirected() ? "true" : "false") +
	       ", nodes=" + std::to_string(_nodes.size()) +
	       ", edges=" + std::to_string(_edges.size());
}

bool GraphNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNetwork::_loadInstance(fields);
	if (!res) {
		return false;
	}

	_nodes.clear();
	_edges.clear();
	std::map<std::string, GraphNode*> nodesByName;
	PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();

	unsigned int nodesSize = fields->loadField("graphNodesSize", 0u);
	for (unsigned int i = 0; i < nodesSize; i++) {
		const std::string prefix = "graphNode" + Util::StrIndex(i) + ".";
		auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		for (auto it = fields->begin(); it != fields->end(); ++it) {
			if (it->first.rfind(prefix, 0) == 0) {
				PersistenceRecord::Entry entry = it->second;
				entry.first = it->first.substr(prefix.size());
				nodeFields->insert(entry);
			}
		}
		if (nodeFields->size() == 0) {
			continue;
		}
		std::string nodeType = nodeFields->loadField("typename", Util::TypeOf<GraphNode>());
		Plugin* nodePlugin = plugins->find(nodeType);
		ModelDataDefinition* loaded = nodePlugin != nullptr
			                              ? nodePlugin->loadNew(_parentModel, nodeFields.get())
			                              : GraphNode::LoadInstance(_parentModel, nodeFields.get());
		GraphNode* node = dynamic_cast<GraphNode*>(loaded);
		if (node == nullptr) {
			traceError("Loaded graph node is not a GraphNode for typename \"" + nodeType + "\"");
			continue;
		}
		node->setModelLevel(_id);
		addNode(node);
		nodesByName[node->getName()] = node;
	}

	unsigned int edgesSize = fields->loadField("graphEdgesSize", 0u);
	for (unsigned int i = 0; i < edgesSize; i++) {
		const std::string prefix = "graphEdge" + Util::StrIndex(i) + ".";
		auto edgeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		for (auto it = fields->begin(); it != fields->end(); ++it) {
			if (it->first.rfind(prefix, 0) == 0) {
				PersistenceRecord::Entry entry = it->second;
				entry.first = it->first.substr(prefix.size());
				edgeFields->insert(entry);
			}
		}
		if (edgeFields->size() == 0) {
			continue;
		}
		std::string edgeType = edgeFields->loadField("typename", Util::TypeOf<GraphEdge>());
		Plugin* edgePlugin = plugins->find(edgeType);
		ModelDataDefinition* loaded = edgePlugin != nullptr
			                              ? edgePlugin->loadNew(_parentModel, edgeFields.get())
			                              : GraphEdge::LoadInstance(_parentModel, edgeFields.get());
		GraphEdge* edge = dynamic_cast<GraphEdge*>(loaded);
		if (edge == nullptr) {
			traceError("Loaded graph edge is not a GraphEdge for typename \"" + edgeType + "\"");
			continue;
		}
		auto sourceIt = nodesByName.find(edge->getSourceNodeName());
		auto destinationIt = nodesByName.find(edge->getDestinationNodeName());
		if (sourceIt == nodesByName.end() || destinationIt == nodesByName.end()) {
			traceError("Skipping graph edge with unknown endpoint while loading GraphNetwork \"" + getName() + "\"");
			continue;
		}
		edge->setSource(sourceIt->second);
		edge->setDestination(destinationIt->second);
		edge->setModelLevel(_id);
		addEdge(edge);
	}
	return true;
}

void GraphNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNetwork::_saveInstance(fields, saveDefaultValues);
	fields->saveField("graphDirected", isDirected() ? 1u : 0u, 0u, saveDefaultValues);
	fields->saveField("graphNodesSize", static_cast<unsigned int>(_nodes.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _nodes.size(); i++) {
		const std::string prefix = "graphNode" + Util::StrIndex(i) + ".";
		auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(nodeFields.get(), _nodes[i]);
		for (auto it = nodeFields->begin(); it != nodeFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
	}
	fields->saveField("graphEdgesSize", static_cast<unsigned int>(_edges.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _edges.size(); i++) {
		const std::string prefix = "graphEdge" + Util::StrIndex(i) + ".";
		auto edgeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(edgeFields.get(), _edges[i]);
		for (auto it = edgeFields->begin(); it != edgeFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
	}
}

bool GraphNetwork::_check(std::string& errorMessage) {
	bool resultAll = DefaultNetwork::_check(errorMessage);
	std::set<Util::identification> nodeIds;
	std::set<Util::identification> edgeIds;

	for (GraphNode* node : _nodes) {
		if (node == nullptr) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has a null node. ";
			resultAll = false;
			continue;
		}
		if (!nodeIds.insert(node->getId()).second) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has duplicate node id " + Util::StrIndex(node->getId()) + ". ";
			resultAll = false;
		}
	}
	for (GraphEdge* edge : _edges) {
		if (edge == nullptr) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has a null edge. ";
			resultAll = false;
			continue;
		}
		if (!edgeIds.insert(edge->getId()).second) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has duplicate edge id " + Util::StrIndex(edge->getId()) + ". ";
			resultAll = false;
		}
		if (edge->getSource() == nullptr || edge->getDestination() == nullptr) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has edge \"" + edge->getName() + "\" with null endpoint. ";
			resultAll = false;
		}
		if (edge->getSource() != nullptr && !containsNode(edge->getSource())) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has edge \"" + edge->getName() + "\" whose source is outside the graph. ";
			resultAll = false;
		}
		if (edge->getDestination() != nullptr && !containsNode(edge->getDestination())) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has edge \"" + edge->getName() + "\" whose destination is outside the graph. ";
			resultAll = false;
		}
		if (edge->hasWeight() && !std::isfinite(edge->getWeight())) {
			errorMessage += "GraphNetwork \"" + getName() + "\" has edge \"" + edge->getName() + "\" with non-finite weight. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void GraphNetwork::_initBetweenReplications() {
	DefaultNetwork::_initBetweenReplications();
}

bool GraphNetwork::_edgeConnects(GraphEdge* edge, GraphNode* source, GraphNode* destination) const {
	if (edge == nullptr || source == nullptr || destination == nullptr) {
		return false;
	}
	return (edge->getSource() == source && edge->getDestination() == destination) ||
	       (edge->getSource() == destination && edge->getDestination() == source);
}

GraphNode* GraphNetwork::_oppositeNode(GraphEdge* edge, GraphNode* node) const {
	if (edge == nullptr || node == nullptr) {
		return nullptr;
	}
	if (edge->getSource() == node) {
		return edge->getDestination();
	}
	if (edge->getDestination() == node) {
		return edge->getSource();
	}
	return nullptr;
}

void GraphNetwork::_removeIncidentEdges(GraphNode* node) {
	_edges.erase(std::remove_if(_edges.begin(), _edges.end(), [node](GraphEdge* edge) {
		return edge != nullptr && (edge->getSource() == node || edge->getDestination() == node);
	}), _edges.end());
}

bool GraphNetwork::_hasNegativeWeight(std::string& errorMessage) const {
	for (GraphEdge* edge : _edges) {
		if (edge != nullptr && edge->hasWeight() && edge->getWeight() < 0.0) {
			errorMessage = "Dijkstra requires nonnegative edge weights; edge \"" + edge->getName() + "\" has negative weight.";
			return true;
		}
	}
	return false;
}

GraphPathResult GraphNetwork::_reconstructPath(GraphNode* source,
                                               GraphNode* destination,
                                               const std::map<GraphNode*, GraphNode*>& predecessors,
                                               const std::map<GraphNode*, GraphEdge*>& predecessorEdges,
                                               double distance) const {
	GraphPathResult result;
	result.reachable = true;
	result.distance = distance;

	std::vector<GraphNode*> reversedNodes;
	std::vector<GraphEdge*> reversedEdges;
	GraphNode* current = destination;
	while (current != nullptr) {
		reversedNodes.push_back(current);
		if (current == source) {
			break;
		}
		auto edgeIt = predecessorEdges.find(current);
		if (edgeIt != predecessorEdges.end() && edgeIt->second != nullptr) {
			reversedEdges.push_back(edgeIt->second);
		}
		auto predecessorIt = predecessors.find(current);
		current = predecessorIt != predecessors.end() ? predecessorIt->second : nullptr;
	}

	if (reversedNodes.empty() || reversedNodes.back() != source) {
		return {};
	}
	result.nodes.assign(reversedNodes.rbegin(), reversedNodes.rend());
	result.edges.assign(reversedEdges.rbegin(), reversedEdges.rend());
	return result;
}
