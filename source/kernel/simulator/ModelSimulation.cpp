/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */;

/*
 * File:   ModelSimulation.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 7 de Novembro de 2018, 18:04
 */

#include "ModelSimulation.h"
#include <iostream>
#include <cstring>
#include <poll.h>
#include <cassert>
#include <thread>
#include <chrono>
#include "Model.h"
#include "Simulator.h"
#include "SourceModelComponent.h"
#include "StatisticsCollector.h"
#include "Counter.h"
#include "ComponentManager.h"
#include "Property.h"
#include "../TraitsKernel.h"

//using namespace GenesysKernel;

ModelSimulation::ModelSimulation(Model* model) {
	_model = model;
	_info = model->getInfos();
	_network = new TraitsKernel<Network_if>::Implementation(model);
	_networkScheduler = new TraitsKernel<NetworkScheduler_if>::Implementation(model);
	_cstatsAndCountersSimulation->setSortFunc([](const ModelDataDefinition* a, const ModelDataDefinition * b) {
		return a->getId() < b->getId();
	});
	_simulationReporter = new TraitsKernel<SimulationReporter_if>::Implementation(this, model, this->_cstatsAndCountersSimulation);
}

void ModelSimulation::startServerSimulation(int argc, char** argv) {
	_network->opt(argc, argv);
	_network->setServer(true);
	TraitsKernel<Network_if>::Socket_Data* _socket = _network->newSocketDataServer();
	_network->serverBind(_socket);
	_network->serverListen(_socket);
	std::thread thread_simulation;
	std::thread thread_results;

	_networkGetResult.lock();
	_networkSimulationEnded.lock();
	thread_results = std::thread(&ModelSimulation::threadWaitResults, this, _socket);
	while (1) {
		Util::NetworkCode code = _network->getNextNetworkEvent();
		switch (code)
		{
			case Util::NetworkCode::C0_Nothing:
				break;
			case Util::NetworkCode::C1_IsAlive:
				_network->sendCodeMessage(Util::NetworkCode::C1_IsAlive);
				break;
			case Util::NetworkCode::C2_Benchmark:
				_network->sendBenchmark();
				break;
			case Util::NetworkCode::C3_Model:
				if (!_network->receiveModel(_socket))
					break;
				_model->getSimulation()->setNumberOfReplications(_socket->_numberOfReplications);
				_model->setSamplerSeed(_socket->_seed);
				if (!_isNetworkModelRunning) {
					if (_model->load("networkModel.gen")) {
						if (thread_simulation.joinable())
							thread_simulation.join();
						_isNetworkModelRunningMutex.lock();
						_isNetworkModelRunning = true;
						_isNetworkModelRunningMutex.unlock();
						thread_simulation = std::thread(&ModelSimulation::start, this);
						_network->sendCodeMessage(Util::NetworkCode::C3_Model);
					} else {
						_network->sendCodeMessage(Util::NetworkCode::C6_Error);
					}
				} else {
					_network->sendCodeMessage(Util::NetworkCode::C6_Error);
				}
				break;
			case Util::NetworkCode::C4_Results:
				if (_isNetworkModelRunning) {
					_getResultNetworkMutex.lock();
					_getResultNetwork = true;
					_getResultNetworkMutex.unlock();
				}
				break;
			case Util::NetworkCode::C5_CalcelOP:
				_getResultNetworkMutex.lock();
				_getResultNetwork = false;
				if (_isNetworkModelRunning) {
					_isNetworkModelRunningMutex.lock();
					_isNetworkModelRunning = false;
					thread_simulation.detach();
					thread_simulation.~thread();
					_isNetworkModelRunningMutex.unlock();
				}
				_getResultNetworkMutex.unlock();
				// _networkSimulationEnded.unlock();
				// _networkSimulationEnded.lock();
				_network->sendCodeMessage(Util::NetworkCode::C5_CalcelOP);
				_network->reset();
				break;
			case Util::NetworkCode::C6_Error:
				break;
			default:
				break;
		}
	}
}

