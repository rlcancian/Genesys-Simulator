/*
 * File:   GraphNode.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#ifndef GRAPHNODE_H
#define GRAPHNODE_H

#include "plugins/components/ModalModel/DefaultNode.h"

/*!
 * \brief Mathematical graph vertex used by GraphNetwork.
 *
 * A GraphNode is a model data definition, not a process ModelComponent. It
 * reuses DefaultNode's persistable identity/name/flag infrastructure without
 * inheriting process-flow Connection semantics.
 */
class GraphNode : public DefaultNode {
public:
	GraphNode(Model* model, std::string name = "");
	virtual ~GraphNode() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
};

#endif /* GRAPHNODE_H */
