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

#include <unordered_map>
#include <exception>
#include "SimulationScenario.h"
#include "Simulator.h"
#include "Traits.h"

SimulationScenario::SimulationScenario() : _responseValues(new std::map<std::string, double>()),
_selectedControls(new std::map<std::string, double>),
_selectedResponses(new std::set<std::string>()) {
}

SimulationScenario::~SimulationScenario() {
    delete _responseValues;
    delete _selectedControls;
    delete _selectedResponses;
}

bool SimulationScenario::startSimulation(Simulator * simulator) {
    // model->loadmodel _modelFilename
    // set values for the _selectedControls
    // model->startSimulation
    // get the value of the _selectedResponses from the model and store results on _responseValues
    // clear selected controls and responses
    // close the model
    
    delete _responseValues;
    _responseValues = new std::map<std::string, double>();
    
    auto loaded = simulator->getModels()->loadModel(_modelFilename);
    if (!loaded)
        return false;
    auto model = simulator->getModels()->current();

    // Setting control values   
    auto control_list = model->getControls();
    for (auto i = 0; i < control_list->size(); ++i) {
        auto control = control_list->getAtRank(i);
        auto control_name = control->getName();
        try {
            control->setValue(_selectedControls->at(control_name));
        } catch (std::out_of_range& e) {
            continue;
        }
    }

    // Running simulation
    model->getSimulation()->start();

    // Acquiring response values
    auto response_list = model->getResponses();
    for (auto i = 0; i < response_list->size(); ++i) {
        auto response = response_list->getAtRank(i);
        auto response_name = response->getName();
        if (_selectedResponses->find(response_name) != _selectedResponses->end()) {
            (*_responseValues)[response_name] = response->getValue();
        }
    }

    delete _selectedControls;
    delete _selectedResponses;
    _selectedControls = new std::map<std::string, double>;
    _selectedResponses = new std::set<std::string>();

    model->clear();
    simulator->getModels()->remove(model);
    return true;
}

void SimulationScenario::setScenarioName(std::string _name) {
    _scenarioName = _name;
}

std::string SimulationScenario::getScenarioName() const {
    return _scenarioName;
}

std::map<std::string, double>* SimulationScenario::getResponseValues() const {
    return _responseValues;
}

double SimulationScenario::getResponseValue(std::string responseName) {
    return (*_responseValues)[responseName];
}

void SimulationScenario::selectResponse(std::string responseName) {
    _selectedResponses->emplace(responseName);
}

void SimulationScenario::setModelFilename(std::string modelFilename) {
    _modelFilename = modelFilename;
}

std::string SimulationScenario::getModelFilename() const {
    return _modelFilename;
}

std::set<std::string>* SimulationScenario::getSelectedResponses() const {
    return _selectedResponses;
}

std::map<std::string, double>* SimulationScenario::getSelectedControls() const {
    return _selectedControls;
}

void SimulationScenario::setControlValue(std::string controlName, double value) {
    (*_selectedControls)[controlName] = value;
}

void SimulationScenario::setScenarioDescription(std::string _scenarioDescription) {
    _scenarioDescription = _scenarioDescription;
}

std::string SimulationScenario::getScenarioDescription() const {
    return _scenarioDescription;
}