void ModelSimulation::threadWaitResults(Network_if::Socket_Data* socket) {
	while (1) {
		_networkSimulationEnded.lock();
		if (_getResultNetwork) {
			_network->sendCodeMessage(Util::NetworkCode::C4_Results);
			_network->sendModelResults(socket);
			_network->reset();
			_isNetworkModelRunningMutex.lock();
			_isNetworkModelRunning = false;
			_isNetworkModelRunningMutex.unlock();
			_getResultNetworkMutex.lock();
			_getResultNetwork = false;
			_getResultNetworkMutex.unlock();
		}
	}
}

void ModelSimulation::startClientSimulation(int argc, char** argv) {
	_network->opt(argc, argv);
	if (_numberOfReplications == 1) {
		_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "There is only one replication defined. Network simulation will be ignored.");
		start();
		return;
	}
	if (!_network->check()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Network check failed. Cannot start simulation in network.");
		return;
	}
	_network->setClient(true);
	_network->createSockets(&_network->_sockets);
	_network->getBenchmarks(&_network->_sockets);
	_networkReplicationMutex.lock();
	// _networkMutex.lock();
	_networkScheduler->set(_model->getSimulation()->_numberOfReplications, &_network->_sockets);
	//Create a thread for every socket...
	int threads_size = _network->_sockets.size();
	std::thread thread_simulation[threads_size];
	std::thread thread_start;
	thread_start = std::thread(&ModelSimulation::start, this);
	for (int i = 0; i < threads_size; i++) {
		thread_simulation[i] = std::thread(&ModelSimulation::threadClientSimulation, this, _network->_sockets.at(i));
	}

	// start();
	thread_start.join();
	for (int i = 0; i < threads_size; i++)
		thread_simulation[i].detach();
	for (int i = 0; i < _network->_sockets.size(); i++)
		close(_network->_sockets.at(i)->_socket);

}

void ModelSimulation::insertNetworkData(NetworkScheduler_if::Scheduler_Info* info, std::vector<double>* data) {
	//_
	_networkDataMutex.lock();
	_data.clear();
	for (int i = 0; i < data->size(); i++)
		_data.insert(_data.begin(), data->at(i));
	info->finished = true;
	_networkReplicationMutex.unlock();
}

void ModelSimulation::threadClientSimulation(Network_if::Socket_Data* socket) {
	TraitsKernel<NetworkScheduler_if>::Scheduler_Info* info = nullptr;
	while((info = _networkScheduler->getNextSimulation(socket)) != nullptr) {
		std::cout << info->id << std::endl;
		std::cout << info->numberOfReplications << std::endl;
		std::cout << info->seed << std::endl;
		std::cout << info->time_stamp << std::endl;
		std::cout << info->finished << std::endl;
		socket->_id = info->id;
		socket->_seed = info->seed;
		socket->_numberOfReplications = info->numberOfReplications;
		_network->sendModel(socket);
		std::vector<double> _results;
		_network->receiveModelResults(socket, &_results);
		if (_results.size() == 0)
			continue;
		insertNetworkData(info, &_results);
	}
	// _networkMutex.unlock();
}

std::string ModelSimulation::show() {
	return "numberOfReplications=" + std::to_string(_numberOfReplications) +
			",replicationLength=" + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(this->_replicationLengthTimeUnit) +
			",terminatingCondition=\"" + this->_terminatingCondition + "\"" +
			",warmupTime=" + std::to_string(this->_warmUpPeriod) + " " + Util::StrTimeUnitLong(this->_warmUpPeriodTimeUnit);
}

