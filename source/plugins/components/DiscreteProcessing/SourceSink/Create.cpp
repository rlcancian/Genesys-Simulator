/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Create.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 20:12
 */

#include "plugins/components/DiscreteProcessing/SourceSink/Create.h"
#include "../../../../kernel/simulator/model/Model.h"
#include "../../../../kernel/simulator/essentialPlugins/EntityType.h"
#include "../../../../kernel/simulator/model/ModelDataManager.h"
#include <cassert>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Create::GetPluginInformation;
}
#endif

ModelDataDefinition* Create::NewInstance(Model* model, std::string name) {
	return new Create(model, name);
}

void Create::setTimeBetweenCreationsFormula(Formula*_timeBetweenCreationsFormula) {
	this->_timeBetweenCreationsFormula = _timeBetweenCreationsFormula;
}

Formula* Create::getTimeBetweenCreationsFormula() const {
	return _timeBetweenCreationsFormula;
}

void Create::setTimeBetweenCreationsSchedule(Schedule* _timeBetweenCreationsSchedule) {
	this->_timeBetweenCreationsSchedule = _timeBetweenCreationsSchedule;
}

Schedule* Create::getTimeBetweenCreationsSchedule() const {
	return _timeBetweenCreationsSchedule;
}

Create::Create(Model* model, std::string name) : SourceModelComponent(model, Util::TypeOf<Create>(), name) {
	SimulationControlGenericClass<Formula*, Model*, Formula>* propTimeBetweenCreationsFormula = new SimulationControlGenericClass<Formula*, Model*, Formula>(
									_parentModel,
									std::bind(&Create::getTimeBetweenCreationsFormula, this), std::bind(&Create::setTimeBetweenCreationsFormula, this, std::placeholders::_1),
									Util::TypeOf<Create>(), getName(), "TimeBetweenCreationsFormula", "");
	propTimeBetweenCreationsFormula->setDynamicEnabled([this]() -> bool {
		return this->getTimeBetweenCreationsExpression().empty()
				&& this->getTimeBetweenCreationsSchedule() == nullptr;
	});
	SimulationControlGenericClass<Schedule*, Model*, Schedule>* propTimeBetweenCreationsSchedule = new SimulationControlGenericClass<Schedule*, Model*, Schedule>(
									_parentModel,
									std::bind(&Create::getTimeBetweenCreationsSchedule, this), std::bind(&Create::setTimeBetweenCreationsSchedule, this, std::placeholders::_1),
									Util::TypeOf<Create>(), getName(), "TimeBetweenCreationsSchedule", "");
	propTimeBetweenCreationsSchedule->setDynamicEnabled([this]() -> bool {
		return this->getTimeBetweenCreationsExpression().empty()
				&& this->getTimeBetweenCreationsFormula() == nullptr;
	});

	// Configure the inherited SourceModelComponent property without touching
	// controls belonging to other model objects with the same property name.
	for (SimulationControl* control : *getSimulationControls()->list()) {
		if (control != nullptr && control->getName() == "TimeBetweenArrivals") {
			control->setDynamicEnabled([this]() -> bool {
				return this->getTimeBetweenCreationsFormula() == nullptr
						&& this->getTimeBetweenCreationsSchedule() == nullptr;
			});
			break;
		}
	}

	_parentModel->getControls()->insert(propTimeBetweenCreationsFormula);
	_parentModel->getControls()->insert(propTimeBetweenCreationsSchedule);

	// setting properties
	_addSimulationControl(propTimeBetweenCreationsFormula);
	_addSimulationControl(propTimeBetweenCreationsSchedule);
}

std::string Create::show() {
	return SourceModelComponent::show();
}

