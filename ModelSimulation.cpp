/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelSimulation.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 7 de Novembro de 2018, 18:04
 */

#include "ModelSimulation.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include "Model.h"
#include "Simulator.h"
#include "SourceModelComponent.h"
#include "StatisticsCollector.h"
#include "Counter.h"
#include "Traits.h"
#include "SimulationControl.h"
#include "ComponentManager.h"

//using namespace GenesysKernel;

ModelSimulation::ModelSimulation(Model* model) {
	_model = model;
	_info = model->getInfos();
	_statsCountersSimulation->setSortFunc([](const ModelElement* a, const ModelElement * b) {
		return a->getId() < b->getId();
	});
	_simulationReporter = new Traits<SimulationReporter_if>::Implementation(this, model, this->_statsCountersSimulation);
}

std::string ModelSimulation::show() {
	return "numberOfReplications=" + std::to_string(_numberOfReplications) +
			",replicationLength=" + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(this->_replicationLengthTimeUnit) +
			",terminatingCondition=\"" + this->_terminatingCondition + "\"" +
			",warmupTime=" + std::to_string(this->_warmUpPeriod) + " " + Util::StrTimeUnitLong(this->_warmUpPeriodTimeUnit);
}

bool ModelSimulation::_isReplicationEndCondition() {
	bool finish = _model->getFutureEvents()->size() == 0;
	finish |= _model->parseExpression(_terminatingCondition) != 0.0;
	if (_model->getFutureEvents()->size() > 0 && !finish) {
		// replication length has not been achieve (sor far), but next event will happen after that, so it's just fine to set tnow as the replicationLength
		finish |= _model->getFutureEvents()->front()->getTime() > _replicationLength * _replicationTimeScaleFactorToBase;
	}
	return finish;
}

void ModelSimulation::_traceReplicationEnded() {
	std::string causeTerminated = "";
	if (_model->getFutureEvents()->empty()) {
		causeTerminated = "event queue is empty";
	} else if (_stopRequested) {
		causeTerminated = "user requested to stop";
	} else if (_model->getFutureEvents()->front()->getTime() > _replicationLength) {
		causeTerminated = "replication length " + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(_replicationLengthTimeUnit) + " was achieved";
	} else if (_model->parseExpression(_terminatingCondition)) {
		causeTerminated = "termination condition was achieved";
	} else causeTerminated = "unknown";
	std::string message = "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_numberOfReplications) + " has finished with last event at time " + std::to_string(_simulatedTime) + " " + Util::StrTimeUnitLong(_replicationBaseTimeUnit) + " because " + causeTerminated;
	_model->getTracer()->trace(Util::TraceLevel::L5_arrival, message);
}

/*!
 * Checks the model and if ok then initialize the simulation, execute repeatedly each replication and then show simulation statistics
 */
void ModelSimulation::start() {
	if (!_isPaused) { // begin of a new simulation
		Util::SetIndent(0); //force indentation
		if (!_model->check()) {
			_model->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Model check failed. Cannot start simulation.");
			return;
		}
		_initSimulation();
		_model->getOnEvents()->NotifySimulationStartHandlers(new SimulationEvent(0, nullptr));
		_currentReplicationNumber = 1;
		//Util::IncIndent();
		//_initReplication();
	} else { // continue after a pause
		_model->getTracer()->trace("Replication resumed", Util::TraceLevel::L5_arrival);
		_model->getOnEvents()->NotifySimulationPausedStartHandlers(new SimulationEvent(0, nullptr));
	}
	_isRunning = true;
	_isPaused = false;
	//std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio < 1, 1000000000 >> > replicationStartTime = std::chrono::high_resolution_clock::now();
	do {
		Util::SetIndent(1);
		_model->getOnEvents()->NotifyReplicationStartHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
		_initReplication();
		// main simulation loop
		Util::IncIndent();
		while (!_isReplicationEndCondition() && !_pauseRequested) {
			_stepSimulation();
		}
		if (!_pauseRequested) {
			Util::SetIndent(1); // force
			_replicationEnded();
			_currentReplicationNumber++;
			if (_currentReplicationNumber <= _numberOfReplications) {
				//_initReplication();
				if (_pauseOnReplication) {
					_model->getTracer()->trace("End of replication. Simulation is paused.", Util::TraceLevel::L6_internal);
					_pauseRequested = true;
				}
			}
		}
	} while (_currentReplicationNumber <= _numberOfReplications && !_pauseRequested);
	// all replications done (or paused during execution)
	if (!_pauseRequested) { // done
		if (this->_showReportsAfterSimulation)
			_simulationReporter->showSimulationStatistics(); //_cStatsSimulation);
		Util::DecIndent();
		// clear current event
		_currentEntity = nullptr;
		_currentComponent = nullptr;
		//
		_model->getTracer()->trace(Util::TraceLevel::L5_arrival, "Simulation of model \"" + _info->getName() + "\" has finished.\n");
		_model->getOnEvents()->NotifySimulationEndHandlers(new SimulationEvent(0, nullptr));
	} else {
		_pauseRequested = false;
		_isPaused = true;
	}
	_isRunning = false;
}

