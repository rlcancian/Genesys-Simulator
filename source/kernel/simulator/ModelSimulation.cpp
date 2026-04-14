/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */;

/*
 * File:   ModelSimulation.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 7 de Novembro de 2018, 18:04
 */

#include "ModelSimulation.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include "Model.h"
#include "Simulator.h"
#include "StatisticsCollector.h"
#include "Counter.h"
#include "ComponentManager.h"
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

ModelSimulation::ModelSimulation(Model* model) {
	_model = model;
	_info = model->getInfos();
	_cstatsAndCountersSimulation->setSortFunc([](const ModelDataDefinition* a, const ModelDataDefinition*b){
		return a->getId()<b->getId();
	});
	_simulationReporter = new TraitsKernel<SimulationReporter_if>::Implementation(this, model, this->_cstatsAndCountersSimulation);
	// Create and register the base simulation controls while tracking their ownership locally.
	SimulationControl* replicationBaseTimeUnit = new SimulationControlTimeUnit(
			 std::bind(&ModelSimulation::getReplicationBaseTimeUnit, this),
			 std::bind(&ModelSimulation::setReplicationReportBaseTimeUnit, this, std::placeholders::_1),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "ReplicationBaseTimeUnit");
	_model->getControls()->insert(replicationBaseTimeUnit);
	_ownedControls->insert(replicationBaseTimeUnit);
	SimulationControl* replicationLengthTimeUnit = new SimulationControlTimeUnit(
			 std::bind(&ModelSimulation::getReplicationLengthTimeUnit, this),
			 std::bind(&ModelSimulation::setReplicationLengthTimeUnit, this, std::placeholders::_1),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "ReplicationLengthTimeUnit");
	_model->getControls()->insert(replicationLengthTimeUnit);
	_ownedControls->insert(replicationLengthTimeUnit);
	SimulationControl* warmUpPeriodTimeUnit = new SimulationControlTimeUnit(
			 std::bind(&ModelSimulation::getWarmUpPeriodTimeUnit, this),
			 std::bind(&ModelSimulation::setWarmUpPeriodTimeUnit, this, std::placeholders::_1),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "WarmUpPeriodTimeUnit");
	_model->getControls()->insert(warmUpPeriodTimeUnit);
	_ownedControls->insert(warmUpPeriodTimeUnit);
	SimulationControl* warmUpPeriod = new SimulationControlDouble(
			 std::bind(&ModelSimulation::getWarmUpPeriod, this),
			 std::bind(&ModelSimulation::setWarmUpPeriod, this, std::placeholders::_1, Util::TimeUnit::unknown),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "WarmUpPeriod");
	_model->getControls()->insert(warmUpPeriod);
	_ownedControls->insert(warmUpPeriod);
	SimulationControl* numberOfReplications = new SimulationControlUInt(
			 std::bind(&ModelSimulation::getNumberOfReplications, this),
			 std::bind(&ModelSimulation::setNumberOfReplications, this, std::placeholders::_1),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "NumberOfReplications");
	_model->getControls()->insert(numberOfReplications);
	_ownedControls->insert(numberOfReplications);
	SimulationControl* terminatingCondition = new SimulationControlString(
			 std::bind(&ModelSimulation::getTerminatingCondition, this),
			 std::bind(&ModelSimulation::setTerminatingCondition, this, std::placeholders::_1),
			 Util::TypeOf<ModelSimulation>(), "ModelSimulation", "TerminatingCondition");
	_model->getControls()->insert(terminatingCondition);
	_ownedControls->insert(terminatingCondition);
}

ModelSimulation::~ModelSimulation() {
	// Remove and destroy only the base controls explicitly created by this simulation object.
	if (_ownedControls != nullptr) {
		for (SimulationControl* control : *_ownedControls->list()) {
			_model->getControls()->remove(control);
			delete control;
		}
		delete _ownedControls;
		_ownedControls = nullptr;
	}
	if (_cstatsAndCountersSimulation != nullptr) {
		for (ModelDataDefinition* data : *_cstatsAndCountersSimulation->list()) {
			delete data;
		}
		delete _cstatsAndCountersSimulation;
		_cstatsAndCountersSimulation = nullptr;
	}
	delete _cstatsAndCountersMapSimulation;
	_cstatsAndCountersMapSimulation = nullptr;
	delete _breakpointsOnTime;
	_breakpointsOnTime = nullptr;
	delete _breakpointsOnComponent;
	_breakpointsOnComponent = nullptr;
	delete _breakpointsOnEntity;
	_breakpointsOnEntity = nullptr;
	if (_ownsSimulationReporter) {
		delete _simulationReporter;
	}
	_simulationReporter = nullptr;
}

