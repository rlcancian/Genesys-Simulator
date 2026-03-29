/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ExperimentManager.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 4 de maio de 2021, 11:43
 */

#include "ExperimentManager.h"
#include "Simulator.h"

ExperimentManager::ExperimentManager(Simulator* simulator) {
	_simulator = simulator;
	_currentSimulationExperiment = nullptr;
}

List<SimulationExperiment*>* ExperimentManager::getExperiments() const {
	return _experiments;
}

SimulationExperiment* ExperimentManager::newSimulationExperiment() {
	_currentSimulationExperiment = new SimulationExperiment(); //_simulator
	return _currentSimulationExperiment;
}

void ExperimentManager::insert(SimulationExperiment* experiment) {
	_experiments->insert(experiment);
	this->_currentSimulationExperiment = experiment;
	if (_simulator != nullptr) {
		_simulator->getTraceManager()->trace("Experiment successfully inserted", TraceManager::Level::L2_results);
	}
}

void ExperimentManager::remove(SimulationExperiment* experiment) {
	_experiments->remove(experiment);
	if (_currentSimulationExperiment == experiment) {
		_currentSimulationExperiment = this->front();
	}
	delete experiment; //->~SimulationExperiment();
	if (_simulator != nullptr) {
		_simulator->getTraceManager()->trace("Experiment successfully removed", TraceManager::Level::L2_results);
	}
}

unsigned int ExperimentManager::size() {
	return _experiments->size();
}

bool ExperimentManager::saveSimulationExperiment(std::string filename) {
	/// @todo This method currently reflects the unfinished higher-level experiment layer.
	// @TODO: implement
	if (_currentSimulationExperiment != nullptr) {
		filename = ""; // just to avoid not using it
		//return _currentSimulationExperiment->save(filename);
	}
	return false;
}

bool ExperimentManager::loadSimulationExperiment(std::string filename) {
	/// @todo The higher-level experiment workflow is still under construction.
	SimulationExperiment* experiment = new SimulationExperiment(); //_simulator);
	// @TODO: not implemented yet!
	filename = ""; // jut to avoid not using it
	bool res = false; // = experiment->load(filename);
	if (res) {
		this->insert(experiment);
		if (_simulator != nullptr) {
			_simulator->getTraceManager()->trace("Experiment successfully loaded", TraceManager::Level::L2_results);
		}
	} else {
		delete experiment; //->~SimulationExperiment();
		if (_simulator != nullptr) {
			_simulator->getTraceManager()->trace("Experiment could not be loaded", TraceManager::Level::L2_results);
		}
	}
	return res;
}

void ExperimentManager::setCurrent(SimulationExperiment* experiment) {
	this->_currentSimulationExperiment = experiment;
}

SimulationExperiment* ExperimentManager::current() {
	return _currentSimulationExperiment;
}

SimulationExperiment* ExperimentManager::front() {
	return _experiments->front();
}

SimulationExperiment* ExperimentManager::next() {
	return _experiments->next();
}