bool ModelSimulation::_isReplicationEndCondition() {
	bool finish = _model->getFutureEvents()->size() == 0;
	if (!finish) {
		finish = _model->getFutureEvents()->front()->getTime() > _replicationLength * _replicationTimeScaleFactorToBase;
		if (!finish && _terminatingCondition != "") {
			finish = _model->parseExpression(_terminatingCondition) != 0.0;
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
	} else if (_model->getFutureEvents()->front()->getTime() > _replicationLength) {
		causeTerminated = "replication length " + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(_replicationLengthTimeUnit) + " was achieved";
	} else if (_model->parseExpression(_terminatingCondition)) {
		causeTerminated = "termination condition was achieved";
	} else causeTerminated = "unknown";
	std::chrono::duration<double> duration = std::chrono::system_clock::now() - this->_startRealSimulationTimeReplication;
	std::string message = "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_numberOfReplications) + " has finished with last event at time " + std::to_string(_simulatedTime) + " " + Util::StrTimeUnitLong(_replicationBaseTimeUnit) + " because " + causeTerminated + "; Elapsed time " + std::to_string(duration.count()) + " seconds.";
	_model->getTracer()->traceSimulation(this, Util::TraceLevel::L2_results, message);
}

SimulationEvent* ModelSimulation::_createSimulationEvent(void* thiscustomObject) {
	SimulationEvent* se = new SimulationEvent();
	//	se->currentComponent = _currentComponent;
	//	se->currentEntity = _currentEntity;
	se->currentEvent = _currentEvent;
	//	se->currentinputPortNumber = _currentinputPortNumber;
	se->currentReplicationNumber = _currentReplicationNumber;
	se->customObject = thiscustomObject;
	se->_isPaused = this->_isPaused;
	se->_isRunning = this->_isRunning;
	se->pauseRequested = _pauseRequested;
	se->simulatedTime = _simulatedTime;
	se->stopRequested = _stopRequested;
	return se;
}

/*!
 * Checks the model and if ok then initialize the simulation, execute repeatedly each replication and then show simulation statistics
 */
void ModelSimulation::start() {
	std::cout << "chegu aqui::" << std::endl;
	if (!_simulationIsInitiated) { // begin of a new simulation
		Util::SetIndent(0); //force indentation
		if (!_model->check()) {
			_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Model check failed. Cannot start simulation.");
			return;
		}
		_initSimulation();
		_isRunning = true; // set it before notifying handlers
		_model->getOnEvents()->NotifySimulationStartHandlers(_createSimulationEvent());
	}
	_isRunning = true;
	if (_isPaused) { // continue after a pause
		_model->getTracer()->trace("Replication resumed", Util::TraceLevel::L3_errorRecover);
		_isPaused = false; // set it before notifying handlers
		_model->getOnEvents()->NotifySimulationResumeHandlers(_createSimulationEvent());
	}
	bool replicationEnded;
	do {
		if(!_network->isClient()) {
			if (!_replicationIsInitiaded) {
				Util::SetIndent(1);
				_initReplication();
				_model->getOnEvents()->NotifyReplicationStartHandlers(_createSimulationEvent());
				Util::IncIndent();
			}
			replicationEnded = _isReplicationEndCondition();
			while (!replicationEnded) { // this is the main simulation loop
				_stepSimulation();
				replicationEnded = _isReplicationEndCondition();
				if (_pauseRequested || _stopRequested) { //check this only after _stepSimulation() and not on loop entering conditin
					break;
				}
			};
		}
		if (replicationEnded || _network->isClient()) {
			if (_network->isClient())
				_networkReplicationMutex.lock();
			Util::SetIndent(1); // force
			_replicationEnded();
			_currentReplicationNumber++;
			if (_currentReplicationNumber <= _numberOfReplications) {
				if (_pauseOnReplication) {
					_model->getTracer()->trace("End of replication. Simulation is paused.", Util::TraceLevel::L7_internal);
					_pauseRequested = true;
				}
			} else {
				_pauseRequested = false;
			}
			if (_network->isClient())
				_networkDataMutex.unlock();
		}
	} while (_currentReplicationNumber <= _numberOfReplications && !(_pauseRequested || _stopRequested));
	// all replications done (or paused during execution)
	_isRunning = false;
	if (!_pauseRequested) { // done or stopped
		_stopRequested = false;
		_simulationEnded();
	} else { // paused
		_model->getTracer()->trace("Replication paused", Util::TraceLevel::L3_errorRecover);
		_pauseRequested = false; // set them before notifying handlers
		_isPaused = true;
		_model->getOnEvents()->NotifySimulationPausedHandlers(_createSimulationEvent());
	}
	if (_network->isServer())
		_networkSimulationEnded.unlock();
}

void ModelSimulation::_simulationEnded() {
	_simulationIsInitiated = false;
	if (this->_showReportsAfterSimulation)
		_simulationReporter->showSimulationStatistics(); //_cStatsSimulation);
	Util::DecIndent();
	// clear current event
	//_currentEntity = nullptr;
	//_currentComponent = nullptr;
	_currentEvent = nullptr;
	//
	std::chrono::duration<double> duration = std::chrono::system_clock::now() - this->_startRealSimulationTimeSimulation;
	_model->getTracer()->traceSimulation(this, Util::TraceLevel::L5_event, "Simulation of model \"" + _info->getName() + "\" has finished. Elapsed time " + std::to_string(duration.count()) + " seconds.");
	_model->getOnEvents()->NotifySimulationEndHandlers(_createSimulationEvent());
}

void ModelSimulation::_replicationEnded() {
	_traceReplicationEnded();
	_model->getOnEvents()->NotifyReplicationEndHandlers(_createSimulationEvent());
	// if (this->_showReportsAfterReplication)
	// 	_simulationReporter->showReplicationStatistics();
	//_simulationReporter->showSimulationResponses();
	_actualizeSimulationStatistics();
	_replicationIsInitiaded = false;
}

void ModelSimulation::_actualizeSimulationStatistics() {
	int count = 0;
	//@TODO: should not be only CSTAT and COUNTER, but any modeldatum that generateReportInformation
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
			if (datasim->getClassname() == UtilTypeOfStatisticsCollector) {
				//if ((*itSim)->getName() == _cte_stCountSimulNamePrefix + sc->getName() && dynamic_cast<StatisticsCollector*> (*itSim)->getParent() == sc->getParent()) { // found
				if (datasim->getName() == _cte_stCountSimulNamePrefix + cstatModel->getName()) {
					if (dynamic_cast<StatisticsCollector*> (datasim)->getParent() == cstatModel->getParent()) { // found
						cstatSimulation = dynamic_cast<StatisticsCollector*> (datasim); // (*itSim);
						break;
					}
				}
			}
		}
		if (cstatSimulation == nullptr) {
			// this is a new cstat created during the last replication and didn't existed in simulation before
			cstatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix + cstatModel->getName(), cstatModel->getParent(), false);
			_cstatsAndCountersSimulation->insert(cstatSimulation);
		}
		assert(cstatSimulation != nullptr);
		// actualize simulation cstat statistics by collecting the new value from the model/replication stat
		if (_network->isServer()) {
			_network->insertNewData(cstatModel->getStatistics()->average());
			cstatSimulation->getStatistics()->getCollector()->addValue(cstatModel->getStatistics()->average());
		} else if (_network->isClient()) {
			//pass
			std::cout << "[B]" <<  _data.at(count) << std::endl;
			cstatSimulation->getStatistics()->getCollector()->addValue(_data.at(count));
			count++;

		} else {
			cstatSimulation->getStatistics()->getCollector()->addValue(cstatModel->getStatistics()->average());
		}
	}
	// runs over all Counters in the model and create the equivalent for the entire simulation
	Counter *counterModel; //, *cntSim;
	List<ModelDataDefinition*>* counters = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
	for (ModelDataDefinition* countData : *counters->list()) {
		counterModel = dynamic_cast<Counter*> (countData);
		cstatSimulation = nullptr;
		for (ModelDataDefinition* dataSim : *_cstatsAndCountersSimulation->list()) {
			if (dataSim->getClassname() == UtilTypeOfStatisticsCollector) {
				if (dataSim->getName() == _cte_stCountSimulNamePrefix + counterModel->getName()) {
					if (dynamic_cast<StatisticsCollector*> (dataSim)->getParent() == counterModel->getParent()) {
						cstatSimulation = dynamic_cast<StatisticsCollector*> (dataSim);
						break;
					}
				}
			}
		}
		assert(cstatSimulation != nullptr);
		// actualize simulation cstat statistics by collecting the new value from the model/replication stat
		if (_network->isServer()) {
			_network->insertNewData(counterModel->getCountValue());
			cstatSimulation->getStatistics()->getCollector()->addValue(counterModel->getCountValue());
		} else if (_network->isClient()) {
			std::cout << "[B]" << _data.at(count) << std::endl;
			cstatSimulation->getStatistics()->getCollector()->addValue(_data.at(count));
			count++;
		} else {
			cstatSimulation->getStatistics()->getCollector()->addValue(counterModel->getCountValue());
		}
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
	tm->traceReport("Analyst Name: " + _info->getAnalystName());
	tm->traceReport("Project Title: " + _info->getProjectTitle());
	tm->traceReport("Number of Replications: " + std::to_string(_numberOfReplications));
	tm->traceReport("Replication Length: " + std::to_string(_replicationLength) + " " + Util::StrTimeUnitLong(_replicationLengthTimeUnit));
	tm->traceReport("Replication/Report Base TimeUnit: " + Util::StrTimeUnitLong(_replicationBaseTimeUnit));
	//tm->traceReport(Util::TraceLevel::simulation, "");
	// model controls and responses
	std::string controls;
	for (PropertyBase* control : * _model->getControls()->list()) {
		controls += control->getName() + "(" + control->getClassname() + ")=" + std::to_string(control->getValue()) + ", ";
	}
	controls = controls.substr(0, controls.length() - 2);
	tm->traceReport("> Simulation controls: " + controls);
	std::string responses;
	for (PropertyBase* pg : *_model->getResponses()->list()) {
		responses += pg->getName() + "(" + pg->getClassname() + "), ";
	}
	responses = responses.substr(0, responses.length() - 2);
	if (TraitsKernel<SimulationReporter_if>::showSimulationResponses) {
		tm->traceReport("> Simulation responses: " + responses);
	}
	tm->traceReport("");
}

