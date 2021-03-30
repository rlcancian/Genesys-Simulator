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

SimulationScenario::SimulationScenario() {
}

bool SimulationScenario::startSimulation(std::string* errorMessage) {
    // model->loadmodel _modelFilename
    // set values for the _selectedControls
    // model->startSimulation
    // get the value of the _selectedResponses from the model and store results on _responseValues
    // clear selected controls and responses
    // close the model

    // Creating and loading model
    std::cout <<this->getScenarioName()<< ": I shall do the honour of loading the model from "<< this->_modelFilename << std::endl;
    Simulator * simulator = new Simulator();
    Model * model = new Model(simulator);
    model->load(this->_modelFilename);
    model->check();
    std::cout <<this->getScenarioName()<< ": My load is completed"<< std::endl;
    // Setting control values
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
                std::cout <<this->getScenarioName()<< ": set value "<< (*mod_it)->getName()<< std::endl;
                (*mod_it)->setValue((*scen_it)->second);
            }
        }
    }
    
    // Running simulation
    model->getSimulation()->start();

    model->check();
    // Acquiring response values
    for (
            std::list<std::string>::iterator scen_it = this->_selectedResponses->begin();
            scen_it != this->_selectedResponses->end();
            ++scen_it
            ) {
        std::cout << "my selectedRes: "<< *scen_it <<", but, the model doesn't seem to have responses in it: "<<std::endl;
        for (
                std::list<SimulationResponse*>::iterator mod_it = model->getResponses()->list()->begin();
                mod_it != model->getResponses()->list()->end();
                ++mod_it
                ) {
            std::cout << "n to entarndo aq ne scen_it: "<< (*scen_it) <<". mod_it :"<< (*mod_it)->getName()<<std::endl;
            if (!(*scen_it).compare((*mod_it)->getName())) {
                std::cout << "n to entarndo aq ne"<< (*scen_it) << std::endl;
                this->_responseValues->push_back(new std::pair<std::string, double>((*scen_it), (*mod_it)->getValue()));
            }
        }
    }

    model->clear();
    delete model;
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
    std::cout << "a'aight, Imma get the response value of "<< responseName << std::endl;
    for(std::list<std::pair<std::string, double>*>::iterator it = this->_responseValues->begin(); it != this->_responseValues->end(); ++it) {
        std::cout << "getResponseValue for, response name :" << (*it)->first <<std::endl;
        if (!(*it)->first.compare(responseName))
            return (*it)->second;
    }
    std::cout << "Oh noo, there's no value for this response, chief"<<std::endl;
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