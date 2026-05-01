/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MarkovChain.h
 * Author: rlcancian
 *
 * Created on 24 de Outubro de 2019, 18:26
 */

#ifndef MARKOVCHAIN_H
#define MARKOVCHAIN_H

#include "kernel/simulator/ModelComponent.h"
#include "kernel/statistics/Sampler_if.h"

class MarkovChain : public ModelComponent {
public: // constructors
	MarkovChain(Model* model, std::string name = "");
	virtual ~MarkovChain() override;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // get and set
	void setTransitionProbabilityMatrix(ModelDataDefinition* transitionMatrix);
	ModelDataDefinition* getTransitionMatrix() const;
	void setCurrentState(ModelDataDefinition* currentState);
	ModelDataDefinition* getCurrentState() const;
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // virtual
	virtual void _initBetweenReplications() override;
	virtual bool _check(std::string& errorMessage) override;
//	virtual void _createInternalAndAttachedData() override;
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private: // methods
	unsigned int _drawNextState(Entity* entity, unsigned int currentState);
	double _readStateValue(Entity* entity) const;
	void _writeStateValue(Entity* entity, double value);
	double _readTransitionProbability(Entity* entity, unsigned int fromState, unsigned int toState) const;
	unsigned int _stateCount() const;
private: // attributes 1:1
	ModelDataDefinition* _transitionProbMatrix = nullptr;
	ModelDataDefinition* _currentState = nullptr;
private: // attributes 1:n
	Sampler_if* _sampler = nullptr;
};

#endif /* MARKOVCHAIN_H */
