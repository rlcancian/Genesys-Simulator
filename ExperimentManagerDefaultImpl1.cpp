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
#include "Simulator.h"

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
        Simulator *simulator = new Simulator();
        Model *model = new Model(simulator);
        model->load(modelFilename);
        return model->getControls();
}

List<SimulationResponse*>* ExperimentManagerDefaultImpl1::extractResponsesFromModel(std::string modelFilename) const {
	Simulator *simulator = new Simulator();
        Model *model = new Model(simulator);
        model->load(modelFilename);
        return model->getResponses();
}

void ExperimentManagerDefaultImpl1::startSimulationOfScenario(SimulationScenario* scenario) {
    std::string a("AAA");
    scenario->startSimulation(&a);
}

void ExperimentManagerDefaultImpl1::startSimulation() {
    for (std::list<SimulationScenario*>::iterator i = _scenarios->list()->begin(); i != _scenarios->list()->end(); ++i) {
        startSimulationOfScenario(*i);
    }
}

void ExperimentManagerDefaultImpl1::stopSimulation() {
	// \todo: implement
}

void ExperimentManagerDefaultImpl1::addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) {
}