std::string ModelSimulation::show() {
	return "numberOfReplications="+std::to_string(_numberOfReplications)+
			",replicationLength="+std::to_string(_replicationLength)+" "+Util::StrTimeUnitLong(this->_replicationLengthTimeUnit)+
			",terminatingCondition=\""+this->_terminatingCondition+"\""+
			",warmupTime="+std::to_string(this->_warmUpPeriod)+" "+Util::StrTimeUnitLong(this->_warmUpPeriodTimeUnit);
}

bool ModelSimulation::_isReplicationEndCondition() {
	bool finish = _model->getFutureEvents()->size()==0;
	if (!finish) {
		finish = _model->getFutureEvents()->front()->getTime()>_replicationLength * _replicationTimeScaleFactorToBase;
		if (!finish&&_terminatingCondition!="") {
			finish = _model->parseExpression(_terminatingCondition)!=0.0;
		}
	}
	return finish;
}

void ModelSimulation::_traceReplicationEnded() {
	std::string causeTerminated = "";
	if (_model->getFutureEvents()->empty()) {
		causeTerminated = "event queue is empty";
	} else if (_stopRequested) {
		causeTerminated = "user requested to stop";
	} else if (_model->getFutureEvents()->front()->getTime()>_replicationLength * _replicationTimeScaleFactorToBase) {
		causeTerminated = "replication length "+std::to_string(_replicationLength)+" "+Util::StrTimeUnitLong(_replicationLengthTimeUnit)+" was achieved";
	} else if (_model->parseExpression(_terminatingCondition)) {
		causeTerminated = "termination condition was achieved";
	} else
		causeTerminated = "it just did it :-|. Sorry";
	std::chrono::duration<double> duration = std::chrono::system_clock::now()-this->_startRealSimulationTimeReplication;
	std::string message = "Replication "+std::to_string(_currentReplicationNumber)+" of "+std::to_string(_numberOfReplications)+" has finished with last event at time "+std::to_string(_simulatedTime)+" "+Util::StrTimeUnitLong(_replicationBaseTimeUnit)+" because "+causeTerminated+"; Elapsed time "+std::to_string(duration.count())+" seconds.";
	_model->getTracer()->traceSimulation(this, TraceManager::Level::L2_results, message);
}

std::unique_ptr<SimulationEvent> ModelSimulation::_createSimulationEvent(void* thiscustomObject) {
	auto se = std::unique_ptr<SimulationEvent>(new SimulationEvent());
	//	se->currentComponent = _currentComponent;
	//	se->currentEntity = _currentEntity;
	se->currentEvent = _currentEvent;
	//	se->currentinputPortNumber = _currentinputPortNumber;
	se->currentReplicationNumber = _currentReplicationNumber;
	se->customObject = thiscustomObject;
    se->_isPaused = _isPaused;
    se->_isRunning = _isRunning;
	se->pauseRequested = _pauseRequested;
	se->simulatedTime = _simulatedTime;
	se->stopRequested = _stopRequested;
	return se;
}

/*!
 * Checks the model and if ok then initialize the simulation, execute repeatedly each replication and then show simulation statistics
 */
void ModelSimulation::start() {
	if (!_simulationIsInitiated) { // begin of a new simulation
		Util::SetIndent(0); //force indentation
		if (!_model->check()) {
			_model->getTracer()->traceError("Model check failed. Cannot start simulation.");
			return;
		}
		_initSimulation();
		_isRunning = true; // set it before notifying handlers
		auto simulationEvent = _createSimulationEvent();
		_model->getOnEventManager()->NotifySimulationStartHandlers(simulationEvent.get());
	}
	_isRunning = true;
	if (_isPaused) { // continue after a pause
		_model->getTracer()->trace("Replication resumed", TraceManager::Level::L3_errorRecover);
		_isPaused = false; // set it before notifying handlers
		auto simulationEvent = _createSimulationEvent();
		_model->getOnEventManager()->NotifySimulationResumeHandlers(simulationEvent.get());
	}
	bool replicationEnded;
	do {
		if (!_replicationIsInitiaded) {
			Util::SetIndent(1);
			_initReplication();
			auto simulationEvent = _createSimulationEvent();
			_model->getOnEventManager()->NotifyReplicationStartHandlers(simulationEvent.get());
			_model->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "Running Replication");
		}
		replicationEnded = _isReplicationEndCondition();
		while (!replicationEnded) { // this is the main simulation loop
			_stepSimulation();
			replicationEnded = _isReplicationEndCondition();
			if (_pauseRequested||_stopRequested) { //check this only after _stepSimulation() and not on loop entering conditin
				break;
			}
		};
		if (replicationEnded) {
			Util::SetIndent(1); // force
			_replicationEnded();
			_currentReplicationNumber++;
			if (_currentReplicationNumber<=_numberOfReplications) {
				if (_pauseOnReplication) {
					_model->getTracer()->trace("End of replication. Simulation is paused.", TraceManager::Level::L7_internal);
					_pauseRequested = true;
				}
			} else {
				_pauseRequested = false;
			}
		}
	} while (_currentReplicationNumber<=_numberOfReplications && !(_pauseRequested||_stopRequested));
	// all replications done (or paused during execution)
	_isRunning = false;
	if (!_pauseRequested) { // done or stopped
		_stopRequested = false;
		_simulationEnded();
	} else { // paused
		_model->getTracer()->trace("Replication paused", TraceManager::Level::L3_errorRecover);
		_pauseRequested = false; // set them before notifying handlers
		_isPaused = true;
		auto simulationEvent = _createSimulationEvent();
		_model->getOnEventManager()->NotifySimulationPausedHandlers(simulationEvent.get());
	}
}


