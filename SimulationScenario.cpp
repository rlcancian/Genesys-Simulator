/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationScenario.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 10 de Outubro de 2018, 18:21
 */

#include "SimulationScenario.h"

SimulationScenario::SimulationScenario() {
}

bool SimulationScenario::startSimulation(std::string* errorMessage) {
	// model->loadmodel _modelFilename
	// set values for the _selectedControls
	// model->startSimulation
	// get the value of the _selectedResponses from the model and store results on _responseValues
	// clear selected controls and responses
	// close the model
	return false;
}

void SimulationScenario::setScenarioName(std::string _name) {
	this->_scenarioName = _name;
}

std::string SimulationScenario::getScenarioName() const {
	return _scenarioName;
}

void SimulationScenario::setModelFilename(std::string _modelFilename) {
	this->_modelFilename = _modelFilename;
}

std::string SimulationScenario::getModelFilename() const {
	return _modelFilename;
}

std::list<std::string>* SimulationScenario::getSelectedResponses() const {
	return _selectedResponses;
}

std::list<std::pair<std::string, double>*>* SimulationScenario::getSelectedControls() const {
	return _selectedControls;
}

void SimulationScenario::setScenarioDescription(std::string _scenarioDescription) {
	this->_scenarioDescription = _scenarioDescription;
}

std::string SimulationScenario::getScenarioDescription() const {
	return _scenarioDescription;
}