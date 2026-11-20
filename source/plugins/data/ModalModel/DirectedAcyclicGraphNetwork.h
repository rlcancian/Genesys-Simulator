/*
 * File:   DirectedAcyclicGraphNetwork.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef DIRECTEDACYCLICGRAPHNETWORK_H
#define DIRECTEDACYCLICGRAPHNETWORK_H

#include "DirectedGraphNetwork.h"

/*!
 * \brief Directed graph specialization that enforces the DAG invariant.
 *
 * Edges that would create a directed cycle are rejected on insertion, and the
 * model checker validates the invariant again for corrupted persistence.
 */
class DirectedAcyclicGraphNetwork : public DirectedGraphNetwork {
public:
	DirectedAcyclicGraphNetwork(Model* model, std::string name = "");
	virtual ~DirectedAcyclicGraphNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	virtual bool addEdge(GraphEdge* edge) override;

protected:
	virtual bool _check(std::string& errorMessage) override;
};

#endif /* DIRECTEDACYCLICGRAPHNETWORK_H */
