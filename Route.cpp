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
#include "Model.h"
#include "Attribute.h"
#include "Simulator.h"
#include "Sequence.h"

Route::Route(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Route>(), name) {
}

std::string Route::show() {
	std::string msg = ModelComponent::show() +
			",destinationType=" + std::to_string(static_cast<int> (this->_routeDestinationType)) +
			",timeExpression=" + this->_routeTimeExpression + " " + Util::StrTimeUnit(this->_routeTimeTimeUnit);
	if (_station != nullptr)
		msg += ",station=" + this->_station->getName();
	return msg;
}

ModelComponent* Route::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
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

void Route::_execute(Entity* entity) {
	Station* destinyStation = _station;
	if (_routeDestinationType == Route::DestinationType::BySequence) {
		unsigned int sequenceId = static_cast<unsigned int> (entity->getAttributeValue("Entity.Sequence"));
		unsigned int step = static_cast<unsigned int> (entity->getAttributeValue("Entity.SequenceStep"));
		Sequence* sequence = static_cast<Sequence*> (_parentModel->getElements()->getElement(Util::TypeOf<Sequence>(), sequenceId));
		SequenceStep* seqStep = sequence->getSteps()->getAtRank(step);
		if (seqStep == nullptr) {
			step = 0;
			seqStep = sequence->getSteps()->getAtRank(step);
			assert(seqStep != nullptr);
		}
		destinyStation = seqStep->getStation();
		for (std::list<std::string>::iterator it = seqStep->getAssignments()->begin(); it != seqStep->getAssignments()->end(); it++) {
			_parentModel->parseExpression((*it));
		}
		entity->setAttributeValue("Entity.SequenceStep", step + 1.0);
	}
	if (_reportStatistics)
		_numberIn->incCountValue();
	// adds the route time to the TransferTime statistics / attribute related to the Entitys
	double routeTime = _parentModel->parseExpression(_routeTimeExpression) * Util::TimeUnitConvert(_routeTimeTimeUnit, _parentModel->getInfos()->getReplicationLengthTimeUnit());
	if (entity->getEntityType()->isReportStatistics())
		entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + ".TransferTime")->getStatistics()->getCollector()->addValue(routeTime);
	entity->setAttributeValue("Entity.TotalTransferTime", entity->getAttributeValue("Entity.TotalTransferTime") + routeTime);
	if (routeTime > 0.0) {
		// calculates when this Entity will reach the end of this route and schedule this Event
		double routeEndTime = _parentModel->getSimulation()->getSimulatedTime() + routeTime;
		Event* newEvent = new Event(routeEndTime, entity, destinyStation->getEnterIntoStationComponent());
		_parentModel->getFutureEvents()->insert(newEvent);
		_parentModel->getTracer()->trace("End of route of entity " + std::to_string(entity->entityNumber()) + " to the component \"" + destinyStation->getEnterIntoStationComponent()->getName() + "\" was scheduled to time " + std::to_string(routeEndTime));
	} else {
		// send without delay
		_parentModel->sendEntityToComponent(entity, destinyStation->getEnterIntoStationComponent(), 0.0);
	}
}

bool Route::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_routeTimeExpression = (*fields->find("routeTimeExpression")).second;
		this->_routeTimeTimeUnit = static_cast<Util::TimeUnit> (std::stoi((*fields->find("routeTimeTimeUnit")).second));
		this->_routeDestinationType = static_cast<Route::DestinationType> (std::stoi((*fields->find("routeDestinationType")).second));
		std::string stationName = ((*(fields->find("stationName"))).second);
		Station* station = dynamic_cast<Station*> (_parentModel->getElements()->getElement(Util::TypeOf<Station>(), stationName));
		this->_station = station;
	}
	return res;
}

void Route::_initBetweenReplications() {
	_numberIn->clear();
}

std::map<std::string, std::string>* Route::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	fields->emplace("stationId", std::to_string(this->_station->getId()));
	fields->emplace("stationName", (this->_station->getName()));
	fields->emplace("routeTimeExpression", "\"" + this->_routeTimeExpression + "\"");
	fields->emplace("routeTimeTimeUnit", std::to_string(static_cast<int> (this->_routeTimeTimeUnit)));
	fields->emplace("routeDestinationType", std::to_string(static_cast<int> (this->_routeDestinationType)));
	return fields;
}

bool Route::_check(std::string* errorMessage) {
	//include attributes needed
	ElementManager* elements = _parentModel->getElements();
	std::vector<std::string> neededNames = {"Entity.TotalTransferTime", "Entity.Station"};
	std::string neededName;
	for (unsigned int i = 0; i < neededNames.size(); i++) {
		neededName = neededNames[i];
		if (elements->getElement(Util::TypeOf<Attribute>(), neededName) == nullptr) {
			Attribute* attr1 = new Attribute(_parentModel, neededName);
			elements->insert(attr1);
		}
	}
	// include StatisticsCollector needed in EntityType
	std::list<ModelElement*>* enttypes = elements->getElementList(Util::TypeOf<EntityType>())->list();
	for (std::list<ModelElement*>::iterator it = enttypes->begin(); it != enttypes->end(); it++) {
		if ((*it)->isReportStatistics())
			static_cast<EntityType*> ((*it))->addGetStatisticsCollector((*it)->getName() + ".TransferTime"); // force create this CStat before simulation starts
	}
	bool resultAll = true;
	resultAll &= _parentModel->checkExpression(_routeTimeExpression, "Route time expression", errorMessage);
	if (this->_routeDestinationType == Route::DestinationType::Station) {
		resultAll &= _parentModel->getElements()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
		if (resultAll) {
			resultAll &= _station->getEnterIntoStationComponent() != nullptr;
			if (!resultAll) {
				errorMessage->append("Station has no component to enter into it");
			}
		}
	}
	//_model->getParent()->getPluginManager()->
	return resultAll;
}

PluginInformation* Route::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Route>(), &Route::LoadInstance);
	info->setSendTransfer(true);
	info->insertDynamicLibFileDependence("station.so");
	return info;
}

void Route::_createInternalElements() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, _name + "." + "CountNumberIn", this);
			_childrenElements->insert({"CountNumberIn", _numberIn});
		}
	} else
		if (_numberIn != nullptr) {
		_removeChildrenElements();
	}
}