void ModelSimulation::_replicationEnded() {
	_traceReplicationEnded();
	_model->getOnEvents()->NotifyReplicationEndHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
	if (this->_showReportsAfterReplication)
		_simulationReporter->showReplicationStatistics();
	//_simulationReporter->showSimulationResponses();
	_actualizeSimulationStatistics();
}

void ModelSimulation::_actualizeSimulationStatistics() {
	//\todo: should not be only CSTAT and COUNTER, but any element that generateReportInformation
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();

	StatisticsCollector *sc, *scSim;
	//ModelElement* me;
	List<ModelElement*>* cstats = _model->getElements()->getElementList(Util::TypeOf<StatisticsCollector>());
	for (std::list<ModelElement*>::iterator itMod = cstats->list()->begin(); itMod != cstats->list()->end(); itMod++) {
		sc = dynamic_cast<StatisticsCollector*> ((*itMod));
		scSim = nullptr;
		for (std::list<ModelElement*>::iterator itSim = _statsCountersSimulation->list()->begin(); itSim != _statsCountersSimulation->list()->end(); itSim++) {
			if ((*itSim)->getClassname() == UtilTypeOfStatisticsCollector) {
				if ((*itSim)->getName() == _cte_stCountSimulNamePrefix + sc->getName() && dynamic_cast<StatisticsCollector*> (*itSim)->getParent() == sc->getParent()) {
					// found
					scSim = dynamic_cast<StatisticsCollector*> (*itSim);
					break;
				}
			}
		}
		//scSim = dynamic_cast<StatisticsCollector*> (*(this->_statsCountersSimulation->find((*it))));
		if (scSim == nullptr) {
			// this is a cstat created during the last replication
			//	    scSim = new StatisticsCollector(_model->elements(), sc->name(), sc->getParent());
			_statsCountersSimulation->insert(scSim);
		}
		assert(scSim != nullptr);
		scSim->getStatistics()->getCollector()->addValue(sc->getStatistics()->average());
	}
	Counter *cnt; //, *cntSim;
	List<ModelElement*>* counters = _model->getElements()->getElementList(Util::TypeOf<Counter>());
	for (std::list<ModelElement*>::iterator itMod = counters->list()->begin(); itMod != counters->list()->end(); itMod++) {
		cnt = dynamic_cast<Counter*> ((*itMod));
		//cntSim = nullptr;
		scSim = nullptr;
		for (std::list<ModelElement*>::iterator itSim = _statsCountersSimulation->list()->begin(); itSim != _statsCountersSimulation->list()->end(); itSim++) {
			if ((*itSim)->getClassname() == UtilTypeOfStatisticsCollector) {
				//_model->getTraceManager()->trace(Util::TraceLevel::simulation, (*itSim)->getName() + " == "+_cte_stCountSimulNamePrefix + cnt->getName());
				if ((*itSim)->getName() == _cte_stCountSimulNamePrefix + cnt->getName() && dynamic_cast<StatisticsCollector*> (*itSim)->getParent() == cnt->getParent()) {
					// found
					scSim = dynamic_cast<StatisticsCollector*> (*itSim);
					break;
				}
			}
			/*
			if ((*itSim)->getTypename() == UtilTypeOfCounter) {
			if ((*itSim)->getName() == _cte_stCountSimulNamePrefix + cnt->getName() && dynamic_cast<Counter*> (*itSim)->getParent() == cnt->getParent()) {
				// found
				cntSim = dynamic_cast<Counter*> (*itSim);
				break;
			}
			}
			 */
		}
		/*
		assert(cntSim != nullptr);
		cntSim->incCountValue(cnt->getCountValue());
		 */
		assert(scSim != nullptr);
		scSim->getStatistics()->getCollector()->addValue(cnt->getCountValue());
	}
}

