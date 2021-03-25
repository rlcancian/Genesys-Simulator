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
#include <list>
#include "SimulationResponse.h"
#include "SimulationControl.h"

/*!
 * Represents a scenario where a specific model (defined my ModelFilename) will be simulated. To each scenario will be associated a set of SimulationControl and SimulationResponse, and their values are set to the scenario by the ProcessAnalyser.
 */
class SimulationScenario {
public:
	SimulationScenario();
	virtual ~SimulationScenario() = default;
public: // results
	bool startSimulation(std::string* errorMessage);
	std::list<std::pair<std::string, double>*>* getResponseValues() const; /*!< The final result of the simulationScenario */
	std::list<std::pair<std::string, double>*>* getControlValues() const;
	double getResponseValue(std::string responseName);
public: // 
	double getControlValue(std::string controlName);
	void setControlValue(std::string controlName, double value);
public: // gets
	void setModelFilename(std::string _modelFilename);
	std::string getModelFilename() const;
	void setScenarioName(std::string _name);
	std::string getScenarioName() const;
	std::list<SimulationResponse*>* getSelectedResponses() const; // access to the list to insert or remove responses
	std::list<SimulationControl*>* getSelectedControls() const; // access to the list to insert or remove controls
	void setScenarioDescription(std::string _scenarioDescription);
	std::string getScenarioDescription() const;
private:
        std::string _scenarioName;
	std::string _scenarioDescription;
	std::string _modelFilename;
	std::list<SimulationControl*>* _selectedControls = new std::list<SimulationControl*>(); /*!< a subset of SimulationControls available in the model */
	std::list<SimulationResponse*>* _selectedResponses = new std::list<SimulationResponse*>(); /*!< a subset of SimulationResponses available in the model */
	std::list<std::pair<std::string, double>*>* _controlValues; /*!< stored values of the parameters used in simulation <name of control, value set> */
	std::list<std::pair<std::string, double>*>* _responseValues; /*!< stored values of the results returned by simulation <name of response, value returned>*/
};

#endif /* SIMULATIONSCENARIO_H */

