/*
 * File:   MarkovState.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 31 de Agosto de 2026
 */

#ifndef MARKOVSTATE_H
#define MARKOVSTATE_H

#include "plugins/components/ModalModel/DefaultNode.h"

/*!
 * \brief State node used by MarkovChainNetwork DTMC models.
 *
 * MarkovState is a ModelDataDefinition, not a process ModelComponent. It
 * represents one finite state in a time-homogeneous discrete-time Markov chain.
 */
class MarkovState : public DefaultNode {
public:
	MarkovState(Model* model, std::string name = "");
	virtual ~MarkovState() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
};

#endif /* MARKOVSTATE_H */