void ModelSimulation::_simulationEnded() {
	_simulationIsInitiated = false;
	std::chrono::duration<double> duration = std::chrono::system_clock::now()-this->_startRealSimulationTimeSimulation;
	Util::DecIndent();
	_model->getTracer()->traceSimulation(this, "Simulation of model \""+_info->getName()+"\" has finished. Elapsed time "+std::to_string(duration.count())+" seconds.", TraceManager::Level::L5_event);
	auto simulationEvent = _createSimulationEvent();
	_model->getOnEventManager()->NotifySimulationEndHandlers(simulationEvent.get());
	if (this->_showReportsAfterSimulation)
		_simulationReporter->showSimulationStatistics(); //_cStatsSimulation);
	// clear current event
	//_currentEntity = nullptr;
	//_currentComponent = nullptr;
	_currentEvent = nullptr;
	//
}

void ModelSimulation::_replicationEnded() {
	_traceReplicationEnded();
	auto simulationEvent = _createSimulationEvent();
	_model->getOnEventManager()->NotifyReplicationEndHandlers(simulationEvent.get());
	if (this->_showReportsAfterReplication)
		_simulationReporter->showReplicationStatistics();
	//_simulationReporter->showSimulationResponses();
	_actualizeSimulationStatistics();
	_replicationIsInitiaded = false;
}

void ModelSimulation::_actualizeSimulationStatistics() {
	// @ToDo: (importante): should not be only CSTAT and COUNTER, but any modeldatum that generateReportInformation
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();

	StatisticsCollector *cstatModel, *cstatSimulation;
	// runs over all StatisticCollectors in the model and create the equivalent for the entire simulation
	List<ModelDataDefinition*>* cstats = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());
	for (ModelDataDefinition* data : *cstats->list()) {
		cstatModel = dynamic_cast<StatisticsCollector*> (data); // ((*itMod));
		cstatSimulation = nullptr;
		//for (std::list<ModelDataDefinition*>::iterator itSim = _statsCountersSimulation->list()->begin(); itSim != _statsCountersSimulation->list()->end(); itSim++) {
		for (ModelDataDefinition* datasim : *_cstatsAndCountersSimulation->list()) {
			//if ((*itSim)->getClassname() == UtilTypeOfStatisticsCollector) {
			if (datasim->getClassname()==UtilTypeOfStatisticsCollector) {
				//if ((*itSim)->getName() == _cte_stCountSimulNamePrefix + sc->getName() && dynamic_cast<StatisticsCollector*> (*itSim)->getParent() == sc->getParent()) { // found
				if (datasim->getName()==_cte_stCountSimulNamePrefix+cstatModel->getName()) {
					if (dynamic_cast<StatisticsCollector*> (datasim)->getParent()==cstatModel->getParent()) { // found
						cstatSimulation = dynamic_cast<StatisticsCollector*> (datasim); // (*itSim);
						break;
					}
				}
			}
		}
		if (cstatSimulation==nullptr) {
			// this is a new cstat created during the last replication and didn't existed in simulation before
			cstatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix+cstatModel->getName(), cstatModel->getParent(), false);
			_cstatsAndCountersSimulation->insert(cstatSimulation);
		}
		assert(cstatSimulation!=nullptr);
		// actualize simulation cstat statistics by collecting the new value from the model/replication stat
		cstatSimulation->getStatistics()->getCollector()->addValue(cstatModel->getStatistics()->average());
	}
	// runs over all Counters in the model and create the equivalent for the entire simulation
	Counter *counterModel; //, *cntSim;
	List<ModelDataDefinition*>* counters = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
	for (ModelDataDefinition* countData : *counters->list()) {
		counterModel = dynamic_cast<Counter*> (countData);
		cstatSimulation = nullptr;
		for (ModelDataDefinition* dataSim : *_cstatsAndCountersSimulation->list()) {
			if (dataSim->getClassname()==UtilTypeOfStatisticsCollector) {
				if (dataSim->getName()==_cte_stCountSimulNamePrefix+counterModel->getName()) {
					if (dynamic_cast<StatisticsCollector*> (dataSim)->getParent()==counterModel->getParent()) {
						cstatSimulation = dynamic_cast<StatisticsCollector*> (dataSim);
						break;
					}
				}
			}
		}
		assert(cstatSimulation!=nullptr);
		// actualize simulation cstat statistics by collecting the new value from the model/replication stat
		cstatSimulation->getStatistics()->getCollector()->addValue(counterModel->getCountValue());
	}
}

