/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ProcessAnalyserDefaultImpl1.cpp
 * Author: rlcancian
 * 
 * Created on 20 de Maio de 2019, 20:45
 */

#include "ExperimentManagerDefaultImpl1.h"

ExperimentManagerDefaultImpl1::ExperimentManagerDefaultImpl1() {
}

List<SimulationScenario*>* ExperimentManagerDefaultImpl1::getScenarios() const {
        return _scenarios;
}

List<SimulationControl*>* ExperimentManagerDefaultImpl1::getControls() const {
	return _controls;
}

List<SimulationResponse*>* ExperimentManagerDefaultImpl1::getResponses() const {
        return _responses;
}

List<SimulationControl*>* ExperimentManagerDefaultImpl1::extractControlsFromModel(std::string modelFilename) const {
	// \todo: implement
}

List<SimulationResponse*>* ExperimentManagerDefaultImpl1::extractResponsesFromModel(std::string modelFilename) const {
	// \todo: implement
}

void ExperimentManagerDefaultImpl1::startSimulationOfScenario(SimulationScenario* scenario) {
    scenario->startSimulation("error message goes here");
}

void ExperimentManagerDefaultImpl1::startSimulation() {
    for (std::list<SimulationScenario*>::iterator i = _scenarios->list()->begin(); i != _scenarios->list(); ++i) {
        startSimulationOfScenario(*i);
    }
}

void ExperimentManagerDefaultImpl1::stopSimulation() {
	// \todo: implement
}

void ExperimentManagerDefaultImpl1::addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) {
}