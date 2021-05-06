/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationScenario.h
 * Author: rafael.luiz.cancian
 *
 * Created on 10 de Outubro de 2018, 18:21
 */

#ifndef SIMULATIONSCENARIO_H
#define SIMULATIONSCENARIO_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Model.h"
#include "SimulationResponse.h"
#include "SimulationControl.h"

/*!
 * Represents a scenario where a specific model (defined my ModelFilename) will be simulated. To each scenario will be associated a set of SimulationControl and SimulationResponse, and their values are set to the scenario by the ProcessAnalyser.
 */
class SimulationScenario {
public:
    SimulationScenario();
    virtual ~SimulationScenario();
public: // results
    bool startSimulation(Simulator * simulator, std::string* errorMessage);
    std::unordered_map<std::string, double>* getResponseValues() const; /*!< The final result of the simulationScenario */
    double getResponseValue(std::string responseName);
public: // gets and sets
    void setModelFilename(std::string _modelFilename);
    std::string getModelFilename() const;
    void setScenarioName(std::string _name);
    std::string getScenarioName() const;
    void setScenarioDescription(std::string _scenarioDescription);
    std::string getScenarioDescription() const;
    std::unordered_map<std::string, double>* getSelectedControls() const; // access to the list to insert or remove controls
    double getControlValue(std::string controlName);
    void selectResponse(std::string responseName);
    void setControlValue(std::string controlName, double value);
    std::unordered_set<std::string>* getSelectedResponses() const; // access to the list to insert or remove responses
private:
    std::string _scenarioName;
    std::string _scenarioDescription;
    std::string _modelFilename;
    std::unordered_map<std::string, double>* _selectedControls; /*!< a subset of SimulationControls available in the model (chosen by user)*/
    std::unordered_set<std::string>* _selectedResponses; /*!< a subset of SimulationResponses available in the model (chosen by user) */
    std::unordered_map<std::string, double>* _responseValues; /*!< stored values of the results returned by simulation <name of response, value returned>*/
};

#endif /* SIMULATIONSCENARIO_H */

