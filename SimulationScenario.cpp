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
#include "Simulator.h"
#include "Traits.h"

SimulationScenario::SimulationScenario() {
}

SimulationScenario::~SimulationScenario() {
    delete this->_responseValues;
    delete this->_selectedControls;
    delete this->_selectedResponses;
}

bool SimulationScenario::startSimulation(Simulator * simulator, std::string* errorMessage) {
    // model->loadmodel _modelFilename
    // set values for the _selectedControls
    // model->startSimulation
    // get the value of the _selectedResponses from the model and store results on _responseValues
    // clear selected controls and responses
    // close the model

    // Setting control values   
    bool loaded = simulator->getModels()->loadModel(this->_modelFilename);
    if (!loaded)
        return false;
    Model * model = simulator->getModels()->current();
    
    for (
            std::list<std::pair<std::string, double>*>::iterator scen_it = this->_selectedControls->begin();
            scen_it != this->_selectedControls->end();
            ++scen_it
            ) {
        for (
                std::list<SimulationControl*>::iterator mod_it = model->getControls()->list()->begin();
                mod_it != model->getControls()->list()->end();
                ++mod_it
                ) {
            if (!(*scen_it)->first.compare((*mod_it)->getName())) {
                
                (*mod_it)->setValue((*scen_it)->second);
            }
        }
    }
    
    // Running simulation
    model->getSimulation()->start();

    // Acquiring response values
    for (
            std::list<std::string>::iterator scen_it = this->_selectedResponses->begin();
            scen_it != this->_selectedResponses->end();
            ++scen_it
            ) {
        for (
                std::list<SimulationResponse*>::iterator mod_it = model->getResponses()->list()->begin();
                mod_it != model->getResponses()->list()->end();
                ++mod_it
                ) {
            if (!(*scen_it).compare((*mod_it)->getName())) {
                this->_responseValues->push_back(new std::pair<std::string, double>((*scen_it), (*mod_it)->getValue()));
            }
        }
    }
    
    this->_selectedControls = new std::list<std::pair<std::string, double>*>();
    this->_selectedResponses = new std::list<std::string>();
    
    return true;
}

void SimulationScenario::setScenarioName(std::string _name) {
    this->_scenarioName = _name;
}

std::string SimulationScenario::getScenarioName() const {
    return _scenarioName;
}

std::list<std::pair<std::string, double>*>* SimulationScenario::getResponseValues() const {
    return _responseValues;
}

double SimulationScenario::getResponseValue(std::string responseName) {
    for(std::list<std::pair<std::string, double>*>::iterator it = this->_responseValues->begin(); it != this->_responseValues->end(); ++it) {
        if (!(*it)->first.compare(responseName))
            return (*it)->second;
    }
    return -1;
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