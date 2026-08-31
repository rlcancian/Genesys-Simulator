/*
 * File:   DirectedGraphNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef DIRECTEDGRAPHNETWORK_H
#define DIRECTEDGRAPHNETWORK_H

#include "GraphNetwork.h"

/*!
 * \brief Directed mathematical graph specialization.
 *
 * In DirectedGraphNetwork an edge u->v makes v a successor of u and u a
 * predecessor of v. It does not imply the reverse relation. Self-loops and
 * parallel edges are supported.
 */
class DirectedGraphNetwork : public GraphNetwork {
public:
	DirectedGraphNetwork(Model* model, std::string name = "", std::string dataDefinitionTypename = Util::TypeOf<DirectedGraphNetwork>());
	virtual ~DirectedGraphNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	virtual bool isDirected() const override;
	virtual std::vector<GraphNode*> getPredecessors(GraphNode* node) const override;
	virtual std::vector<GraphNode*> getSuccessors(GraphNode* node) const override;
	virtual std::vector<GraphEdge*> getIncomingEdges(GraphNode* node) const override;
	virtual std::vector<GraphEdge*> getOutgoingEdges(GraphNode* node) const override;
	virtual unsigned int inDegree(GraphNode* node) const override;
	virtual unsigned int outDegree(GraphNode* node) const override;
	std::vector<std::vector<GraphNode*>> stronglyConnectedComponents() const;
	virtual bool hasCycle() const override;
	virtual GraphTopologicalOrderResult topologicalOrder() const override;

protected:
	virtual bool _edgeConnects(GraphEdge* edge, GraphNode* source, GraphNode* destination) const override;
	virtual GraphNode* _oppositeNode(GraphEdge* edge, GraphNode* node) const override;
};

#endif /* DIRECTEDGRAPHNETWORK_H */