void Create::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	_parentModel->getDataManager()->insert(entity->getClassname(), entity);
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
    // init entity attibutes
    entity->setAttributeValue("Entity.ArrivalTime", tnow);
	entity->setAttributeValue("Entity.Type", (double) entity->getEntityType()->getId());
	//entity->setAttributeValue("Entity.Picture", 1);
	if (_entitiesAwaitingCurrentArrivalDispatch > 0) {
		--_entitiesAwaitingCurrentArrivalDispatch;
	}
	if (_entitiesAwaitingCurrentArrivalDispatch == 0) {
		double timeBetweenCreations, timeScale, newArrivalTime;
		const double parsedMaxCreations = _parentModel->parseExpression(this->_maxCreationsExpression);
		const unsigned int maxCreations = parsedMaxCreations <= 0.0 ? 0u
				: parsedMaxCreations >= static_cast<double>(std::numeric_limits<unsigned int>::max())
					? std::numeric_limits<unsigned int>::max()
					: static_cast<unsigned int>(parsedMaxCreations);
		if (_timeBetweenCreationsExpression != "") {
			timeBetweenCreations = _parentModel->parseExpression(_timeBetweenCreationsExpression);
		} else if (_timeBetweenCreationsSchedule != nullptr) {
			timeBetweenCreations = _parentModel->parseExpression(_timeBetweenCreationsSchedule->getExpression());
		} else if (_timeBetweenCreationsFormula != nullptr) {
			timeBetweenCreations = _parentModel->parseExpression(_timeBetweenCreationsFormula->getExpression());
		} else {
			//Never could get here
			assert(false);
		}
		timeScale = Util::TimeUnitConvert(this->_timeBetweenCreationsTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
		newArrivalTime = std::max<double>(tnow + timeBetweenCreations*timeScale, tnow); // force no time travel to past. Not sure if it should really be avoided
        // create new entities (to be sent to this component in future at time newArrivalTime
		const unsigned int remainingCreations = maxCreations > _entitiesCreatedSoFar
				? maxCreations - _entitiesCreatedSoFar : 0u;
		const unsigned int nextArrivalCreations = std::min(_entitiesPerCreation, remainingCreations);
		traceSimulation(this, "Scheduling next Create arrival", TraceManager::Level::L7_internal);
		traceSimulation(this, "Inter-arrival value=" + std::to_string(timeBetweenCreations) + ", scaled arrival time=" + std::to_string(newArrivalTime), TraceManager::Level::L8_detailed);
		_entitiesAwaitingCurrentArrivalDispatch = nextArrivalCreations;
		for (unsigned int i = 0; i < nextArrivalCreations; i++) {
			_entitiesCreatedSoFar++;
				Entity* newEntity = _parentModel->createEntity(entity->getEntityType()->getName() + "_%", false);
				newEntity->setEntityType(entity->getEntityType());
				Event* newEvent = new Event(newArrivalTime, newEntity, this);
				_parentModel->getFutureEvents()->insert(newEvent);
				traceSimulation(this, "Arrival of " + newEntity->getName() + " scheduled for time " + std::to_string(newArrivalTime) + Util::StrTimeUnitShort(_parentModel->getSimulation()->getReplicationBaseTimeUnit()), TraceManager::Level::L9_mostDetailed);
			}
		}
	if (_reportStatistics)
		_numberOut->incCountValue();
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

PluginInformation* Create::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Create>(), &Create::LoadInstance, &Create::NewInstance);
	info->setSource(true);
	info->setMaximumInputs(0);
	info->setMinimumInputs(0);
	info->insertDynamicLibFileDependence("formula.so");
	info->insertDynamicLibFileDependence("schedule.so");
	info->setCategory("Arena/SourceSink");
	std::string text = "This module is intended as the starting point for entities in a simulation model.";
	text += "	Entities are created using a schedule or based on a time between arrivals. Entities then leave the module to begin processing through the system.";
	text += "	The entity type is specified in this module.";
	text += "	Animation showing the number of entities created is displayed when the module is placed.";
	text += "	TYPICAL USES: (1) The start of a part’s production in a manufacturing line;";
	text += "	(2) A document’s arrival (for example, order, check, application) into a business process;";
	text += "	(3)A customer’s arrival at a service process (for example, retail store, restaurant, information desk)";
	info->setDescriptionHelp(text);
	//info->insertDynamicLibFileDependence("attribute.so");
	//info->insertDynamicLibFileDependence("entitytype.so");
	//info->insertDynamicLibFileDependence("statisticscollector.so");
	return info;
}

ModelComponent* Create::LoadInstance(Model* model, PersistenceRecord *fields) {
	Create* newComponent = new Create(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Create instance: " + std::string(e.what()));
	}

	return newComponent;
}

bool Create::_loadInstance(PersistenceRecord *fields) {
	return SourceModelComponent::_loadInstance(fields);
}

void Create::_initBetweenReplications() {
	SourceModelComponent::_initBetweenReplications();
	_entitiesAwaitingCurrentArrivalDispatch = _entitiesCreatedSoFar;
}

void Create::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	SourceModelComponent::_saveInstance(fields, saveDefaultValues);
}

bool Create::_check(std::string& errorMessage) {
	bool resultAll = SourceModelComponent::_check(errorMessage);

	const bool hasExpression = !_timeBetweenCreationsExpression.empty();
	const bool hasSchedule = _timeBetweenCreationsSchedule != nullptr;
	const bool hasFormula = _timeBetweenCreationsFormula != nullptr;
	const unsigned int activeGenerators = static_cast<unsigned int>(hasExpression) + static_cast<unsigned int>(hasSchedule) + static_cast<unsigned int>(hasFormula);

	if (activeGenerators != 1) {
		errorMessage += "Create must define exactly one time-between-creations source: expression, schedule, or formula. ";
		resultAll = false;
	}
	if (hasSchedule) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Schedule>(), _timeBetweenCreationsSchedule, "TimeBetweenCreationsSchedule", errorMessage);
		if (_timeBetweenCreationsSchedule != nullptr) {
			resultAll &= _parentModel->checkExpression(_timeBetweenCreationsSchedule->getExpression(), "TimeBetweenCreationsSchedule", errorMessage);
		}
	}
	if (hasFormula) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Formula>(), _timeBetweenCreationsFormula, "TimeBetweenCreationsFormula", errorMessage);
		if (_timeBetweenCreationsFormula != nullptr) {
			resultAll &= _parentModel->checkExpression(_timeBetweenCreationsFormula->getExpression(), "TimeBetweenCreationsFormula", errorMessage);
		}
	}
	return resultAll;
}

void Create::_createEditableDataDefinitions() {
	SourceModelComponent::_createEditableDataDefinitions();
	if (_timeBetweenCreationsSchedule != nullptr) {
		_optionalEditableDataDefinitionInsert("TimeBetweenCreationsSchedule", _timeBetweenCreationsSchedule);
	} else {
		_optionalEditableDataDefinitionRemove("TimeBetweenCreationsSchedule");
	}
	if (_timeBetweenCreationsFormula != nullptr) {
		_optionalEditableDataDefinitionInsert("TimeBetweenCreationsFormula", _timeBetweenCreationsFormula);
	} else {
		_optionalEditableDataDefinitionRemove("TimeBetweenCreationsFormula");
	}
}

void Create::_createInternalStatisticReporters() {
	if (_reportStatistics && _numberOut == nullptr) {
		_numberOut = new Counter(_parentModel, getName() + "." + "CountNumberOut", this);
		_statisticReporterInsert("CountNumberOut", _numberOut);
	} else if (_reportStatistics && _numberOut != nullptr) {
		_statisticReporterInsert("CountNumberOut", _numberOut);
	} else if (!_reportStatistics && _numberOut != nullptr) {
		_statisticReportersClear();
		_numberOut = nullptr;
	}
}
