/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Route.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#include <cassert>
#include "Route.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/Simulator.h"
#include "../data/Sequence.h"
#include "../data/Label.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Route::GetPluginInformation;
}
#endif

ModelDataDefinition* Route::NewInstance(Model* model, std::string name) {
	return new Route(model, name);
}

Route::Route(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Route>(), name) {
}

std::string Route::show() {
	std::string msg = ModelComponent::show() +
			",destinationType=" + std::to_string(static_cast<int> (this->_routeDestinationType)) +
			",timeExpression=" + this->_routeTimeExpression + " " + Util::StrTimeUnitShort(this->_routeTimeTimeUnit);
	if (_station != nullptr)
		msg += ",station=" + this->_station->getName();
	return msg;
}

ModelComponent* Route::LoadInstance(Model* model, PersistenceRecord *fields) {
	Route* newComponent = new Route(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Route::setStation(Station* _station) {
	this->_station = _station;
}

void Route::setStationName(std::string stationName) {
	ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName);
	if (data != nullptr) {
		_station = dynamic_cast<Station*> (data);
	} else {
		_station = _parentModel->getParentSimulator()->getPlugins()->newInstance<Station>(_parentModel, stationName);
	}
}

Station* Route::getStation() const {
	return _station;
}

void Route::setRouteTimeExpression(std::string _routeTimeExpression) {
	this->_routeTimeExpression = _routeTimeExpression;
}

std::string Route::getRouteTimeExpression() const {
	return _routeTimeExpression;
}

void Route::setRouteTimeTimeUnit(Util::TimeUnit _routeTimeTimeUnit) {
	this->_routeTimeTimeUnit = _routeTimeTimeUnit;
}

Util::TimeUnit Route::getRouteTimeTimeUnit() const {
	return _routeTimeTimeUnit;
}

void Route::setRouteDestinationType(DestinationType _routeDestinationType) {
	this->_routeDestinationType = _routeDestinationType;
}

Route::DestinationType Route::getRouteDestinationType() const {
	return _routeDestinationType;
}

void Route::setLabel(Label* _label) {
	this->_label = _label;
}

Label* Route::getLabel() const {
	return _label;
}

void Route::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	Station* destinyStation = _station;
	Label* destinyLabel = _label;
	if (_routeDestinationType == Route::DestinationType::Sequence) {
		unsigned int sequenceId = static_cast<unsigned int> (entity->getAttributeValue("Entity.Sequence"));
		unsigned int step = static_cast<unsigned int> (entity->getAttributeValue("Entity.SequenceStep"));
		Sequence* sequence = static_cast<Sequence*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Sequence>(), sequenceId));
		SequenceStep* seqStep = sequence->getSteps()->getAtRank(step);
		if (seqStep == nullptr) {
			step = 0;
			seqStep = sequence->getSteps()->getAtRank(step);
			assert(seqStep != nullptr);
		}
		destinyStation = seqStep->getStation(); // one of two is nullptr
		destinyLabel = seqStep->getLabel();
		for (Assignment* assignment : *seqStep->getAssignments()) {
			_parentModel->parseExpression(assignment->getDestination() + "=" + assignment->getExpression());
		}
		entity->setAttributeValue("Entity.SequenceStep", step + 1.0);
	}
	if (_reportStatistics)
		_numberIn->incCountValue();
	// adds the route time to the TransferTime statistics / attribute related to the Entitys
	double routeTime = _parentModel->parseExpression(_routeTimeExpression) * Util::TimeUnitConvert(_routeTimeTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
	if (entity->getEntityType()->isReportStatistics())
		entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + ".TransferTime")->getStatistics()->getCollector()->addValue(routeTime);
	entity->setAttributeValue("Entity.TotalTransferTime", entity->getAttributeValue("Entity.TotalTransferTime") + routeTime);
	if (routeTime > 0.0) {
		// calculates when this Entity will reach the end of this route and schedule this Event
		double routeEndTime = _parentModel->getSimulation()->getSimulatedTime() + routeTime;
		Event* newEvent;
		if (destinyStation != nullptr) {
			newEvent = new Event(routeEndTime, entity, destinyStation->getEnterIntoStationComponent());
			_parentModel->getTracer()->traceSimulation(this, "End of route of "/*entity " + std::to_string(entity->entityNumber())*/ + entity->getName() + " to the component \"" + destinyStation->getEnterIntoStationComponent()->getName() + "\" was scheduled to time " + std::to_string(routeEndTime));
		} else {// destination is Label
			newEvent = new Event(routeEndTime, entity, destinyLabel->getEnterIntoLabelComponent());
			_parentModel->getTracer()->traceSimulation(this, "End of route of "/*entity " + std::to_string(entity->entityNumber())*/ + entity->getName() + " to the component \"" + destinyLabel->getEnterIntoLabelComponent()->getName() + "\" was scheduled to time " + std::to_string(routeEndTime));
		}
		_parentModel->getFutureEvents()->insert(newEvent);
	} else {
		// send without delay
		if (destinyStation != nullptr) {
			_parentModel->sendEntityToComponent(entity, destinyStation->getEnterIntoStationComponent(), 0.0);
		} else {// destination is Label
			_parentModel->sendEntityToComponent(entity, destinyLabel->getEnterIntoLabelComponent(), 0.0);
		}
	}
}

