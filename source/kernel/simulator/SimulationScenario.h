/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SimulationScenario.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 10 de Outubro de 2018, 18:21
 */

#ifndef SIMULATIONSCENARIO_H
#define SIMULATIONSCENARIO_H

#include <string>
#include <list>

class Simulator;

/*!
 * Represents a scenario where a specific model (defined my ModelFilename) will be simulated. To each scenario will be associated a set of SimulationControl and SimulationResponse, and their values are set to the scenario by the ProcessAnalyser.
 */
/**
 * @brief Represents a candidate simulation scenario within the unfinished experiment layer.
 *
 * The original GenESyS execution flow relied on ModelSimulation for the basic
 * experimental controls already supported by the kernel.
 *
 * SimulationScenario was introduced later to support richer scenario-based
 * experimentation, potentially including alternative models, modified controls
 * and future Process Analyzer-like workflows.
 *
 * This class is still under development and should currently be treated as an
 * evolving kernel-side abstraction for experiment scenarios.
 */
class SimulationScenario {
public:
	SimulationScenario();
	virtual ~SimulationScenario();
public: // results
	bool startSimulation(Simulator *sim, std::string& errorMessage);
	std::list<std::pair<std::string, double>*>* getResponseValues() const; /*!< The final result of the simulationScenario */
	std::list<std::pair<std::string, double>*>* getControlValues() const;
	double getResponseValue(const std::string& responseName);
public: // gets and sets
	void setModelFilename(std::string _modelFilename);
	std::string getModelFilename() const;
	void setScenarioName(std::string _name);
	std::string getScenarioName() const;
	void setScenarioDescription(std::string _scenarioDescription);
	std::string getScenarioDescription() const;
	std::list<std::string>* getSelectedControls() const; // access to the list to insert or remove controls
	double getControlValue(const std::string& controlName);
	std::list<std::string>* getSelectedResponses() const; // access to the list to insert or remove responses
	// @ToDo: (importante): Clarify ownership and replacement semantics for selected controls.
	// This setter currently allocates a new list and replaces the stored pointer.
	void setSelectedControls(std::list<std::string>* selectedControls);
	void setControl(std::string name, double value) const;
private:
	std::string _scenarioName;
	std::string _scenarioDescription;
	std::string _modelFilename;
	std::list<std::string>* _selectedControls = new std::list<std::string>(); /*!< a subset of SimulationControls available in the model (chosen by user)*/
	std::list<std::string>* _selectedResponses = new std::list<std::string>(); /*!< a subset of SimulationResponses available in the model (chosen by user) */
	std::list<std::pair<std::string, double>*>* _controlValues = new std::list<std::pair<std::string, double>*>();
	// @ToDo: (importante): Define initialization and ownership rules for response values.
	std::list<std::pair<std::string, double>*>* _responseValues = new std::list<std::pair<std::string, double>*>(); /*!< stored values of the results returned by simulation <name of response, value returned>*/
};

#endif /* SIMULATIONSCENARIO_H */