void ModelSimulation::_showSimulationHeader() {
	TraceManager* tm = _model->getTracer();
	//tm->traceReport("\n-----------------------------------------------------");
	// simulator infos
	tm->traceReport(_model->getParentSimulator()->getName());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLicence());
	tm->traceReport(_model->getParentSimulator()->getLicenceManager()->showLimits());
	// model infos
	tm->traceReport("Analyst Name: "+_info->getAnalystName());
	tm->traceReport("Project Title: "+_info->getProjectTitle());
	tm->traceReport("Number of Replications: "+std::to_string(_numberOfReplications));
	tm->traceReport("Replication Length: "+std::to_string(_replicationLength)+" "+Util::StrTimeUnitLong(_replicationLengthTimeUnit));
	tm->traceReport("Replication/Report Base TimeUnit: "+Util::StrTimeUnitLong(_replicationBaseTimeUnit));
	//tm->traceReport(TraceManager::Level::simulation, "");
//    // model controls and responses
//    std::string controls;
//    for (/*PropertyBase**/SimulationControl* control : * _model->getControls()->list()) {
//        // @ToDo: (importante): IMPORTANT CONTROLS AND RESPONSES MUST WORK NO MATTER THE PROPERTIES PProperties
//        controls += control->getName()+"("+control->getClassname()+")="+control->getValue()+", ";
//    }
//    controls = controls.substr(0, controls.length()-2);
//    tm->traceReport("> Simulation controls: "+controls);
	std::string responses;
	for (SimulationResponse* pg : *_model->getResponses()->list()) {
		responses += pg->getName()+"("+pg->getClassname()+"), ";
	}
	responses = responses.substr(0, responses.length()-2);
	if (this->_showSimulationResposesInReport) {
        //if constexpr (TraitsKernel<SimulationReporter_if>::showSimulationResponses) {
		tm->traceReport("> Simulation responses: "+responses);
	}
	tm->traceReport("");
}

/*!
 * Initialize once for all replications
 */
void ModelSimulation::_initSimulation() {
	_startRealSimulationTimeSimulation = std::chrono::system_clock::now();
	_showSimulationHeader();
	_model->getTracer()->trace(TraceManager::Level::L5_event, "");
	_model->getTracer()->traceSimulation(this, TraceManager::Level::L5_event, "Simulation of model \""+_info->getName()+"\" is starting.");
	// defines the time scale factor to adjust replicatonLength to replicationBaseTime
	_replicationTimeScaleFactorToBase = Util::TimeUnitConvert(this->_replicationLengthTimeUnit, this->_replicationBaseTimeUnit);
	// copy all CStats and Counters (used in a replication) to CStats and counters for the whole simulation
	// @ToDo: (importante): Should not be CStats and Counters, but any modeldatum that generates report importation
	this->_cstatsAndCountersSimulation->clear();
	StatisticsCollector* cstat;
    List<ModelDataDefinition*>* simulationStatistics = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());
    for (ModelDataDefinition* cstatData : *simulationStatistics->list()) {
		cstat = dynamic_cast<StatisticsCollector*> (cstatData);
		// this new CSat should NOT be inserted into the model (so the false as last argument)
		StatisticsCollector* newCStatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix+cstat->getName(), cstat->getParent(), false);
		this->_cstatsAndCountersSimulation->insert(newCStatSimulation);
	}
	// copy all Counters (used in a replication) to Counters for the whole simulation
	Counter* counter;
    List<ModelDataDefinition*>* simulationCounters = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
    for (ModelDataDefinition* counterData : *simulationCounters->list()) {
		counter = dynamic_cast<Counter*> (counterData);
		/* // we do NOT add a counter in the simulation. We add a CStat that collect statistics about the Counter
		Counter* newCountSimul = new Counter(_cte_stCountSimulNamePrefix + counter->getName(), counter->getParent());
		this->_statsCountersSimulation->insert(newCountSimul);
		 */
		// addin a cstat (to stat the counts)
		StatisticsCollector* newCStatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix+counter->getName(), counter->getParent(), false);
		this->_cstatsAndCountersSimulation->insert(newCStatSimulation);
	}
	_simulationIsInitiated = true;
	_replicationIsInitiaded = false;
	_currentReplicationNumber = 1;
}

