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
	// @ToDo: (importante): implement
	return nullptr;
}

//List<SimulationControl*>* ExperimentManagerDefaultImpl1::getControls() const {
//	return _controls;
//}

//List<SimulationResponse*>* ExperimentManagerDefaultImpl1::getResponses() const {
// @ToDo: (importante): implement
//	return nullptr;
//}

//List<SimulationControl*>* ExperimentManagerDefaultImpl1::extractControlsFromModel(std::string modelFilename) const {
// @ToDo: (importante): implement
//	modelFilename = ""; //just to use it
//	return nullptr;
//}

//List<SimulationResponse*>* ExperimentManagerDefaultImpl1::extractResponsesFromModel(std::string modelFilename) const {
// @ToDo: (importante): implement
//	modelFilename = ""; // juts to use it
//	return nullptr;
//}

void ExperimentManagerDefaultImpl1::startSimulationOfScenario(SimulationScenario* scenario) {
	// @ToDo: (importante): implement
}

void ExperimentManagerDefaultImpl1::startExperiment() {
	// @ToDo: (importante): implement
}

void ExperimentManagerDefaultImpl1::stopExperiment() {
	// @ToDo: (importante): implement
}

void ExperimentManagerDefaultImpl1::addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) {
}
