/*
 * File:   GraphNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef GRAPHNETWORK_H
#define GRAPHNETWORK_H

#include "DefaultNetwork.h"

#include <map>
#include <string>
#include <vector>

class GraphEdge;
class GraphNode;

struct GraphSearchTreeEntry {
	GraphNode* node = nullptr;
	GraphNode* predecessor = nullptr;
	GraphEdge* edge = nullptr;
	unsigned int distance = 0;
};

/*!
 * \brief Result of a deterministic graph traversal.
 *
 * The traversal order follows node/edge insertion order. Temporary search
 * structures are not persisted in GraphNetwork.
 */
struct GraphTraversalResult {
	std::vector<GraphNode*> visitOrder;
	std::vector<GraphSearchTreeEntry> tree;

	bool visited(GraphNode* node) const;
	GraphNode* predecessorOf(GraphNode* node) const;
	GraphEdge* edgeTo(GraphNode* node) const;
	unsigned int distanceTo(GraphNode* node) const;
};

/*!
 * \brief Explicit graph path result.
 *
 * `reachable=false` represents absence of a path. `errorMessage` reports
 * rejected operations such as Dijkstra over negative-weight edges.
 */
struct GraphPathResult {
	bool reachable = false;
	double distance = 0.0;
	std::vector<GraphNode*> nodes;
	std::vector<GraphEdge*> edges;
	std::string errorMessage = "";
};

struct GraphTopologicalOrderResult {
	bool acyclic = false;
	std::vector<GraphNode*> order;
	std::string errorMessage = "";
};

/*!
 * \brief Mathematical graph specialization of DefaultNetwork.
 *
 * GraphNetwork represents G=(V,E) and algorithms over that topology. It is not
 * a process network of ModelComponent+Connection, and its inherited activation
 * remains inert: a pure graph has no current vertex or firing/movement
 * semantics. Dynamic traversal, random walks and routing belong to future
 * explicit specializations/adapters.
 *
 * Algorithms use insertion-order adjacency for deterministic behavior:
 * BFS/DFS/connected components/cycle detection are O(V+E); unweighted shortest
 * path is BFS O(V+E); Dijkstra with the simple implementation here is
 * O(V^2+E). Temporary visited, predecessor, queue, stack and distance data are
 * local to each algorithm call and are never persisted.
 */
class GraphNetwork : public DefaultNetwork {
public:
	GraphNetwork(Model* model, std::string name = "", std::string dataDefinitionTypename = Util::TypeOf<GraphNetwork>());
	virtual ~GraphNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	virtual bool isDirected() const;
	virtual bool addNode(GraphNode* node);
	virtual bool removeNode(GraphNode* node);
	bool containsNode(GraphNode* node) const;
	GraphNode* findNodeByName(const std::string& name) const;
	std::vector<GraphNode*> getNodes() const;

	virtual bool addEdge(GraphEdge* edge);
	virtual bool removeEdge(GraphEdge* edge);
	bool containsEdge(GraphEdge* edge) const;
	GraphEdge* findEdgeByName(const std::string& name) const;
	std::vector<GraphEdge*> getEdges() const;
	bool hasEdge(GraphNode* source, GraphNode* destination) const;
	std::vector<GraphEdge*> getEdgesBetween(GraphNode* source, GraphNode* destination) const;

	std::vector<GraphNode*> getNeighbors(GraphNode* node) const;
	std::vector<GraphEdge*> getIncidentEdges(GraphNode* node) const;
	unsigned int degree(GraphNode* node) const;
	virtual std::vector<GraphNode*> getPredecessors(GraphNode* node) const;
	virtual std::vector<GraphNode*> getSuccessors(GraphNode* node) const;
	virtual std::vector<GraphEdge*> getIncomingEdges(GraphNode* node) const;
	virtual std::vector<GraphEdge*> getOutgoingEdges(GraphNode* node) const;
	virtual unsigned int inDegree(GraphNode* node) const;
	virtual unsigned int outDegree(GraphNode* node) const;

	GraphTraversalResult breadthFirstSearch(GraphNode* source) const;
	GraphTraversalResult depthFirstSearch(GraphNode* source) const;
	bool hasPath(GraphNode* source, GraphNode* destination) const;
	std::vector<GraphNode*> reachableNodes(GraphNode* source) const;
	GraphPathResult shortestPathUnweighted(GraphNode* source, GraphNode* destination) const;
	GraphPathResult shortestPathDijkstra(GraphNode* source, GraphNode* destination) const;
	std::vector<std::vector<GraphNode*>> connectedComponents() const;
	virtual bool hasCycle() const;
	virtual GraphTopologicalOrderResult topologicalOrder() const;
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;

protected:
	virtual bool _edgeConnects(GraphEdge* edge, GraphNode* source, GraphNode* destination) const;
	virtual GraphNode* _oppositeNode(GraphEdge* edge, GraphNode* node) const;

private:
	void _removeIncidentEdges(GraphNode* node);
	bool _hasNegativeWeight(std::string& errorMessage) const;
	GraphPathResult _reconstructPath(GraphNode* source,
	                                 GraphNode* destination,
	                                 const std::map<GraphNode*, GraphNode*>& predecessors,
	                                 const std::map<GraphNode*, GraphEdge*>& predecessorEdges,
	                                 double distance) const;

private:
	std::vector<GraphNode*> _nodes;
	std::vector<GraphEdge*> _edges;
};

#endif /* GRAPHNETWORK_H */