void ModelSimulation::_initReplication() {
	_startRealSimulationTimeReplication = std::chrono::system_clock::now();
	TraceManager* tm = _model->getTracer();
	tm->traceSimulation(this, TraceManager::Level::L5_event, "");
	tm->traceSimulation(this, TraceManager::Level::L2_results, "Replication "+std::to_string(_currentReplicationNumber)+" of "+std::to_string(_numberOfReplications)+" is starting.");
	// Destroys pending events before resetting replication state to avoid leaking queued heap events.
	while (!_model->getFutureEvents()->empty()) {
		Event* event = _model->getFutureEvents()->front();
		_model->getFutureEvents()->pop_front();
		delete event;
	}
	// Destroys transient entities before clearing per-replication runtime data.
	List<ModelDataDefinition*>* entities = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Entity>());
	while (!entities->empty()) {
		ModelDataDefinition* data = entities->front();
		Entity* entity = dynamic_cast<Entity*>(data);
		if (entity == nullptr) {
			entities->pop_front();
			continue;
		}
		// Delegates entity release to Model because Entity destruction is restricted to model ownership paths.
		_model->removeEntity(entity);
	}
	_simulatedTime = 0.0;
	// init all components between replications
	Util::IncIndent();
	tm->traceSimulation(this, TraceManager::Level::L8_detailed, "Initing Replication");
	Util::IncIndent();
	{
		Util::ResetIdOfType(Util::TypeOf<Entity>());
		Util::ResetIdOfType(Util::TypeOf<Event>());
		for (auto it = _model->getComponentManager()->begin(); it != _model->getComponentManager()->end(); ++it) {
			ModelComponent::InitBetweenReplications(*it);
		}
		// init all elements between replications
		// Iterate over a value snapshot of class names so replication init does not depend on manual deletes.
		std::list<std::string> elementTypes = _model->getDataManager()->getDataDefinitionClassnames();
		for (std::string elementType : elementTypes) {//std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
			List<ModelDataDefinition*>* elements = _model->getDataManager()->getDataDefinitionList(elementType);
			for (ModelDataDefinition* modeldatum : *elements->list()) {//std::list<ModelDataDefinition*>::iterator it = elements->list()->begin(); it != elements->list()->end(); it++) {
				ModelDataDefinition::InitBetweenReplications(modeldatum);
			}
		}
		//}
		if (this->_initializeStatisticsBetweenReplications) {
			_clearStatistics();
		}
	}
	Util::DecIndent();
	this->_replicationIsInitiaded = true;
}

void ModelSimulation::_clearStatistics() {
	// @ToDo: (pequena alteração): create a OnClearStatistics event handler
	StatisticsCollector* cstat;
	List<ModelDataDefinition*>* list = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());
	for (ModelDataDefinition* modelData : *list->list()) {
		cstat = static_cast<StatisticsCollector*>(modelData);
		cstat->getStatistics()->getCollector()->clear();
	}
	Counter* counter;
	list = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
	for (ModelDataDefinition* modelData : *list->list()) {
		counter = static_cast<Counter*>(modelData);
		counter->clear();
	}
}

void ModelSimulation::_checkWarmUpTime(Event* nextEvent) {
	double warmupTime = Util::TimeUnitConvert(_warmUpPeriodTimeUnit, _replicationBaseTimeUnit);
	warmupTime *= _warmUpPeriod;
	if (warmupTime>0.0&&_model->getSimulation()->getSimulatedTime()<=warmupTime&&nextEvent->getTime()>warmupTime) {// warmuTime. Time to initStats
		_model->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Warmup time reached. Statistics are being reseted.");
		_clearStatistics();
	}
}

