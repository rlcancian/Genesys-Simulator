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
	_info = model->getInfos(); // why??
	_statsCountersSimulation->setSortFunc([](const ModelElement* a, const ModelElement * b) {
		return a->getId() < b->getId();
	});
	_simulationReporter = new Traits<SimulationReporter_if>::Implementation(this, model, this->_statsCountersSimulation);
}

bool ModelSimulation::_isReplicationEndCondition() {
	bool finish = _model->getFutureEvents()->size() == 0;
	finish |= _model->parseExpression(_info->getTerminatingCondition()) != 0.0;
	if (_model->getFutureEvents()->size() > 0 && !finish) {
		// replication length has not been achieve (sor far), but next event will happen after that, so it's just fine to set tnow as the replicationLength
		finish |= _model->getFutureEvents()->front()->getTime() > _info->getReplicationLength();
	}
	return finish;
}

void ModelSimulation::_traceReplicationEnded() {
	std::string causeTerminated = "";
	if (_model->getFutureEvents()->empty()) {
		causeTerminated = "event queue is empty";
	} else if (_stopRequested) {
		causeTerminated = "user requested to stop";
	} else if (_model->getFutureEvents()->front()->getTime() > _info->getReplicationLength()) {
		causeTerminated = "replication length " + std::to_string(_info->getReplicationLength()) + " was achieved";
	} else if (_model->parseExpression(_info->getTerminatingCondition())) {
		causeTerminated = "termination condition was achieved";
	} else causeTerminated = "unknown";
	std::string message = "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_info->getNumberOfReplications()) + " has finished with last event at time " + std::to_string(_simulatedTime) + " because " + causeTerminated;
	_model->getTracer()->trace(Util::TraceLevel::modelSimulationEvent, message);
}

/*!
 * Checks the model and if ok then initialize the simulation, execute repeatedly each replication and then show simulation statistics
 */
void ModelSimulation::start() {
	if (!_model->check()) {
		_model->getTracer()->trace(Util::TraceLevel::errorFatal, "Model check failed. Cannot start simulation.");
		return;
	}
	Util::SetIndent(0); //force indentation
	if (!_isPaused) {
		_initSimulation();
		_model->getOnEvents()->NotifySimulationStartHandlers(new SimulationEvent(0, nullptr));
		_currentReplicationNumber = 1;
		Util::IncIndent();
		_initReplication();
	} else {
		_model->getOnEvents()->NotifySimulationPausedStartHandlers(new SimulationEvent(0, nullptr));
	}
	_running = true;
	_isPaused = false;
	//std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio < 1, 1000000000 >> > replicationStartTime = std::chrono::high_resolution_clock::now();
	do {
		Util::SetIndent(1);
		_model->getOnEvents()->NotifyReplicationStartHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
		// main simulation loop
		Util::IncIndent();
		while (!_isReplicationEndCondition() && !_pauseRequested) {
			_stepSimulation();
		}
		Util::SetIndent(1); // force
		if (!_pauseRequested) {
			_replicationEnded();
			_currentReplicationNumber++;
			if (_currentReplicationNumber <= _info->getNumberOfReplications()) {
				_initReplication();
			}
		}
	} while (_currentReplicationNumber <= _info->getNumberOfReplications() && !_pauseRequested);
	if (!_pauseRequested) {
		_simulationReporter->showSimulationStatistics(); //_cStatsSimulation);
		Util::DecIndent();

		_model->getTracer()->trace(Util::TraceLevel::modelSimulationEvent, "Simulation of model \"" + _info->getName() + "\" has finished.\n");
		_model->getOnEvents()->NotifySimulationEndHandlers(new SimulationEvent(0, nullptr));
	} else {
		_pauseRequested = false;
		_isPaused = true;
	}
	_running = false;
}

