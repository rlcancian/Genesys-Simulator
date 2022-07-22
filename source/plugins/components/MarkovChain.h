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

#include "../../kernel/simulator/ModelComponent.h"
#include "../data/Variable.h"
#include "../../kernel/statistics/Sampler_if.h"

class MarkovChain : public ModelComponent {
public: // constructors
	MarkovChain(Model* model, std::string name = "");
	virtual ~MarkovChain() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // get and set
	void setTransitionProbabilityMatrix(Variable* _transitionMatrix);
	Variable* getTransitionMatrix() const;
	Variable* getCurrentState() const;
	void setInitialDistribution(Variable* _initialDistribution);
	Variable* getInitialState() const;
	void setInitilized(bool _initilized);
	bool isInitilized() const;
	void setCurrentState(Variable* _currentState);
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual void _initBetweenReplications();
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private: // methods
private: // attributes 1:1
	Variable* _transitionProbMatrix;
	Variable* _initialDistribution;
	Variable* _currentState;
	bool _initilized = false;
private: // attributes 1:n
	Sampler_if* _sampler;
};

#endif /* MARKOVCHAIN_H */