void ModelSimulation::_stepSimulation() {
	// "onReplicationStep" event is triggered before taking the event from the calendar, and
	// "onProcessEvent" is triggered after the event is removed and turned into the current one, but before it is processed, and
	// "onAfterProcessEvent" is triggered after the event is processed
	auto simulationEvent = _createSimulationEvent();
	_model->getOnEventManager()->NotifyReplicationStepHandlers(simulationEvent.get());
	Event* nextEvent = _model->getFutureEvents()->front();
	if (_warmUpPeriod>0.0)
		_checkWarmUpTime(nextEvent);
	if (nextEvent->getTime()<=_replicationLength*_replicationTimeScaleFactorToBase) {
		if (_checkBreakpointAt(nextEvent)) {
			// Keeps the event in the queue when a breakpoint pauses execution before processing.
			this->_pauseRequested = true;
		} else {
			// Removes the event from the queue only when it will actually be processed.
			_model->getFutureEvents()->pop_front();
			if (nextEvent->getTime()>_simulatedTime)
				_model->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, "");
			_model->getTracer()->traceSimulation(this, TraceManager::Level::L5_event, "Event {"+nextEvent->show()+"}");
			Util::IncIndent();
			this->_currentEvent = nextEvent;
			//assert(_simulatedTime <= event->getTime()); // _simulatedTime only goes forward (futureEvents is chronologically sorted
			if (nextEvent->getTime()>=_simulatedTime) { // the philosophycal approach taken is: if the next event is in the past, lets just assume it's happening rigth now...
				_simulatedTime = nextEvent->getTime();
			}
			auto processEvent = _createSimulationEvent();
			_model->getOnEventManager()->NotifyProcessEventHandlers(processEvent.get());
			try {
				_dispatchEvent(nextEvent);
			} catch (std::exception &e) {
				_model->getTracer()->traceError("Error on processing event ("+nextEvent->show()+")", e);
			}
			auto afterProcessEvent = _createSimulationEvent();
			_model->getOnEventManager()->NotifyAfterProcessEventHandlers(afterProcessEvent.get());
			// Deletes processed events only after after-process notifications to preserve observer access.
			delete nextEvent;
			_currentEvent = nullptr;
			if (_pauseOnEvent) {
				_pauseRequested = true;
			}
			Util::DecIndent();
		}
	} else {
		// Removes and destroys out-of-window events to keep event lifecycle ownership consistent.
		_model->getFutureEvents()->pop_front();
		delete nextEvent;
		this->_simulatedTime = _replicationLength * _replicationTimeScaleFactorToBase; ////nextEvent->getTime(); // just to advance time to beyond simulatedTime
	}
}


void ModelSimulation::_dispatchEvent(Event* event) {
	InternalEvent* intEvent = dynamic_cast<InternalEvent*> (event);
	if (intEvent==nullptr) {
        /* @ToDo: (pequena alteração): SHOW ONLY BASED ON CONFIGURATION */
		_model->getTracer()->traceSimulation(this, TraceManager::Level::L9_mostDetailed, "Entity "+event->getEntity()->show());
		try {
			ModelComponent::DispatchEvent(event); //->_onDispatchEvent(entity, inputPortNumber);
		} catch (const std::exception& e) {
			_model->getTracer()->traceError("Error executing component "+event->getComponent()->show(), e);
		}
	} else { // ohh, this is brand new (2022/07/05). An "Internal Event", wich is unrelated to an entity or to a component
		ModelDataDefinition* df = static_cast<ModelDataDefinition*> (intEvent->object());
		std::string msg = "Event \""+intEvent->description()+"\" handled by \""+df->getName()+"\"";
		_model->getTracer()->traceSimulation(intEvent->object(), TraceManager::Level::L7_internal, msg);
		intEvent->dispatchEvent();
		//intEvent->eventHandler()(intEvent->parameter());
	}
}

bool ModelSimulation::_checkBreakpointAt(Event* event) {
	bool res = false;
	if (dynamic_cast<InternalEvent*> (event)==nullptr) {
		if (_breakpointsOnComponent->find(event->getComponent())!=_breakpointsOnComponent->list()->end()) {
			if (_justTriggeredBreakpointsOnComponent==event->getComponent()) {
				_justTriggeredBreakpointsOnComponent = nullptr;
			} else {
				_justTriggeredBreakpointsOnComponent = event->getComponent();
                auto se = _createSimulationEvent();
                _model->getOnEventManager()->NotifyBreakpointHandlers(se.get());
				_model->getTracer()->trace("Breakpoint found at component '"+event->getComponent()->getName()+"'. Replication is paused.", TraceManager::Level::L5_event);

				res = true;
			}
		}
		if (_breakpointsOnEntity->find(event->getEntity())!=_breakpointsOnEntity->list()->end()) {
			if (_justTriggeredBreakpointsOnEntity==event->getEntity()) {
				_justTriggeredBreakpointsOnEntity = nullptr;
			} else {
				_justTriggeredBreakpointsOnEntity = event->getEntity();
				_model->getTracer()->trace("Breakpoint found at entity '"+event->getEntity()->getName()+"'. Replication is paused.", TraceManager::Level::L5_event);
                auto se = _createSimulationEvent();
                _model->getOnEventManager()->NotifyBreakpointHandlers(se.get());
				res = true;
			}
		}
	}
	double time;
	for (double breakpointTime : *_breakpointsOnTime->list()) {
		time = breakpointTime;
		if (_simulatedTime<time&&event->getTime()>=time) {
			if (_justTriggeredBreakpointsOnTime==time) { // just trrigered this breakpoint
				_justTriggeredBreakpointsOnTime = 0.0;
			} else {
				_justTriggeredBreakpointsOnTime = time;
				_model->getTracer()->trace("Breakpoint found at time '"+std::to_string(event->getTime())+"'. Replication is paused.", TraceManager::Level::L5_event);
                auto se = _createSimulationEvent();
                _model->getOnEventManager()->NotifyBreakpointHandlers(se.get());

				return true;
			}
		}
	}
	return res;
}


