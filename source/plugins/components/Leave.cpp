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
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Leave::GetPluginInformation;
}
#endif

ModelDataDefinition* Leave::NewInstance(Model* model, std::string name) {
	return new Leave(model, name);
}

Leave::Leave(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Leave>(), name) {
}

std::string Leave::show() {
	return ModelComponent::show() + ",station=" + this->_station->getName();
}

ModelComponent* Leave::LoadInstance(Model* model, PersistenceRecord *fields) {
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

void Leave::setStationName(std::string stationName) {
	ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName);
	if (data != nullptr) {
		_station = dynamic_cast<Station*> (data);
	} else {
		_station = _parentModel->getParentSimulator()->getPlugins()->newInstance<Station>(_parentModel, stationName);
	}
}

Station* Leave::getStation() const {
	return _station;
}

void Leave::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	if (_reportStatistics)
		_numberIn->incCountValue();
	_station->leave(entity);
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

//void Leave::_initBetweenReplications() {}

bool Leave::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		std::string stationName = fields->loadField("station", "");
		Station* station = dynamic_cast<Station*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName));
		this->_station = station;
	}
	return res;
}

void Leave::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	std::string text = "";
	if (_station != nullptr) {
		text = _station->getName();
	}
	fields->saveField("station", text, "", saveDefaultValues);
}

PluginInformation* Leave::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Leave>(), &Leave::LoadInstance, &Leave::NewInstance);
	info->insertDynamicLibFileDependence("station.so");
	info->setCategory("Material Handling");
	return info;
}

void Leave::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
			_internalDataInsert("CountNumberIn", _numberIn);
		} else
			if (_numberIn != nullptr) {
			_internalDataClear();
		}
	}
	_attachedDataInsert("Station", _station);
}

bool Leave::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
	return resultAll;
}