void ModelSimulation::_showSimulationHeader() {
	TraceManager* tm = _model->getTracer();
	tm->traceReport("\n-----------------------------------------------------");
	// simulator infos
	tm->traceReport(_model->getParentSimulator()->getName());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLicence());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLimits());
	// model infos
	tm->traceReport("Analyst Name: " + _info->getAnalystName());
	tm->traceReport("Project Title: " + _info->getProjectTitle());
	tm->traceReport("Number of Replications: " + std::to_string(_numberOfReplications));
	tm->traceReport("Replication Length: " + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(_replicationLengthTimeUnit));
	tm->traceReport("Base TimeUnit: " + Util::StrTimeUnitLong(_replicationBaseTimeUnit));
	//tm->traceReport(Util::TraceLevel::simulation, "");
	// model controls and responses
	std::string controls;
	for (SimulationControl* control : * _model->getControls()->list()) {
		controls += control->getName() + "(" + control->getType() + ")=" + std::to_string(control->getValue()) + ", ";
	}
	controls = controls.substr(0, controls.length() - 2);
	tm->traceReport("> Simulation controls: " + controls);
	std::string responses;
	//for (std::list<SimulationResponse*>::iterator it = _model->getResponses()->list()->begin(); it != _model->getResponses()->list()->end(); it++) {
	//	responses += (*it)->getName() + "(" + (*it)->getType() + "), ";
	//}
	//responses = responses.substr(0, responses.length() - 2);
	//tm->traceReport("> Simulation responses: " + responses);
	tm->traceReport("");
}

/*!
 * Initialize once for all replications
 */
void ModelSimulation::_initSimulation() {
	_showSimulationHeader();
	//model->tracer()->trace(Util::TraceLevel::L5_arrival, "------------------------------");
	_model->getTracer()->trace(Util::TraceLevel::L5_arrival, "");
	_model->getTracer()->trace(Util::TraceLevel::L5_arrival, "Simulation of model \"" + _info->getName() + "\" is starting.");
	// defines the time scale factor to adjust replicatonLength to replicationBaseTime
	_replicationTimeScaleFactorToBase = Util::TimeUnitConvert(this->_replicationLengthTimeUnit, this->_replicationBaseTimeUnit);
	// copy all CStats and Counters (used in a replication) to CStats and counters for the whole simulation
	// \todo: Should not be CStats and Counters, but any element that generates report importation
	this->_statsCountersSimulation->clear();
	StatisticsCollector* cstat;
	List<ModelElement*>* cstats = _model->getElements()->getElementList(Util::TypeOf<StatisticsCollector>());
	for (std::list<ModelElement*>::iterator it = cstats->list()->begin(); it != cstats->list()->end(); it++) {
		cstat = dynamic_cast<StatisticsCollector*> ((*it));
		// this new CSat should NOT be inserted into the model
		StatisticsCollector* newCStatSimul = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix + cstat->getName(), cstat->getParent(), false);
		//_model->elements()->remove(Util::TypeOf<StatisticsCollector>(), newCStatSimul); // remove from model, since it is automatically inserted by the constructor
		this->_statsCountersSimulation->insert(newCStatSimul);
	}
	// copy all Counters (used in a replication) to Counters for the whole simulation
	// \todo: Counters in replication should be converted into CStats in simulation. Each value counted in a replication should be added in a CStat for Stats.
	Counter* counter;
	List<ModelElement*>* counters = _model->getElements()->getElementList(Util::TypeOf<Counter>());
	for (std::list<ModelElement*>::iterator it = counters->list()->begin(); it != counters->list()->end(); it++) {
		counter = dynamic_cast<Counter*> ((*it));
		// adding a counter
		/*
		Counter* newCountSimul = new Counter(_cte_stCountSimulNamePrefix + counter->getName(), counter->getParent());
		this->_statsCountersSimulation->insert(newCountSimul);
		 */
		// addin a cstat (to stat the counts)
		StatisticsCollector* newCStatSimul = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix + counter->getName(), counter->getParent(), false);
		this->_statsCountersSimulation->insert(newCStatSimul);
	}
	this->_simulationIsInitiated = true; // \todo Check the uses of _simulationIsInitiated and when it should be set to false
}