void ModelSimulation::pause() {
	_pauseRequested = true;
}

void ModelSimulation::step() {
	bool savedPauseRequest = _pauseRequested;
	_pauseRequested = true;
	this->start();
	_pauseRequested = savedPauseRequest;
}

void ModelSimulation::stop() {
	this->_stopRequested = true;
}

void ModelSimulation::setPauseOnEvent(bool _pauseOnEvent) {
	this->_pauseOnEvent = _pauseOnEvent;
	_hasChanged = true;
}

bool ModelSimulation::isPauseOnEvent() const {
	return _pauseOnEvent;
}

void ModelSimulation::setInitializeStatistics(bool _initializeStatistics) {
	this->_initializeStatisticsBetweenReplications = _initializeStatistics;
	_hasChanged = true;
}

bool ModelSimulation::isInitializeStatistics() const {
	return _initializeStatisticsBetweenReplications;
}

void ModelSimulation::setInitializeSystem(bool _initializeSystem) {
	this->_initializeSystem = _initializeSystem;
	_hasChanged = true;
}

bool ModelSimulation::isInitializeSystem() const {
	return _initializeSystem;
}

void ModelSimulation::setStepByStep(bool _stepByStep) {
	this->_stepByStep = _stepByStep;
	_hasChanged = true;
}

bool ModelSimulation::isStepByStep() const {
	return _stepByStep;
}

