/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelSimulation.h
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 18:04
 */

#ifndef MODELSIMULATION_H
#define MODELSIMULATION_H

//#include <chrono>
#include "Event.h"
#include "Entity.h"
#include "ModelInfo.h"
#include "SimulationReporter_if.h"
//#include "Counter.h"
//namespace GenesysKernel {
	class Model;

	/*!
	 * The ModelSimulation controls the simulation of a model, alowing to start, pause, resume e stop a simulation, composed by 
	 * a set of replications.
	 */
	class ModelSimulation {
	public:
		ModelSimulation(Model* model);
		virtual ~ModelSimulation() = default;
	public: // simulation control
		void start(); ///< Starts a sequential execution of a simulation, ie, a set of replications of this model.
		void pause();
		void step(); ///< Executes the processing of a single event, the next one in the future events list.
		void stop();
		void restart();
	public: // gets and sets
		void setPauseOnEvent(bool _pauseOnEvent);
		bool isPauseOnEvent() const;
		void setStepByStep(bool _stepByStep);
		bool isStepByStep() const;
		void setInitializeStatistics(bool _initializeStatistics);
		bool isInitializeStatistics() const;
		void setInitializeSystem(bool _initializeSystem);
		bool isInitializeSystem() const;
		void setPauseOnReplication(bool _pauseBetweenReplications);
		bool isPauseOnReplication() const;
	public: // gets and sets ModelSubParts 
        void setReporter(SimulationReporter_if* _simulationReporter);
        SimulationReporter_if* getReporter() const;
	public: // only gets
		double getSimulatedTime() const; /*! The current time in the model being simulated, i.e., the instant when the current event was triggered */
		bool isRunning() const;
        bool isPaused() const;
		unsigned int getCurrentReplicationNumber() const;
		ModelComponent* getCurrentComponent() const;
		Entity* getCurrentEntity() const;
		unsigned int getCurrentInputNumber() const;
        void setShowReportsAfterReplication(bool showReportsAfterReplication);
        bool isShowReportsAfterReplication() const;
        void setShowReportsAfterSimulation(bool showReportsAfterSimulation);
        bool isShowReportsAfterSimulation() const;
		/*
		 * PRIVATE
		 */
	private: // simulation control
		void _initSimulation();
		void _initReplication(); ///< Clear the event list, restarts simulated time, initialize event list and statistics, request components to reinitialize
		void _initStatistics();
		void _checkWarmUpTime(Event* nextEvent);
		void _stepSimulation();
		void _replicationEnded();
		void _processEvent(Event* event);
	private:
		bool _isReplicationEndCondition();
		void _actualizeSimulationStatistics();
		void _showSimulationHeader();
		void _traceReplicationEnded();
	private:
		double _simulatedTime = 0.0;
		// \todo: list of double double _breakOnTimes;
		// \todo: list of modules _breakOnModules;
		bool _stepByStep = false;
		bool _pauseOnReplication = false;
		bool _pauseOnEvent = false;
		bool _initializeStatisticsBetweenReplications = true;
		bool _initializeSystem = true;
		bool _running = false;
		bool _isPaused = false;
		bool _pauseRequested = false;
		bool _stopRequested = false;
		bool _simulationIsInitiated = false;
		bool _replicationIsInitiaded = false;
		bool _showReportsAfterSimulation = true;
		bool _showReportsAfterReplication = true;
	private:
		Entity* _currentEntity;
		ModelComponent* _currentComponent;
		unsigned int _currentInputNumber;
		unsigned int _currentReplicationNumber;
		//std::chrono::steady_clock::time_point _replicationStartTime;
		//std::chrono::steady_clock::time_point _replicationEndTime;
	private:
		const std::string _cte_stCountSimulNamePrefix = ""; //Simul.";
		//std::list<ModelElement*>* _countersSimulation = new std::list<ModelElement*>();
	private:
		Model* _model;
		ModelInfo* _info;
		SimulationReporter_if* _simulationReporter;
		List<ModelElement*>* _statsCountersSimulation = new List<ModelElement*>();
	};
//namespace\\}
#endif /* MODELSIMULATION_H */

