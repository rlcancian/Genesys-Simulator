/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Enter.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:14
 */

#include "Enter.h"
#include "Model.h"
#include "Counter.h"

Enter::Enter(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Enter>(), name) {
}

std::string Enter::show() {
	return ModelComponent::show() + ",station=" + this->_station->getName();
}

ModelComponent* Enter::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Enter* newComponent = new Enter(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Enter::setStation(Station* _station) {
	this->_station = _station;
	_station->setEnterIntoStationComponent(this);
}

Station* Enter::getStation() const {
	return _station;
}

void Enter::_execute(Entity* entity) {
	if (_reportStatistics)
		_numberIn->incCountValue();
	_station->enter(entity);
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

bool Enter::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		std::string stationName = LoadField(fields, "station", "");
		Station* station = dynamic_cast<Station*> (_parentModel->getElements()->getElement(Util::TypeOf<Station>(), stationName));
		this->_station = station;
	}
	return res;
}

void Enter::_initBetweenReplications() {
	_numberIn->clear();
}

std::map<std::string, std::string>* Enter::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	SaveField(fields, "station", _station->getName(), "");
	return fields;
}

bool Enter::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getElements()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
	return resultAll;
}

PluginInformation* Enter::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Enter>(), &Enter::LoadInstance);
	info->setReceiveTransfer(true);
	info->insertDynamicLibFileDependence("station.so");
	return info;
}

void Enter::_createInternalElements() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
			_childrenElements->insert({"CountNumberIn", _numberIn});
		}
	} else
		if (_numberIn != nullptr) {
		_removeChildrenElements();
	}
}