void ModelSimulation::_replicationEnded() {
	_traceReplicationEnded();
	_model->getOnEvents()->NotifyReplicationEndHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
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
	tm->traceReport(_model->getParentSimulator()->name());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLicence());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLimits());
	// model infos
	tm->traceReport("Analyst Name: " + _info->getAnalystName());
	tm->traceReport("Project Title: " + _info->getProjectTitle());
	tm->traceReport("Number of Replications: " + std::to_string(_info->getNumberOfReplications()));
	tm->traceReport("Replication Length: " + std::to_string(_info->getReplicationLength()) + " " + Util::StrTimeUnit(_info->getReplicationLengthTimeUnit()));
	//tm->traceReport(Util::TraceLevel::simulation, "");
	// model controls and responses
	std::string controls;
	for (std::list<SimulationControl*>::iterator it = _model->getControls()->list()->begin(); it != _model->getControls()->list()->end(); it++) {
		controls += (*it)->getName() + "(" + (*it)->getType() + ")=" + std::to_string((*it)->getValue()) + ", ";
	}
	controls = controls.substr(0, controls.length() - 2);
	tm->traceReport("> Simulation controls: " + controls);
	std::string responses;
	for (std::list<SimulationResponse*>::iterator it = _model->getResponses()->list()->begin(); it != _model->getResponses()->list()->end(); it++) {
		responses += (*it)->getName() + "(" + (*it)->getType() + "), ";
	}
	responses = responses.substr(0, responses.length() - 2);
	tm->traceReport("> Simulation responses: " + responses);
	tm->traceReport("");
}

/*!
 * Initialize once for all replications
 */
void ModelSimulation::_initSimulation() {
	_showSimulationHeader();
	//model->tracer()->trace(Util::TraceLevel::modelSimulationEvent, "------------------------------");
	_model->getTracer()->trace(Util::TraceLevel::modelSimulationEvent, "");
	_model->getTracer()->trace(Util::TraceLevel::modelSimulationEvent, "Simulation of model \"" + _info->getName() + "\" is starting.");
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
}

void ModelSimulation::_initReplication() {
	TraceManager* tm = _model->getTracer();
	tm->trace(Util::TraceLevel::modelSimulationEvent, "");
	tm->trace(Util::TraceLevel::modelSimulationEvent, "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_info->getNumberOfReplications()) + " is starting.");

	_model->getFutureEvents()->clear();
	_simulatedTime = 0.0;
	_pauseRequested = false;

	//if (_currentReplicationNumber > 1) {
	// init all components between replications
	for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
		ModelElement::InitBetweenReplications((*it));
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
	double warmupTime = Util::TimeUnitConvert(_model->getInfos()->getWarmUpPeriodTimeUnit(), _model->getInfos()->getReplicationLengthTimeUnit());
	warmupTime *= _model->getInfos()->getWarmUpPeriod();
	if (warmupTime > 0.0 && _model->getSimulation()->getSimulatedTime() <= warmupTime && nextEvent->getTime() > warmupTime) {// warmuTime. Time to initStats
		_model->getTracer()->trace(Util::TraceLevel::modelInternal, "Warmup time reached. Statistics are being reseted.");
		_initStatistics();
	}
}

void ModelSimulation::_stepSimulation() {
	// process one single event
	Event* nextEvent;
	nextEvent = _model->getFutureEvents()->front();
	if (_model->getInfos()->getWarmUpPeriod() > 0.0)
		_checkWarmUpTime(nextEvent);
	if (nextEvent->getTime() <= _info->getReplicationLength()) {
		_model->getFutureEvents()->pop_front();
		_model->getOnEvents()->NotifyReplicationStepHandlers(new SimulationEvent(_currentReplicationNumber, nullptr));
		_processEvent(nextEvent);
	} else {
		this->_simulatedTime = _model->getInfos()->getReplicationLength(); ////nextEvent->getTime(); // just to advance time to beyond simulatedTime
	}
}

void ModelSimulation::_processEvent(Event* event) {
	//	_model->tracer()->traceSimulation(Util::TraceLevel::modelSimulationEvent, event->time(), event->entity(), event->component(), "");
	_model->getTracer()->trace(Util::TraceLevel::modelSimulationEvent, "Processing event=(" + event->show() + ")");
	_model->getTracer()->trace(Util::TraceLevel::modelSimulationInternal, "Current Entity: " + event->getEntity()->show());
	this->_currentEntity = event->getEntity();
	this->_currentComponent = event->getComponent();
	this->_currentInputNumber = event->getComponentInputNumber();
	assert(_simulatedTime <= event->getTime());
	_simulatedTime = event->getTime();
	_model->getOnEvents()->NotifyProcessEventHandlers(new SimulationEvent(_currentReplicationNumber, event));
	Util::IncIndent();
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
}

void ModelSimulation::restart() {
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
	return _running;
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

bool ModelSimulation::isPaused() const {
	return _isPaused;
}