/*!
 * Initialize once for all replications
 */
void ModelSimulation::_initSimulation() {
	_startRealSimulationTimeSimulation = std::chrono::system_clock::now();
	_showSimulationHeader();
	_model->getTracer()->trace(Util::TraceLevel::L5_event, "");
	_model->getTracer()->traceSimulation(this, Util::TraceLevel::L5_event, "Simulation of model \"" + _info->getName() + "\" is starting.");
	// defines the time scale factor to adjust replicatonLength to replicationBaseTime
	_replicationTimeScaleFactorToBase = Util::TimeUnitConvert(this->_replicationLengthTimeUnit, this->_replicationBaseTimeUnit);
	// copy all CStats and Counters (used in a replication) to CStats and counters for the whole simulation
	// @TODO: Should not be CStats and Counters, but any modeldatum that generates report importation
	this->_cstatsAndCountersSimulation->clear();
	StatisticsCollector* cstat;
	List<ModelDataDefinition*>* cstats = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());
	for (ModelDataDefinition* cstatData : *cstats->list()) {
		cstat = dynamic_cast<StatisticsCollector*> (cstatData);
		// this new CSat should NOT be inserted into the model (so the false as last argument)
		StatisticsCollector* newCStatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix + cstat->getName(), cstat->getParent(), false);
		this->_cstatsAndCountersSimulation->insert(newCStatSimulation);
	}
	// copy all Counters (used in a replication) to Counters for the whole simulation
	// @TODO: Counters in replication should be converted into CStats in simulation. Each value counted in a replication should be added in a CStat for Stats.
	Counter* counter;
	List<ModelDataDefinition*>* counters = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
	for (ModelDataDefinition* counterData : *counters->list()) {
		counter = dynamic_cast<Counter*> (counterData);
		/* // we do NOT add a counter in the simulation. We add a CStat that collect statistics about the Counter
		Counter* newCountSimul = new Counter(_cte_stCountSimulNamePrefix + counter->getName(), counter->getParent());
		this->_statsCountersSimulation->insert(newCountSimul);
		 */
		// addin a cstat (to stat the counts)
		StatisticsCollector* newCStatSimulation = new StatisticsCollector(_model, _cte_stCountSimulNamePrefix + counter->getName(), counter->getParent(), false);
		this->_cstatsAndCountersSimulation->insert(newCStatSimulation);
	}
	_simulationIsInitiated = true; // @TODO Check the uses of _simulationIsInitiated and when it should be set to false
	_replicationIsInitiaded = false;
	_currentReplicationNumber = 1;
}