//void Route::_initBetweenReplications() {}

bool Route::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_routeTimeExpression = fields->loadField("routeTimeExpression", DEFAULT.routeTimeExpression);
		this->_routeTimeTimeUnit = fields->loadField("routeTimeTimeUnit", DEFAULT.routeTimeTimeUnit);
		this->_routeDestinationType = static_cast<Route::DestinationType> (fields->loadField("destinationType", static_cast<int> (DEFAULT.routeDestinationType)));
		if (_routeDestinationType == DestinationType::Station) {
			std::string stationName = fields->loadField("station", "");
			Station* station = dynamic_cast<Station*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName));
			this->_station = station;
		}
		if (_routeDestinationType == DestinationType::Label) {
			std::string stationName = fields->loadField("label", "");
			Label* label = dynamic_cast<Label*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName));
			this->_label = label;
		}
	}
	return res;
}

void Route::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("destinationType", static_cast<int> (_routeDestinationType), static_cast<int> (DEFAULT.routeDestinationType), saveDefaultValues);
	if (_routeDestinationType == DestinationType::Station && _station != nullptr) {
		fields->saveField("station", _station->getName());
	}
	if (_routeDestinationType == DestinationType::Label && _label != nullptr) {
		fields->saveField("label", _label->getName());
	}
	fields->saveField("routeTimeExpression", _routeTimeExpression, DEFAULT.routeTimeExpression, saveDefaultValues);
	fields->saveField("routeTimeTimeUnit", _routeTimeTimeUnit, DEFAULT.routeTimeTimeUnit, saveDefaultValues);
}

PluginInformation* Route::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Route>(), &Route::LoadInstance, &Route::NewInstance);
	info->setSendTransfer(true);
	info->setCategory("Material Handling");
	info->insertDynamicLibFileDependence("station.so");
	info->insertDynamicLibFileDependence("sequence.so");
	info->insertDynamicLibFileDependence("label.so");
	std::string help = "The Route module transfers an entity to a specified station or the next station in the station visitation sequence defined for the entity.";
	help += " A delay time to transfer to the next station may be defined.";
	help += " When an entity enters the Route module, its Station attribute (Entity.Station) is set to the destination station.";
	help += " The entity is then sent to the destination station, using the route time specified.";
	help += " If the station destination is entered as By Sequence, the next station is determined by the entity’s sequence and step within the set (defined by special-purpose attributes Entity.Sequence and Entity.Jobstep, respectively).";
	help += " TYPICAL USES: (1) Send a part to its next processing station based on its routing slip;";
	help += " (2) Send an account balance call to an account agent; (3) Send restaurant customers to a specific table";
	info->setDescriptionHelp(help);
	return info;
}

void Route::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
			_internalDataInsert("CountNumberIn", _numberIn);
		}
	} else
		if (_numberIn != nullptr) {
		_internalDataClear();
	}
	_attachedAttributesInsert({"Entity.TotalTransferTime", "Entity.Station", "Entity.Sequence", "Entity.SequenceStep"});
	if (_parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
		if (_station == nullptr && this->_routeDestinationType == Route::DestinationType::Station) {
			_station = _parentModel->getParentSimulator()->getPlugins()->newInstance<Station>(_parentModel);
		}
		if (_label == nullptr && this->_routeDestinationType == Route::DestinationType::Label) {
			_label = _parentModel->getParentSimulator()->getPlugins()->newInstance<Label>(_parentModel);
		}
	}
	this->_attachedDataInsert("Station", _station);
	this->_attachedDataInsert("Label", _label);
}

bool Route::_check(std::string* errorMessage) {
	// include StatisticsCollector needed in EntityType
	std::list<ModelDataDefinition*>* enttypes = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
	for (ModelDataDefinition* modeldatum : *enttypes) {
		if (modeldatum->isReportStatistics())
			static_cast<EntityType*> (modeldatum)->addGetStatisticsCollector(modeldatum->getName() + ".TransferTime"); // force create this CStat before simulation starts
	}
	bool resultAll = true;
	resultAll &= _parentModel->checkExpression(_routeTimeExpression, "Route time expression", errorMessage);
	if (this->_routeDestinationType == Route::DestinationType::Station) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
		if (resultAll) {
			resultAll &= _station->getEnterIntoStationComponent() != nullptr;
			if (!resultAll) {
				errorMessage->append("Station has no component to enter into it");
			}
		}
	}
	if (this->_routeDestinationType == Route::DestinationType::Label) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Label>(), _station, "Label", errorMessage);
		if (resultAll) {
			resultAll &= _label->getEnterIntoLabelComponent() != nullptr;
			if (!resultAll) {
				errorMessage->append("Station has no component to enter into it");
			}
		}
	}
	return resultAll;
}
