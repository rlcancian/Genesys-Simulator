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
_controls(new List<SimulationControl*>()), _responses(new List<SimulationResponse*>()),
_scenarios(new List<SimulationScenario*>()), _loaded_models() {
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

List<SimulationControl*>* ExperimentManagerDefaultImpl1::getControls() const {
    return _controls;
}

List<SimulationResponse*>* ExperimentManagerDefaultImpl1::getResponses() const {
    return _responses;
}

List<SimulationControl*>* ExperimentManagerDefaultImpl1::extractControlsFromModel(std::string modelFilename) {
    Model * model;
    try {
        model = _loaded_models.at(modelFilename);
    } catch (std::out_of_range & e) {
        _simulator->getModels()->loadModel(modelFilename);
        model = _simulator->getModels()->current();
        _loaded_models[modelFilename] = model;
    }
    auto model_controls = model->getControls();
    for (auto i = 0; i < model_controls->size(); ++i) {
        auto control = model_controls->getAtRank(i);
        _controls->insert(control);
    }
    return model_controls;
}

List<SimulationResponse*>* ExperimentManagerDefaultImpl1::extractResponsesFromModel(std::string modelFilename) {
    Model * model;
    try {
        model = _loaded_models.at(modelFilename);
    } catch (std::out_of_range & e) {
        _simulator->getModels()->loadModel(modelFilename);
        model = _simulator->getModels()->current();
        _loaded_models[modelFilename] = model;
    }
    _simulator->getModels()->loadModel(modelFilename);
    model->check();
    auto model_responses = model->getResponses();
    for (auto i = 0; i < model_responses->size(); ++i) {
        auto response = model_responses->getAtRank(i);
        _responses->insert(response);
    }
    return model_responses;
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