void ModelSimulation::_initReplication() {
	_startRealSimulationTimeReplication = std::chrono::system_clock::now();
	TraceManager* tm = _model->getTracer();
	tm->traceSimulation(this, Util::TraceLevel::L5_event, ""); //@TODO L5 and L2??
	tm->traceSimulation(this, Util::TraceLevel::L2_results, "Replication " + std::to_string(_currentReplicationNumber) + " of " + std::to_string(_numberOfReplications) + " is starting.");
	_model->getFutureEvents()->clear();
	_model->getDataManager()->getDataDefinitionList("Entity")->clear();
	_simulatedTime = 0.0;
	// init all components between replications
	Util::IncIndent();
	tm->traceSimulation(this, Util::TraceLevel::L8_detailed, "Initing Replication");
	Util::IncIndent();
	for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
		ModelComponent::InitBetweenReplications((*it));
	}
	// init all elements between replications
	std::list<std::string>* elementTypes = _model->getDataManager()->getDataDefinitionClassnames();
	for (std::string elementType : *elementTypes) {//std::list<std::string>::iterator typeIt = elementTypes->begin(); typeIt != elementTypes->end(); typeIt++) {
		List<ModelDataDefinition*>* elements = _model->getDataManager()->getDataDefinitionList(elementType);
		for (ModelDataDefinition* modeldatum : *elements->list()) {//std::list<ModelDataDefinition*>::iterator it = elements->list()->begin(); it != elements->list()->end(); it++) {
			ModelDataDefinition::InitBetweenReplications(modeldatum);
		}
	}
	Util::DecIndent();
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
	for (std::list<ModelComponent*>::iterator it = _model->getComponents()->begin(); it != _model->getComponents()->end(); it++) {
		source = dynamic_cast<SourceModelComponent*> (*it);
		if (source != nullptr) {
			creationTime = source->getFirstCreation();
			numToCreate = source->getEntitiesPerCreation();
			for (unsigned int i = 1; i <= numToCreate; i++) {
				newEntity = _model->createEntity(source->getEntityType()->getName() + "_%", false);
				newEntity->setEntityType(source->getEntityType());
				newEvent = new Event(creationTime, newEntity, (*it));
				_model->getFutureEvents()->insert(newEvent);
			}
			source->setEntitiesCreated(numToCreate);
		}
	}
	if (this->_initializeStatisticsBetweenReplications) {
		_clearStatistics();
	}
	this->_replicationIsInitiaded = true; // @TODO Check the uses of _replicationIsInitiaded and when it should be set to false
}

