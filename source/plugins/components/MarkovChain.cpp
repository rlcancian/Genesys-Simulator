/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   MarkovChain.cpp
 * Author: rlcancian
 * 
 * Created on 24 de Outubro de 2019, 18:26
 */

#include "MarkovChain.h"

#include "../../kernel/simulator/Model.h"
#include "../data/Variable.h"
//#include "ProbDistribDefaultImpl1.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/TraitsKernel.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MarkovChain::GetPluginInformation;
}
#endif

ModelDataDefinition* MarkovChain::NewInstance(Model* model, std::string name) {
	return new MarkovChain(model, name);
}

MarkovChain::MarkovChain(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<MarkovChain>(), name) {
	_sampler = new TraitsKernel<Sampler_if>::Implementation();
}

std::string MarkovChain::show() {
	return ModelComponent::show() + "";
}

ModelComponent* MarkovChain::LoadInstance(Model* model, PersistenceRecord *fields) {
	MarkovChain* newComponent = new MarkovChain(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void MarkovChain::setTransitionProbabilityMatrix(Variable* _transitionMatrix) {
	this->_transitionProbMatrix = _transitionMatrix;
}

Variable* MarkovChain::getTransitionMatrix() const {
	return _transitionProbMatrix;
}

Variable* MarkovChain::getCurrentState() const {
	return _currentState;
}

void MarkovChain::setCurrentState(Variable* _currentState) {
	this->_currentState = _currentState;
}

void MarkovChain::setInitialDistribution(Variable* _initialDistribution) {
	this->_initialDistribution = _initialDistribution;
}

Variable* MarkovChain::getInitialState() const {
	return _initialDistribution;
}

void MarkovChain::setInitilized(bool _initilized) {
	this->_initilized = _initilized;
}

bool MarkovChain::isInitilized() const {
	return _initilized;
}

void MarkovChain::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	//_parentModel->getTracer()->trace("I'm just a dummy model and I'll just send the entity forward");
	unsigned int size;
	double rnd, sum, value;
	if (!_initilized) {
		// define the initial state based on initial probabilities
		size = _initialDistribution->getDimensionSizes()->front();
		rnd = _sampler->random(); //parentSimulator()->tools()->sampler()->random();
		double sum = 0.0;
		for (unsigned int i = 0; i < size; i++) {
			value = _initialDistribution->getValue(std::to_string(i));
			sum += value;
			if (sum > rnd) {
				_currentState->setValue(i); // _currentState =  i;
				break;
			}
		}
		_parentModel->getTracer()->traceSimulation(this, "Initial current state=" + std::to_string(_currentState->getValue()));
		_initilized = true;
	} else {
		size = _transitionProbMatrix->getDimensionSizes()->front();
		rnd = _sampler->random(); //parentSimulator()->tools()->sampler()->random();
		sum = 0.0;
		for (unsigned int i = 0; i < size; i++) {
			std::string index = std::to_string(static_cast<unsigned int> (_currentState->getValue())) + "," + std::to_string(i);
			value = _transitionProbMatrix->getValue(index);
			sum += value;
			if (sum > rnd) {
				_currentState->setValue(i);
				break;
			}
		}
		_parentModel->getTracer()->trace("Current state=" + std::to_string(_currentState->getValue()));
	}
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool MarkovChain::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}
	return res;
}

void MarkovChain::_initBetweenReplications() {
	this->_initilized = false;
}

void MarkovChain::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool MarkovChain::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

PluginInformation* MarkovChain::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MarkovChain>(), &MarkovChain::LoadInstance, &MarkovChain::NewInstance);
	info->setCategory("Discrete Processing");
	// ...
	return info;
}

