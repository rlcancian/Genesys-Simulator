/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Leave.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:14
 */

#include "Leave.h"
#include "Model.h"

Leave::Leave(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Leave>(), name) {
}

std::string Leave::show() {
	return ModelComponent::show() + ",station=" + this->_station->getName();
}

ModelComponent* Leave::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Leave* newComponent = new Leave(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Leave::setStation(Station* _station) {
	this->_station = _station;
}

Station* Leave::getStation() const {
	return _station;
}

void Leave::_execute(Entity* entity) {
	if (_reportStatistics)
		_numberIn->incCountValue();
	_station->leave(entity);
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

bool Leave::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		std::string stationName = LoadField(fields, "station", "");
		Station* station = dynamic_cast<Station*> (_parentModel->getElements()->getElement(Util::TypeOf<Station>(), stationName));
		this->_station = station;
	}
	return res;
}

void Leave::_initBetweenReplications() {
	_numberIn->clear();
}

std::map<std::string, std::string>* Leave::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	SaveField(fields, "station", _station->getName(), "");
	return fields;
}

bool Leave::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getElements()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
	return resultAll;
}

PluginInformation* Leave::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Leave>(), &Leave::LoadInstance);
	info->insertDynamicLibFileDependence("station.so");
	return info;
}

void Leave::_createInternalElements() {
	if (_reportStatistics)
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
			_childrenElements->insert({"CountNumberIn", _numberIn});
		} else
			if (_numberIn != nullptr) {
			_removeChildrenElements();
		}
}

