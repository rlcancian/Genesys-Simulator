/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ExperimentManager.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
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

/**
 * @brief Manages higher-level simulation experiments.
 *
 * Historically, GenESyS initially supported only the basic experiment controls
 * available through ModelSimulation, especially the number of replications and
 * the replication length.
 *
 * This class represents a later attempt to move more advanced experiment design
 * concerns into the simulation kernel, including higher-level experiment sets,
 * scenario management and future Process Analyzer-like workflows.
 *
 * This layer is still under development and should be treated as an incomplete
 * experimental abstraction on top of the already functional ModelSimulation
 * mechanism.
 */
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
	// Release owned experiment instances and container storage during simulator shutdown.
	virtual ~ExperimentManager();

public:
	SimulationExperiment* newSimulationExperiment();
	void insert(SimulationExperiment* experiment);
	void remove(SimulationExperiment* experiment);
	void setCurrent(SimulationExperiment* experiment);
	/// @todo Implement persistence once SimulationExperiment serialization is defined.
	bool saveSimulationExperiment(std::string filename);
	/// @todo Implement loading once SimulationExperiment deserialization and scenario execution are defined.
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