void ModelSimulation::_initReplication() {
	TraceManager* tm = _model->getTracer();
	tm->trace(Util::TraceLevel::L5_arrival, "");
	tm->trace(Util::TraceLevel::L5_arrival, "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_numberOfReplications) + " is starting.");

	_model->getFutureEvents()->clear();
	_simulatedTime = 0.0;
	_pauseRequested = false;

	//if (_currentReplicationNumber > 1) {
	// init all components between replications
	Util::IncIndent();
	tm->trace(Util::TraceLevel::L7_detailed, "Initing Replication");
	for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
		ModelComponent::InitBetweenReplications((*it));
	}
	// init all elements between replications
	ModelElement* element;
	std::string elementType;
	//std::string* errorMessage = new std::string();
	std::list<std::string>* elementTypes = _model->getElements()->getElementClassnames();
	for (std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
		elementType = (*typeIt);
		List<ModelElement*>* elements = _model->getElements()->getElementList(elementType);
		for (std::list<ModelElement*>::iterator it = elements->list()->begin(); it != elements->list()->end(); it++) {
			element = (*it);
			ModelElement::InitBetweenReplications(element);
		}
	}
	Util::DecIndent();
	//}
	Util::ResetIdOfType(Util::TypeOf<Entity>());
	Util::ResetIdOfType(Util::TypeOf<Event>());

	// insert first creation events
	SourceModelComponent *source;
	Entity *newEntity;
	Event *newEvent;
	double creationTime;
	unsigned int numToCreate;
	//std::list<ModelComponent*>* list = _model->getComponents()->list();
	for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
		source = dynamic_cast<SourceModelComponent*> (*it);
		if (source != nullptr) {
			creationTime = source->getFirstCreation();
			numToCreate = source->getEntitiesPerCreation();
			for (unsigned int i = 1; i <= numToCreate; i++) {
				newEntity = new Entity(_model);
				newEntity->setEntityType(source->getEntityType());
				newEvent = new Event(creationTime, newEntity, (*it));
				_model->getFutureEvents()->insert(newEvent);
			}
			source->setEntitiesCreated(numToCreate);
		}
	}

	if (this->_initializeStatisticsBetweenReplications) {
		_initStatistics();
	}
	this->_replicationIsInitiaded = true; // \todo Check the uses of _replicationIsInitiaded and when it should be set to false
}

void ModelSimulation::_initStatistics() {
	StatisticsCollector* cstat;
	List<ModelElement*>* list = _model->getElements()->getElementList(Util::TypeOf<StatisticsCollector>());
	for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		cstat = (StatisticsCollector*) (*it);
		cstat->getStatistics()->getCollector()->clear();
	}
	Counter* counter;
	list = _model->getElements()->getElementList(Util::TypeOf<Counter>());
	for (std::list<ModelElement*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		counter = (Counter*) (*it);
		counter->clear();
	}
}

void ModelSimulation::_checkWarmUpTime(Event* nextEvent) {
	double warmupTime = Util::TimeUnitConvert(_warmUpPeriodTimeUnit, _replicationBaseTimeUnit);
	warmupTime *= _warmUpPeriod;
	if (warmupTime > 0.0 && _model->getSimulation()->getSimulatedTime() <= warmupTime && nextEvent->getTime() > warmupTime) {// warmuTime. Time to initStats
		_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Warmup time reached. Statistics are being reseted.");
		_initStatistics();
	}
}