void ModelSimulation::_clearStatistics() {
	//@Todo create a OnClearStatistics event handler
	StatisticsCollector* cstat;
	List<ModelDataDefinition*>* list = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());
	for (std::list<ModelDataDefinition*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		cstat = (StatisticsCollector*) (*it);
		cstat->getStatistics()->getCollector()->clear();
	}
	Counter* counter;
	list = _model->getDataManager()->getDataDefinitionList(Util::TypeOf<Counter>());
	for (std::list<ModelDataDefinition*>::iterator it = list->list()->begin(); it != list->list()->end(); it++) {
		counter = (Counter*) (*it);
		counter->clear();
	}
}

void ModelSimulation::_checkWarmUpTime(Event* nextEvent) {
	double warmupTime = Util::TimeUnitConvert(_warmUpPeriodTimeUnit, _replicationBaseTimeUnit);
	warmupTime *= _warmUpPeriod;
	if (warmupTime > 0.0 && _model->getSimulation()->getSimulatedTime() <= warmupTime && nextEvent->getTime() > warmupTime) {// warmuTime. Time to initStats
		_model->getTracer()->traceSimulation(this, Util::TraceLevel::L7_internal, "Warmup time reached. Statistics are being reseted.");
		_clearStatistics();
	}
}

void ModelSimulation::_stepSimulation() {
	// "onReplicationStep" is before taking the event from the calendar and "onProcessEvent" is after the event is removed and turned into the current one
	_model->getOnEvents()->NotifyReplicationStepHandlers(_createSimulationEvent());
	Event* nextEvent = _model->getFutureEvents()->front();
	_model->getFutureEvents()->pop_front();
	if (_warmUpPeriod > 0.0)
		_checkWarmUpTime(nextEvent);
	if (nextEvent->getTime() <= _replicationLength * _replicationTimeScaleFactorToBase) {
		if (_checkBreakpointAt(nextEvent)) {
			this->_pauseRequested = true;
		} else {
			_model->getTracer()->traceSimulation(this, Util::TraceLevel::L5_event, "Event {" + nextEvent->show() + "}");
			Util::IncIndent();
			_model->getTracer()->traceSimulation(this, Util::TraceLevel::L8_detailed, "Entity " + nextEvent->getEntity()->show());
			this->_currentEvent = nextEvent;
			//assert(_simulatedTime <= event->getTime()); // _simulatedTime only goes forward (futureEvents is chronologically sorted
			if (nextEvent->getTime() >= _simulatedTime) { // the philosophycal approach taken is: if the next event is in the past, lets just assume it's happening rigth now...
				_simulatedTime = nextEvent->getTime();
			}
			_model->getOnEvents()->NotifyProcessEventHandlers(_createSimulationEvent());
			try {
				ModelComponent::DispatchEvent(nextEvent);
			} catch (std::exception *e) {
				_model->getTracer()->traceError(*e, "Error on processing event (" + nextEvent->show() + ")");
			}
			if (_pauseOnEvent) {
				_pauseRequested = true;
			}
			Util::DecIndent();
		}
	} else {
		this->_simulatedTime = _replicationLength; ////nextEvent->getTime(); // just to advance time to beyond simulatedTime
	}
}

