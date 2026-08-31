/*
 * File:   GraphEdge.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef GRAPHEDGE_H
#define GRAPHEDGE_H

#include "kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

#include <string>

class GraphNode;

/*!
 * \brief Mathematical graph edge between two GraphNode endpoints.
 *
 * GraphEdge is not a GenESyS process Connection and does not imply Entity
 * flow. It supports directed or undirected interpretation, optional numeric
 * weight, self-loops and parallel edges. A GraphNetwork decides whether the
 * edge is interpreted as directed or undirected when the edge is inserted.
 */
class GraphEdge : public ModelDataDefinition {
public:
	GraphEdge(Model* model, std::string name = "");
	GraphEdge(Model* model, GraphNode* source, GraphNode* destination, std::string name = "");
	virtual ~GraphEdge() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setSource(GraphNode* source);
	GraphNode* getSource() const;
	void setDestination(GraphNode* destination);
	GraphNode* getDestination() const;
	void setDirected(bool directed);
	bool isDirected() const;
	void setWeight(double weight);
	void clearWeight();
	bool hasWeight() const;
	double getWeight(double defaultWeight = 1.0) const;
	std::string getSourceNodeName() const;
	std::string getDestinationNodeName() const;
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	void _syncEndpointNames();

private:
	const struct DEFAULT_VALUES {
		const bool directed = false;
		const bool hasWeight = false;
		const double weight = 1.0;
	} DEFAULT;

	GraphNode* _source = nullptr;
	GraphNode* _destination = nullptr;
	std::string _sourceNodeName = "";
	std::string _destinationNodeName = "";
	bool _directed = DEFAULT.directed;
	bool _hasWeight = DEFAULT.hasWeight;
	double _weight = DEFAULT.weight;
};

#endif /* GRAPHEDGE_H */