void ModelSimulation::_stepSimulation() {
	// process one single event
	Event* nextEvent;
	nextEvent = _model->getFutureEvents()->front();
	if (_warmUpPeriod > 0.0)
		_checkWarmUpTime(nextEvent);
	if (nextEvent->getTime() <= _replicationLength * _replicationTimeScaleFactorToBase) {
		if (_checkBreakpointAt(nextEvent)) {
			this->_pauseRequested = true;
		} else {
			_model->getFutureEvents()->pop_front();
			_model->getOnEvents()->NotifyReplicationStepHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
			_processEvent(nextEvent);
		}
	} else {
		this->_simulatedTime = _replicationLength; ////nextEvent->getTime(); // just to advance time to beyond simulatedTime
	}
}

bool ModelSimulation::_checkBreakpointAt(Event* event) {
	bool res = false;
	SimulationEvent* se = new SimulationEvent(_currentReplicationNumber, event);
	if (_breakpointsOnComponent->find(event->getComponent()) != _breakpointsOnComponent->list()->end()) {
		if (_justTriggeredBreakpointsOnComponent == event->getComponent()) {
			_justTriggeredBreakpointsOnComponent = nullptr;
		} else {
			_justTriggeredBreakpointsOnComponent = event->getComponent();
			_model->getOnEvents()->NotifyBreakpointHandlers(se);
			_model->getTracer()->trace("Breakpoint found at component '" + event->getComponent()->getName() + "'. Replication is paused.", Util::TraceLevel::L5_arrival);

			res = true;
		}
	}
	if (_breakpointsOnEntity->find(event->getEntity()) != _breakpointsOnEntity->list()->end()) {
		if (_justTriggeredBreakpointsOnEntity == event->getEntity()) {
			_justTriggeredBreakpointsOnEntity = nullptr;
		} else {
			_justTriggeredBreakpointsOnEntity = event->getEntity();
			_model->getOnEvents()->NotifyBreakpointHandlers(se);
			_model->getTracer()->trace("Breakpoint found at entity '" + event->getEntity()->getName() + "'. Replication is paused.", Util::TraceLevel::L5_arrival);
			res = true;
		}
	}
	double time;
	for (std::list<double>::iterator it = _breakpointsOnTime->list()->begin(); it != _breakpointsOnTime->list()->end(); it++) {
		time = (*it);
		if (_simulatedTime < time && event->getTime() >= time) {
			if (_justTriggeredBreakpointsOnTime == time) { // just trrigered this breakpoint
				_justTriggeredBreakpointsOnTime = 0.0;
			} else {
				_justTriggeredBreakpointsOnTime = time;
				_model->getOnEvents()->NotifyBreakpointHandlers(se);
				_model->getTracer()->trace("Breakpoint found at time '" + std::to_string(event->getTime()) + "'. Replication is paused.", Util::TraceLevel::L5_arrival);
				return true;
			}
		}
	}
	return res;
}

void ModelSimulation::_processEvent(Event* event) {
	//	_model->tracer()->traceSimulation(Util::TraceLevel::L5_arrival, event->time(), event->entity(), event->component(), "");
	_model->getTracer()->trace(Util::TraceLevel::L5_arrival, "Event {" + event->show() + "}");
	Util::IncIndent();
	_model->getTracer()->trace(Util::TraceLevel::L6_internal, "Entity: " + event->getEntity()->show());
	this->_currentEntity = event->getEntity();
	this->_currentComponent = event->getComponent();
	this->_currentInputNumber = event->getComponentInputNumber();
	assert(_simulatedTime <= event->getTime());
	_simulatedTime = event->getTime();
	_model->getOnEvents()->NotifyProcessEventHandlers(new SimulationEvent(_currentReplicationNumber, event));
	try {
		//event->getComponent()->Execute(event->getEntity(), event->getComponent()); // Execute is static
		ModelComponent::Execute(event->getEntity(), event->getComponent(), event->getComponentInputNumber());
	} catch (std::exception *e) {
		_model->getTracer()->traceError(*e, "Error on processing event (" + event->show() + ")");
	}
	Util::DecIndent();
}

