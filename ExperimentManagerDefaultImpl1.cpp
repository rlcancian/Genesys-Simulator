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

ExperimentManagerDefaultImpl1::ExperimentManagerDefaultImpl1() : _simulator(new Simulator()),
_controls(new std::unordered_set<std::string>()), _responses(new std::unordered_set<std::string>()),
_scenarios(new List<SimulationScenario*>()) {
}

ExperimentManagerDefaultImpl1::~ExperimentManagerDefaultImpl1() {
    delete _simulator;
    delete _responses;
    delete _controls;
    delete _scenarios;
}

List<SimulationScenario*>* ExperimentManagerDefaultImpl1::getScenarios() const {
    return _scenarios;
}

std::unordered_set<std::string>* ExperimentManagerDefaultImpl1::getControls() const {
    return _controls;
}

std::unordered_set<std::string>* ExperimentManagerDefaultImpl1::getResponses() const {
    return _responses;
}

List<std::string> ExperimentManagerDefaultImpl1::extractControlsFromModel(std::string modelFilename) const {
    List<std::string> model_control_names{};
    _simulator->getModels()->loadModel(modelFilename);
    auto model = _simulator->getModels()->current();
    auto model_controls = model->getControls();
    for (auto i = 0; i < model_controls->size(); ++i) {
        std::string control_name = model_controls->getAtRank(i)->getName();
        _controls->insert(control_name);
        model_control_names.insert(control_name);
    }
    _simulator->getModels()->remove(model);
    return std::move(model_control_names);
}

List<std::string> ExperimentManagerDefaultImpl1::extractResponsesFromModel(std::string modelFilename) const {
    List<std::string> model_response_names{};
    _simulator->getModels()->loadModel(modelFilename);
    auto model = _simulator->getModels()->current();
    model->getSimulation()->start();
    auto model_responses = model->getResponses();
    for (auto i = 0; i < model_responses->size(); ++i) {
        std::string response_name = model_responses->getAtRank(i)->getName();
        _responses->insert(response_name);
        model_response_names.insert(response_name);
    }
    _simulator->getModels()->remove(model);
    return std::move(model_response_names);
}

void ExperimentManagerDefaultImpl1::startSimulationOfScenario(SimulationScenario* scenario) {
    scenario->startSimulation(_simulator);
}

void ExperimentManagerDefaultImpl1::startExperiment() {
    for (auto i = 0; i < _scenarios->size(); ++i) {
        startSimulationOfScenario(_scenarios->getAtRank(i));
    }
}

void ExperimentManagerDefaultImpl1::stopExperiment() {
    auto model_manager = _simulator->getModels();
    auto model = model_manager->front();
    for (auto i = 0; i < model_manager->size(); ++i) {
        model->getSimulation()->stop();
        model = model_manager->next();
    }
}

void ExperimentManagerDefaultImpl1::addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) {
}

Simulator * ExperimentManagerDefaultImpl1::simulator() const {
    return _simulator;
}