void ModelSimulation::setPauseOnReplication(bool _pauseOnReplication) {
	this->_pauseOnReplication = _pauseOnReplication;
	_hasChanged = true;
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

//ModelComponent* ModelSimulation::getCurrentComponent() const {
//	return _currentComponent;
//}

//Entity* ModelSimulation::getCurrentEvent()->getEntity() const {
//	return _currentEntity;
//}

void ModelSimulation::setReporter(SimulationReporter_if* _simulationReporter) {
	if (this->_simulationReporter == _simulationReporter) {
		return;
	}
	if (_ownsSimulationReporter) {
		delete this->_simulationReporter;
	}
	this->_simulationReporter = _simulationReporter;
	_ownsSimulationReporter = false;
}

SimulationReporter_if* ModelSimulation::getReporter() const {
	return _simulationReporter;
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

const List<ModelDataDefinition*>* ModelSimulation::getSimulationStatisticsAggregates() const {
	return _cstatsAndCountersSimulation;
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

//void ModelSimulation::setReplicationLength(double _replicationLength) {
//	this->_replicationLength = _replicationLength;
//	_hasChanged = true;
//}

void ModelSimulation::setReplicationLength(double replicationLength, Util::TimeUnit replicationLengthTimeUnit) {
	this->_replicationLength = replicationLength;
	if (replicationLengthTimeUnit!=Util::TimeUnit::unknown)
		this->_replicationLengthTimeUnit = replicationLengthTimeUnit;
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

//void ModelSimulation::setWarmUpPeriod(double warmUpPeriod) {
//	this->_warmUpPeriod = warmUpPeriod;
//	_hasChanged = true;
//}

void ModelSimulation::setWarmUpPeriod(double warmUpPeriod, Util::TimeUnit warmUpPeriodTimeUnit) {
	this->_warmUpPeriod = warmUpPeriod;
	if (warmUpPeriodTimeUnit != Util::TimeUnit::unknown)
		this->_warmUpPeriodTimeUnit = warmUpPeriodTimeUnit;
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

void ModelSimulation::loadInstance(PersistenceRecord *fields) {
	this->_numberOfReplications = fields->loadField("numberOfReplications", DEFAULT.numberOfReplications);
	this->_replicationLength = fields->loadField("replicationLength", DEFAULT.replicationLength);
	this->_replicationLengthTimeUnit = fields->loadField("replicationLengthTimeUnit", DEFAULT.replicationLengthTimeUnit);
	this->_replicationBaseTimeUnit = fields->loadField("replicationBaseTimeUnit", DEFAULT.replicationBeseTimeUnit);
	this->_terminatingCondition = fields->loadField("terminatingCondition", DEFAULT.terminatingCondition);
	this->_warmUpPeriod = fields->loadField("warmUpTime", DEFAULT.warmUpPeriod);
	this->_warmUpPeriodTimeUnit = fields->loadField("warmUpTimeTimeUnit", DEFAULT.warmUpPeriodTimeUnit);
	this->_initializeStatisticsBetweenReplications = fields->loadField("initializeStatistics", DEFAULT.initializeStatisticsBetweenReplications);
	this->_initializeSystem = fields->loadField("initializeSystem", DEFAULT.initializeSystem);
	this->_stepByStep = fields->loadField("stepByStep", false);
	this->_pauseOnEvent = fields->loadField("pauseOnEvent", false);
	this->_pauseOnReplication = fields->loadField("pauseOnReplication", false);
	this->_showReportsAfterReplication = fields->loadField("showReportsAfterReplication", DEFAULT.showReportsAfterReplication);
	this->_showReportsAfterSimulation = fields->loadField("showReportsAfterSimulation", DEFAULT.showReportsAfterSimulation);
	this->_showSimulationControlsInReport = fields->loadField("showSimulationControlsInReport", DEFAULT.showSimulationControlsInReport);
	this->_showSimulationResposesInReport = fields->loadField("showSimulationResposesInReport", DEFAULT.showSimulationResposesInReport);
	// not a field of ModelSimulation, but I'll load it here
	TraceManager::Level traceLevel = static_cast<TraceManager::Level> (fields->loadField("traceLevel", static_cast<int> (TraitsKernel<Model>::traceLevel)));
	this->_model->getTracer()->setTraceLevel(traceLevel);
	_hasChanged = false;
}

// @ToDo: (importante): implement check method (to check things like terminating condition)

void ModelSimulation::saveInstance(PersistenceRecord *fields, bool saveDefaults) {
	fields->saveField("typename", "ModelSimulation");
	fields->saveField("name", ""); //getName());
	fields->saveField("numberOfReplications", _numberOfReplications, DEFAULT.numberOfReplications, saveDefaults);
	fields->saveField("replicationLength", _replicationLength, DEFAULT.replicationLength, saveDefaults);
	fields->saveField("replicationLengthTimeUnit", _replicationLengthTimeUnit, DEFAULT.replicationLengthTimeUnit, saveDefaults);
	fields->saveField("replicationBaseTimeUnit", _replicationBaseTimeUnit, DEFAULT.replicationBeseTimeUnit, saveDefaults);
	fields->saveField("terminatingCondition", _terminatingCondition, DEFAULT.terminatingCondition, saveDefaults);
	fields->saveField("warmUpTime", _warmUpPeriod, DEFAULT.warmUpPeriod, saveDefaults);
	fields->saveField("warmUpTimeTimeUnit", _warmUpPeriodTimeUnit, DEFAULT.warmUpPeriodTimeUnit, saveDefaults);
	fields->saveField("initializeStatistics", _initializeStatisticsBetweenReplications, DEFAULT.initializeStatisticsBetweenReplications, saveDefaults);
	fields->saveField("initializeSystem", _initializeSystem, DEFAULT.initializeSystem, saveDefaults);
	fields->saveField("stepByStep", _stepByStep, false, saveDefaults);
	fields->saveField("pauseOnEvent", _pauseOnEvent, false, saveDefaults);
	fields->saveField("pauseOnReplication", _pauseOnReplication, false, saveDefaults);
	fields->saveField("showReportsAfterReplicaton", _showReportsAfterReplication, DEFAULT.showReportsAfterReplication, saveDefaults);
	fields->saveField("showReportsAfterSimulation", _showReportsAfterSimulation, DEFAULT.showReportsAfterSimulation, saveDefaults);
	fields->saveField("showSimulationControlsInReport", _showSimulationControlsInReport, DEFAULT.showSimulationControlsInReport, saveDefaults);
	fields->saveField("showSimulationResposesInReport", _showSimulationResposesInReport, DEFAULT.showSimulationResposesInReport, saveDefaults);
	// @ToDo: (importante): not a field of ModelSimulation, but I'll save it here for now
	fields->saveField("traceLevel", static_cast<int> (_model->getTracer()->getTraceLevel()), static_cast<int> (TraitsKernel<Model>::traceLevel));
	_hasChanged = false;
}

Event* ModelSimulation::getCurrentEvent() const {
	return _currentEvent;
}

void ModelSimulation::setShowSimulationResposesInReport(bool _showSimulationResposesInReport) {
	this->_showSimulationResposesInReport = _showSimulationResposesInReport;
}

bool ModelSimulation::isShowSimulationResposesInReport() const {
	return _showSimulationResposesInReport;
}

void ModelSimulation::setShowSimulationControlsInReport(bool _showSimulationControlsInReport) {
	this->_showSimulationControlsInReport = _showSimulationControlsInReport;
}

bool ModelSimulation::isShowSimulationControlsInReport() const {
	return _showSimulationControlsInReport;
}

void ModelSimulation::setReplicationReportBaseTimeUnit(Util::TimeUnit _replicationReportBaseTimeUnit) {
	this->_replicationBaseTimeUnit = _replicationReportBaseTimeUnit;
	_hasChanged = true;
}

Util::TimeUnit ModelSimulation::getReplicationBaseTimeUnit() const {
	return _replicationBaseTimeUnit;
}
