/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ExperimentManager.h
 * Author: rlcancian
 *
 * Created on 4 de maio de 2021, 11:43
 */

#ifndef EXPERIMENTMANAGER_H
#define EXPERIMENTMANAGER_H

//#include "Simulator.h"
#include "Persistence.h"
//#include "SimulationControl.h"
//#include "SimulationResponse.h"
#include "SimulationExperiment.h"
#include "../util/List.h"

class Simulator;

class ExperimentManager {// : PersistentObject_base {
public:

	/*	class ModelSimulationInsOuts {
		public:
			ModelSimulationInsOuts() = default;
			struct ControlResponseInfos {
				std::string type;
				std::string parent;
				std::string name;
				double defaultValue;
			};

			List<ControlResponseInfos*>* getResponsesInfos() const {
				return _responsesInfos;
			}

			std::string getModelfilename() const {
				return _experimentfilename;
			}

			List<ControlResponseInfos*>* getControlInfos() const {
				return _controlInfos;
			}
		private:
			std::string _modelfilename;
			List<ControlResponseInfos*>* _responsesInfos = new List<ControlResponseInfos*>();
			List<ControlResponseInfos*>* _controlInfos = new List<ControlResponseInfos*>();
		}; */

public:
	ExperimentManager(Simulator* simulator);

public:
	SimulationExperiment* newSimulationExperiment();
	void insert(SimulationExperiment* experiment);
	void remove(SimulationExperiment* experiment);
	void setCurrent(SimulationExperiment* experiment);
	bool saveSimulationExperiment(std::string filename);
	bool loadSimulationExperiment(std::string filename);
	unsigned int size();
public:
	SimulationExperiment* front();
	SimulationExperiment* current();
	SimulationExperiment* next();
	//SimulationExperiment* end();
public:
	List<SimulationExperiment*>* getExperiments() const;
private:
	Simulator* _simulator;
	SimulationExperiment* _currentSimulationExperiment;
	List<SimulationExperiment*>* _experiments = new List<SimulationExperiment*>();

};

#endif /* EXPERIMENTMANAGER_H */