void ModelSimulation::pause() {
}

void ModelSimulation::step() {
	if (_simulationIsInitiated && _replicationIsInitiaded) {
		if (!_isReplicationEndCondition()) {
			try {
				this->_stepSimulation();
			} catch (std::exception *e) {
				_model->getTracer()->traceError((*e), "Error on simulation step");
			}
		}
	}
}

void ModelSimulation::stop() {
	this->_isPaused = false;
	this->_isRunning = false;
	this->_replicationIsInitiaded = false;
	this->_simulationIsInitiated = false;
}

void ModelSimulation::setPauseOnEvent(bool _pauseOnEvent) {
	this->_pauseOnEvent = _pauseOnEvent;
}

bool ModelSimulation::isPauseOnEvent() const {
	return _pauseOnEvent;
}

void ModelSimulation::setInitializeStatistics(bool _initializeStatistics) {
	this->_initializeStatisticsBetweenReplications = _initializeStatistics;
}

bool ModelSimulation::isInitializeStatistics() const {
	return _initializeStatisticsBetweenReplications;
}

void ModelSimulation::setInitializeSystem(bool _initializeSystem) {
	this->_initializeSystem = _initializeSystem;
}

bool ModelSimulation::isInitializeSystem() const {
	return _initializeSystem;
}

void ModelSimulation::setStepByStep(bool _stepByStep) {
	this->_stepByStep = _stepByStep;
}

bool ModelSimulation::isStepByStep() const {
	return _stepByStep;
}

void ModelSimulation::setPauseOnReplication(bool _pauseOnReplication) {
	this->_pauseOnReplication = _pauseOnReplication;
}

bool ModelSimulation::isPauseOnReplication() const {
	return _pauseOnReplication;
}

double ModelSimulation::getSimulatedTime() const {
	return _simulatedTime;
}

bool ModelSimulation::isRunning() const {
	return _isRunning;
}

unsigned int ModelSimulation::getCurrentReplicationNumber() const {
	return _currentReplicationNumber;
}

ModelComponent* ModelSimulation::getCurrentComponent() const {
	return _currentComponent;
}

Entity* ModelSimulation::getCurrentEntity() const {
	return _currentEntity;
}

void ModelSimulation::setReporter(SimulationReporter_if* _simulationReporter) {
	this->_simulationReporter = _simulationReporter;
}

SimulationReporter_if* ModelSimulation::getReporter() const {
	return _simulationReporter;
}

unsigned int ModelSimulation::getCurrentInputNumber() const {
	return _currentInputNumber;
}

void ModelSimulation::setShowReportsAfterReplication(bool showReportsAfterReplication) {
	this->_showReportsAfterReplication = showReportsAfterReplication;
}

bool ModelSimulation::isShowReportsAfterReplication() const {
	return _showReportsAfterReplication;
}

void ModelSimulation::setShowReportsAfterSimulation(bool showReportsAfterSimulation) {
	this->_showReportsAfterSimulation = showReportsAfterSimulation;
}

bool ModelSimulation::isShowReportsAfterSimulation() const {
	return _showReportsAfterSimulation;
}

List<double>* ModelSimulation::getBreakpointsOnTime() const {
	return _breakpointsOnTime;
}

List<Entity*>* ModelSimulation::getBreakpointsOnEntity() const {
	return _breakpointsOnEntity;
}

List<ModelComponent*>* ModelSimulation::getBreakpointsOnComponent() const {
	return _breakpointsOnComponent;
}

bool ModelSimulation::isPaused() const {
	return _isPaused;
}

void ModelSimulation::setNumberOfReplications(unsigned int _numberOfReplications) {
	this->_numberOfReplications = _numberOfReplications;
	_hasChanged = true;
}

unsigned int ModelSimulation::getNumberOfReplications() const {
	return _numberOfReplications;
}