bool ModelSimulation::_checkBreakpointAt(Event* event) {
	bool res = false;
	SimulationEvent* se = _createSimulationEvent();
	if (_breakpointsOnComponent->find(event->getComponent()) != _breakpointsOnComponent->list()->end()) {
		if (_justTriggeredBreakpointsOnComponent == event->getComponent()) {
			_justTriggeredBreakpointsOnComponent = nullptr;
		} else {
			_justTriggeredBreakpointsOnComponent = event->getComponent();
			_model->getOnEvents()->NotifyBreakpointHandlers(se);
			_model->getTracer()->trace("Breakpoint found at component '" + event->getComponent()->getName() + "'. Replication is paused.", Util::TraceLevel::L5_event);

			res = true;
		}
	}
	if (_breakpointsOnEntity->find(event->getEntity()) != _breakpointsOnEntity->list()->end()) {
		if (_justTriggeredBreakpointsOnEntity == event->getEntity()) {
			_justTriggeredBreakpointsOnEntity = nullptr;
		} else {
			_justTriggeredBreakpointsOnEntity = event->getEntity();
			_model->getTracer()->trace("Breakpoint found at entity '" + event->getEntity()->getName() + "'. Replication is paused.", Util::TraceLevel::L5_event);
			_model->getOnEvents()->NotifyBreakpointHandlers(se);
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
				_model->getTracer()->trace("Breakpoint found at time '" + std::to_string(event->getTime()) + "'. Replication is paused.", Util::TraceLevel::L5_event);
				_model->getOnEvents()->NotifyBreakpointHandlers(se);
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

//ModelComponent* ModelSimulation::getCurrentComponent() const {
//	return _currentComponent;
//}

//Entity* ModelSimulation::getCurrentEvent()->getEntity() const {
//	return _currentEntity;
//}

void ModelSimulation::setReporter(SimulationReporter_if* _simulationReporter) {
	this->_simulationReporter = _simulationReporter;
}

SimulationReporter_if* ModelSimulation::getReporter() const {
	return _simulationReporter;
	//_currentEvent->
}

//unsigned int ModelSimulation::getCurrentinputPortNumber() const {
//	return _currentinputPortNumber;
//}

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
	this->_showReportsAfterReplication = LoadField(fields, "showReportsAfterReplication", DEFAULT.showReportsAfterReplication);
	this->_showReportsAfterSimulation = LoadField(fields, "showReportsAfterSimulation", DEFAULT.showReportsAfterSimulation);
	this->_showSimulationControlsInReport = LoadField(fields, "showSimulationControlsInReport", DEFAULT.showSimulationControlsInReport);
	this->_showSimulationResposesInReport = LoadField(fields, "showSimulationResposesInReport", DEFAULT.showSimulationResposesInReport);
	// not a field of ModelSimulation, but I'll load it here
	Util::TraceLevel traceLevel = static_cast<Util::TraceLevel> (LoadField(fields, "traceLevel", static_cast<int> (TraitsKernel<Model>::traceLevel)));
	this->_model->getTracer()->setTraceLevel(traceLevel);
	_hasChanged = false;
}

// @TODO:!: implement check method (to check things like terminating condition)

std::map<std::string, std::string>* ModelSimulation::saveInstance(bool saveDefaults) {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	SaveField(fields, "typename", "ModelSimulation");
	SaveField(fields, "name", ""); //getName());
	SaveField(fields, "numberOfReplications", _numberOfReplications, DEFAULT.numberOfReplications, saveDefaults);
	SaveField(fields, "replicationLength", _replicationLength, DEFAULT.replicationLength, saveDefaults);
	SaveField(fields, "replicationLengthTimeUnit", _replicationLengthTimeUnit, DEFAULT.replicationLengthTimeUnit, saveDefaults);
	SaveField(fields, "replicationBaseTimeUnit", _replicationBaseTimeUnit, DEFAULT.replicationBeseTimeUnit, saveDefaults);
	SaveField(fields, "terminatingCondition", _terminatingCondition, DEFAULT.terminatingCondition, saveDefaults);
	SaveField(fields, "warmUpTime", _warmUpPeriod, DEFAULT.warmUpPeriod, saveDefaults);
	SaveField(fields, "warmUpTimeTimeUnit", _warmUpPeriodTimeUnit, DEFAULT.warmUpPeriodTimeUnit, saveDefaults);
	SaveField(fields, "showReportsAfterReplicaton", _showReportsAfterReplication, DEFAULT.showReportsAfterReplication, saveDefaults);
	SaveField(fields, "showReportsAfterSimulation", _showReportsAfterSimulation, DEFAULT.showReportsAfterSimulation, saveDefaults);
	SaveField(fields, "showSimulationControlsInReport", _showSimulationControlsInReport, DEFAULT.showSimulationControlsInReport, saveDefaults);
	SaveField(fields, "showSimulationResposesInReport", _showSimulationResposesInReport, DEFAULT.showSimulationResposesInReport, saveDefaults);
	// @TODO not a field of ModelSimulation, but I'll save it here for now
	SaveField(fields, "traceLevel", static_cast<int> (_model->getTracer()->getTraceLevel()), static_cast<int> (TraitsKernel<Model>::traceLevel));
	_hasChanged = false;
	return fields;
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
}

Util::TimeUnit ModelSimulation::getReplicationBaseTimeUnit() const {
	return _replicationBaseTimeUnit;
}

Network_if* ModelSimulation::getNetwork() const {
	return _network;
}
