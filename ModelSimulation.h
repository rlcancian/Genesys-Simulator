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
class ModelSimulation { // 202104 to be subjected to SimulationScenario
public:
	ModelSimulation(Model* model);
	virtual ~ModelSimulation() = default;
public:
	std::string show();
public: // simulation control
	void start(); ///< Starts a sequential execution of a simulation, ie, a set of replications of this model.
	void pause();
	void step(); ///< Executes the processing of a single event, the next one in the future events list.
	void stop();
public: // old modelInfos
	void setNumberOfReplications(unsigned int _numberOfReplications);
	unsigned int getNumberOfReplications() const;
	void setReplicationLength(double _replicationLength);
	double getReplicationLength() const;
	void setReplicationLengthTimeUnit(Util::TimeUnit _replicationLengthTimeUnit);
	Util::TimeUnit getReplicationLengthTimeUnit() const;
    void setReplicationReportBaseTimeUnit(Util::TimeUnit _replicationReportBaseTimeUnit);
    Util::TimeUnit getReplicationBaseTimeUnit() const;
	void setWarmUpPeriod(double _warmUpPeriod);
	double getWarmUpPeriod() const;
	void setWarmUpPeriodTimeUnit(Util::TimeUnit _warmUpPeriodTimeUnit);
	Util::TimeUnit getWarmUpPeriodTimeUnit() const;
	void setTerminatingCondition(std::string _terminatingCondition);
	std::string getTerminatingCondition() const;
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
	List<double>* getBreakpointsOnTime() const;
	List<Entity*>* getBreakpointsOnEntity() const;
	List<ModelComponent*>* getBreakpointsOnComponent() const;
public:
	void loadInstance(std::map<std::string, std::string>* fields);
	std::map<std::string, std::string>* saveInstance();
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
	bool _checkBreakpointAt(Event* event);
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
	bool _isRunning = false;
	bool _isPaused = false;
	bool _pauseRequested = false;
	bool _stopRequested = false;
	bool _simulationIsInitiated = false;
	bool _replicationIsInitiaded = false;
	bool _showReportsAfterSimulation = true;
	bool _showReportsAfterReplication = true;
private:

	const struct DEFAULT_VALUES {
		unsigned int numberOfReplications = 1;
		double replicationLength = 3600.0; // by default, 3600 s = 1.0 h
		Util::TimeUnit replicationLengthTimeUnit = Util::TimeUnit::second;
		Util::TimeUnit replicationBeseTimeUnit = Util::TimeUnit::second;
		Util::TimeUnit replicationReportBaseTimeUnit = Util::TimeUnit::second;
		double warmUpPeriod = replicationLength * 0.10; // 0.0;
		Util::TimeUnit warmUpPeriodTimeUnit = Util::TimeUnit::second;
		std::string terminatingCondition = "";
	} DEFAULT;
	unsigned int _numberOfReplications = DEFAULT.numberOfReplications;
	double _replicationLength = DEFAULT.replicationLength;
	Util::TimeUnit _replicationLengthTimeUnit = DEFAULT.replicationLengthTimeUnit;
	Util::TimeUnit _replicationBaseTimeUnit = DEFAULT.replicationReportBaseTimeUnit;
	double _warmUpPeriod = DEFAULT.warmUpPeriod;
	Util::TimeUnit _warmUpPeriodTimeUnit = DEFAULT.warmUpPeriodTimeUnit;
	std::string _terminatingCondition = DEFAULT.terminatingCondition;
	bool _hasChanged = false;
	double _replicationTimeScaleFactorToBase; // a scale that converts ReplicationLenghtTimeUnit to ReplicationBaseTimeUnit. Future events are in "times" of unit ReplicationBaseTimeUnit 
private:
	Entity* _currentEntity = nullptr;
	ModelComponent* _currentComponent = nullptr;
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
	List<double>* _breakpointsOnTime = new List<double>();
	List<ModelComponent*>* _breakpointsOnComponent = new List<ModelComponent*>();
	List<Entity*>* _breakpointsOnEntity = new List<Entity*>();
	double _justTriggeredBreakpointsOnTime = 0.0;
	ModelComponent* _justTriggeredBreakpointsOnComponent = nullptr;
	Entity* _justTriggeredBreakpointsOnEntity = nullptr;
};
//namespace\\}
#endif /* MODELSIMULATION_H */

