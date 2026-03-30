/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModelSimulation.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 7 de Novembro de 2018, 18:04
 */

#ifndef MODELSIMULATION_H
#define MODELSIMULATION_H

#include <chrono>
#include <memory>
#include "Event.h"
#include "Entity.h"
#include "ModelInfo.h"
#include "SimulationReporter_if.h"
#include "OnEventManager.h"
//#include "Counter.h"
//namespace GenesysKernel {

//#include "Model.h" // for friend functions
class Model;

/*!
 * The ModelSimulation controls the simulation of a model, alowing to start, pause, resume e stop a simulation, composed by
 * a set of replications.
 */
/**
 * @brief Controls the mature, currently functional simulation-execution layer of a model.
 *
 * Historically, this class has been the original and operational experiment
 * mechanism in GenESyS. It already supports the core execution workflow based on
 * replications, replication length, warm-up configuration and event processing.
 *
 * Later experiment-oriented classes such as SimulationExperiment and
 * SimulationScenario were introduced as an attempt to move richer
 * design-of-experiments concerns into the kernel. Those newer classes are still
 * under development, while ModelSimulation remains the stable execution core.
 */
class ModelSimulation { // 202104 to be subjected to SimulationScenario
public:
	ModelSimulation(Model* model);
	virtual ~ModelSimulation() = default;
public:
	std::string show();
// TODO(genesys|experiment-layer|architecture): Keep ModelSimulation as the stable
// execution core while the higher-level experiment abstractions mature.
// Any migration of responsibilities to SimulationExperiment/SimulationScenario
// should preserve the already functional replication-based workflow here.
public: // simulation control
	/// @todo Revisit how this class should interact with the unfinished higher-level experiment layer.
	void start(); //!< Starts a sequential execution of a simulation, ie, a set of replications of this model.
	void pause(); //!<
	void step(); //!< Executes the processing of a single event, the next one in the future events list.
	void stop(); //!<
public: // old modelInfos
	void setNumberOfReplications(unsigned int _numberOfReplications);
	unsigned int getNumberOfReplications() const;
	//void setReplicationLength(double _replicationLength);
	void setReplicationLength(double _replicationLength, Util::TimeUnit _replicationLengthTimeUnit = Util::TimeUnit::unknown);
	double getReplicationLength() const;
	void setReplicationLengthTimeUnit(Util::TimeUnit _replicationLengthTimeUnit);
	Util::TimeUnit getReplicationLengthTimeUnit() const;
	void setReplicationReportBaseTimeUnit(Util::TimeUnit _replicationReportBaseTimeUnit);
	Util::TimeUnit getReplicationBaseTimeUnit() const;
	//void setWarmUpPeriod(double warmUpPeriod);
	void setWarmUpPeriod(double warmUpPeriod, Util::TimeUnit warmUpPeriodTimeUnit = Util::TimeUnit::unknown);
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
	void setShowReportsAfterReplication(bool showReportsAfterReplication);
	bool isShowReportsAfterReplication() const;
	void setShowReportsAfterSimulation(bool showReportsAfterSimulation);
	bool isShowReportsAfterSimulation() const;
	List<double>* getBreakpointsOnTime() const;
	List<Entity*>* getBreakpointsOnEntity() const;
	List<ModelComponent*>* getBreakpointsOnComponent() const;
public:
	void loadInstance(PersistenceRecord *fields);
	void saveInstance(PersistenceRecord *fields, bool saveDefaults);
	Event* getCurrentEvent() const;
	void setShowSimulationResposesInReport(bool _showSimulationResposesInReport);
	bool isShowSimulationResposesInReport() const;
	void setShowSimulationControlsInReport(bool _showSimulationControlsInReport);
	bool isShowSimulationControlsInReport() const;
	/*
	 * PRIVATE
	 */
private: // simulation control
	void _initSimulation(); //!<
	void _initReplication(); //!< Clear the event list, restarts simulated time, initialize event list and statistics, request components to reinitialize
	void _clearStatistics(); //!<
	void _checkWarmUpTime(Event* nextEvent); //!<
	void _stepSimulation(); //!<
	void _replicationEnded(); //!<
	void _simulationEnded(); //!<
private:
	void _dispatchEvent(Event* event);
	bool _checkBreakpointAt(Event* event); //!<
	bool _isReplicationEndCondition(); //!<
	void _actualizeSimulationStatistics(); //!<
	void _showSimulationHeader(); //!<
	void _traceReplicationEnded(); //!<
private:
	std::unique_ptr<SimulationEvent> _createSimulationEvent(void* thiscustomObject = nullptr); //!<
	//friend Entity* Model::createEntity(std::string name, bool insertIntoModel); //@TODO: make it work (only friend functions, not the entire class)
	//friend void Model::removeEntity(Entity* entity);
	//friend void Model::sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay, unsigned int componentinputPortNumber);
	friend class Model;
private:
	double _simulatedTime = 0.0;
	double _justTriggeredBreakpointsOnTime = 0.0;
	// @TODO: list of double double _breakOnTimes;
	// @TODO: list of modules _breakOnModules;
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
	bool _hasChanged = false;
private:

	const struct DEFAULT_VALUES {
		const unsigned int numberOfReplications = 1;
		const double replicationLength = 60.0; // by default, 60s = 1min
		const Util::TimeUnit replicationLengthTimeUnit = Util::TimeUnit::second;
		const Util::TimeUnit replicationBeseTimeUnit = Util::TimeUnit::second;
		const Util::TimeUnit replicationReportBaseTimeUnit = Util::TimeUnit::second;
		const double warmUpPeriod = 0.0;
		const Util::TimeUnit warmUpPeriodTimeUnit = Util::TimeUnit::second;
		const std::string terminatingCondition = "";
		const bool initializeStatisticsBetweenReplications = true;
		const bool initializeSystem = true;
		const bool showReportsAfterSimulation = true;
		const bool showReportsAfterReplication = true;
		const bool showSimulationControlsInReport = true;
	// TODO(genesys|model-simulation|naming): Review legacy spelling such as
	// _showSimulationResposesInReport and rename carefully once all callers are mapped.
		const bool showSimulationResposesInReport = false;
	} DEFAULT;
	unsigned int _numberOfReplications = DEFAULT.numberOfReplications;
	double _replicationLength = DEFAULT.replicationLength;
	Util::TimeUnit _replicationLengthTimeUnit = DEFAULT.replicationLengthTimeUnit;
	Util::TimeUnit _replicationBaseTimeUnit = DEFAULT.replicationReportBaseTimeUnit;
	double _warmUpPeriod = DEFAULT.warmUpPeriod;
	Util::TimeUnit _warmUpPeriodTimeUnit = DEFAULT.warmUpPeriodTimeUnit;
	std::string _terminatingCondition = DEFAULT.terminatingCondition;
	bool _showReportsAfterSimulation = DEFAULT.showReportsAfterSimulation;
	bool _showReportsAfterReplication = DEFAULT.showReportsAfterReplication;
	bool _showSimulationControlsInReport = DEFAULT.showSimulationControlsInReport;
	bool _showSimulationResposesInReport = DEFAULT.showSimulationResposesInReport;
	//
	double _replicationTimeScaleFactorToBase; // a scale that converts ReplicationLenghtTimeUnit to ReplicationBaseTimeUnit. Future events are in "times" of unit ReplicationBaseTimeUnit
	std::chrono::system_clock::time_point _startRealSimulationTimeSimulation;
	std::chrono::system_clock::time_point _startRealSimulationTimeReplication;
private:
	Event* _currentEvent = nullptr;
	unsigned int _currentReplicationNumber;
private:
	const std::string _cte_stCountSimulNamePrefix = ""; //Simul.";
private:
	Model* _model;
	ModelInfo* _info;
	SimulationReporter_if* _simulationReporter;
	//@TODO Change List below to a MAP, associating every CstatOuCounter in the replication to the equivalent in the simulation
	List<ModelDataDefinition*>* _cstatsAndCountersSimulation = new List<ModelDataDefinition*>();
	std::map<ModelDataDefinition*, ModelDataDefinition*>* _cstatsAndCountersMapSimulation = new std::map<ModelDataDefinition*, ModelDataDefinition*>();
	List<double>* _breakpointsOnTime = new List<double>();
	List<ModelComponent*>* _breakpointsOnComponent = new List<ModelComponent*>();
	List<Entity*>* _breakpointsOnEntity = new List<Entity*>();
	ModelComponent* _justTriggeredBreakpointsOnComponent = nullptr;
	Entity* _justTriggeredBreakpointsOnEntity = nullptr;
};
//namespace\\}
#endif /* MODELSIMULATION_H */
