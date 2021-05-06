/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Station.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:12
 */

#include "Station.h"
#include "Entity.h"
#include "Model.h"
#include "Attribute.h"

Station::Station(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Station>(), name) {
}

Station::~Station() {
	//_parentModel->elements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatNumberInStation);
	//_parentModel->elements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatTimeInStation);
}

std::string Station::show() {
	std::string msg = ModelElement::show() + ",enterIntoStationComponent=";
	if (_enterIntoStationComponent == nullptr)
		msg += "NULL";
	else
		msg += _enterIntoStationComponent->getName();
	return msg;
}

void Station::initBetweenReplications() {
	_cstatNumberInStation->getStatistics()->getCollector()->clear();
	_cstatTimeInStation->getStatistics()->getCollector()->clear();

}

void Station::enter(Entity* entity) {
	std::string attributeName = "Entity.ArrivalAt" + this->getName();
	trimwithin(attributeName);
	entity->setAttributeValue(attributeName, _parentModel->getSimulation()->getSimulatedTime());
	entity->setAttributeValue("Entity.Station", _id);
	_numberInStation++;
	if (_reportStatistics)
		this->_cstatNumberInStation->getStatistics()->getCollector()->addValue(_numberInStation);
}

void Station::leave(Entity* entity) {
	std::string attributeName = "Entity.ArrivalAt" + this->getName();
	trimwithin(attributeName);
	double arrivalTime = entity->getAttributeValue(attributeName);
	double timeInStation = _parentModel->getSimulation()->getSimulatedTime() - arrivalTime;
	entity->setAttributeValue("Entity.Station", 0.0);
	_numberInStation--;
	if (_reportStatistics) {
		_cstatNumberInStation->getStatistics()->getCollector()->addValue(_numberInStation);
		_cstatTimeInStation->getStatistics()->getCollector()->addValue(timeInStation);
		if (entity->getEntityType()->isReportStatistics())
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + ".TimeInStations")->getStatistics()->getCollector()->addValue(timeInStation); // \todo: should check if entitytype reports (?)
	}
}

void Station::setEnterIntoStationComponent(ModelComponent* _enterIntoStationComponent) {
	this->_enterIntoStationComponent = _enterIntoStationComponent;
}

ModelComponent* Station::getEnterIntoStationComponent() const {
	return _enterIntoStationComponent;
}

PluginInformation* Station::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Station>(), &Station::LoadInstance);
	return info;
}

ModelElement* Station::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Station* newElement = new Station(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Station::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelElement::_loadInstance(fields);
	if (res) {
		try {
		} catch (...) {
		}
	}
	return res;
}

std::map<std::string, std::string>* Station::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance(); //Util::TypeOf<Station>());
	return fields;
}

bool Station::_check(std::string* errorMessage) {
	/* include attributes needed */
	std::vector<std::string> neededNames = {"Entity.Station"};
	neededNames.insert(neededNames.begin(), "Entity.ArrivalAt" + this->getName());
	std::string neededName;
	for (unsigned int i = 0; i < neededNames.size(); i++) {
		neededName = neededNames[i];
		if (_parentModel->getElements()->getElement(Util::TypeOf<Attribute>(), neededName) == nullptr) {
			new Attribute(_parentModel, neededName);
		}
	}
	//
	return true;
}

void Station::_createInternalElements() {
	if (_reportStatistics) {
		if (_cstatNumberInStation == nullptr) {
			_cstatNumberInStation = new StatisticsCollector(_parentModel, getName() + "." + "NumberInStation", this);
			_cstatTimeInStation = new StatisticsCollector(_parentModel, getName() + "." + "TimeInStation", this);
			_childrenElements->insert({"NumberInStation", _cstatNumberInStation});
			_childrenElements->insert({"TimeInStation", _cstatTimeInStation});
			//
			// include StatisticsCollector needed in EntityType
			std::list<ModelElement*>* enttypes = _parentModel->getElements()->getElementList(Util::TypeOf<EntityType>())->list();
			for (std::list<ModelElement*>::iterator it = enttypes->begin(); it != enttypes->end(); it++) {
				if ((*it)->isReportStatistics())
					static_cast<EntityType*> ((*it))->addGetStatisticsCollector((*it)->getName() + ".TimeInStations"); // force create this CStat before simulation starts
			}

		}
	} else
		if (_cstatNumberInStation != nullptr) {
		_removeChildrenElements();
	}
}