void ModelSimulation::setReplicationLength(double _replicationLength) {
	this->_replicationLength = _replicationLength;
	_hasChanged = true;
}

double ModelSimulation::getReplicationLength() const {
	return _replicationLength;
}

void ModelSimulation::setReplicationLengthTimeUnit(Util::TimeUnit _replicationLengthTimeUnit) {
	this->_replicationLengthTimeUnit = _replicationLengthTimeUnit;
	_hasChanged = true;
}

Util::TimeUnit ModelSimulation::getReplicationLengthTimeUnit() const {
	return _replicationLengthTimeUnit;
}

void ModelSimulation::setWarmUpPeriod(double _warmUpPeriod) {
	this->_warmUpPeriod = _warmUpPeriod;
	_hasChanged = true;
}

double ModelSimulation::getWarmUpPeriod() const {
	return _warmUpPeriod;
}

void ModelSimulation::setWarmUpPeriodTimeUnit(Util::TimeUnit _warmUpPeriodTimeUnit) {
	this->_warmUpPeriodTimeUnit = _warmUpPeriodTimeUnit;
	_hasChanged = true;
}

Util::TimeUnit ModelSimulation::getWarmUpPeriodTimeUnit() const {
	return _warmUpPeriodTimeUnit;
}

void ModelSimulation::setTerminatingCondition(std::string _terminatingCondition) {
	this->_terminatingCondition = _terminatingCondition;
	_hasChanged = true;
}

std::string ModelSimulation::getTerminatingCondition() const {
	return _terminatingCondition;
}

void ModelSimulation::loadInstance(std::map<std::string, std::string>* fields) {
	this->_numberOfReplications = LoadField(fields, "numberOfReplications", DEFAULT.numberOfReplications);
	this->_replicationLength = LoadField(fields, "replicationLength", DEFAULT.replicationLength);
	this->_replicationLengthTimeUnit = LoadField(fields, "replicationLengthTimeUnit", DEFAULT.replicationLengthTimeUnit);
	this->_replicationBaseTimeUnit = LoadField(fields, "replicationBaseTimeUnit", DEFAULT.replicationBeseTimeUnit);
	this->_terminatingCondition = LoadField(fields, "terminatingCondition", DEFAULT.terminatingCondition);
	this->_warmUpPeriod = LoadField(fields, "warmUpTime", DEFAULT.warmUpPeriod);
	this->_warmUpPeriodTimeUnit = LoadField(fields, "warmUpTimeTimeUnit", DEFAULT.warmUpPeriodTimeUnit);
	_hasChanged = false;
}

// \todo:!: implement check method (to check things like terminating condition)

std::map<std::string, std::string>* ModelSimulation::saveInstance() {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	SaveField(fields, "typename", "ModelSimulation");
	SaveField(fields, "name", "fixed experiment name in ModelSimulation"); //getName());
	SaveField(fields, "numberOfReplications", _numberOfReplications, DEFAULT.numberOfReplications);
	SaveField(fields, "replicationLength", _replicationLength, DEFAULT.replicationLength);
	SaveField(fields, "replicationLengthTimeUnit", _replicationLengthTimeUnit, DEFAULT.replicationLengthTimeUnit);
	SaveField(fields, "replicationBaseTimeUnit", _replicationBaseTimeUnit, DEFAULT.replicationBeseTimeUnit);
	SaveField(fields, "terminatingCondition", _terminatingCondition, DEFAULT.terminatingCondition);
	SaveField(fields, "warmUpTime", _warmUpPeriod, DEFAULT.warmUpPeriod);
	SaveField(fields, "warmUpTimeTimeUnit", _warmUpPeriodTimeUnit, DEFAULT.warmUpPeriodTimeUnit);
	_hasChanged = false;
	return fields;
}

void ModelSimulation::setReplicationReportBaseTimeUnit(Util::TimeUnit _replicationReportBaseTimeUnit) {
	this->_replicationBaseTimeUnit = _replicationReportBaseTimeUnit;
}

Util::TimeUnit ModelSimulation::getReplicationBaseTimeUnit() const {
	return _replicationBaseTimeUnit;
}