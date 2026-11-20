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

#include "plugins/components/Transfer/Enter.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Enter::GetPluginInformation;
}
#endif

ModelDataDefinition* Enter::NewInstance(Model* model, std::string name) {
	return new Enter(model, name);
}

Enter::Enter(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Enter>(), name) {
	SimulationControlGenericClass<Station*, Model*, Station>* propStation = new SimulationControlGenericClass<Station*, Model*, Station>(
									_parentModel,
									std::bind(&Enter::getStation, this), std::bind(&Enter::setStation, this, std::placeholders::_1),
									Util::TypeOf<Enter>(), getName(), "Station", "", false, true, false, nullptr, false, false, nullptr, nullptr, nullptr, true);

	_parentModel->getControls()->insert(propStation);

	// setting properties
	_addSimulationControl(propStation);
}

Enter::~Enter() {
	setStation(nullptr);
}

std::string Enter::show() {
	return ModelComponent::show() + ",station=" + (_station != nullptr ? _station->getName() : "");
}

ModelComponent* Enter::LoadInstance(Model* model, PersistenceRecord *fields) {
	Enter* newComponent = new Enter(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Enter instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Enter::setStation(Station* station) {
	if (_station == station) {
		if (station != nullptr && station->getEnterIntoStationComponent() == nullptr) {
			station->setEnterIntoStationComponent(this);
		}
		return;
	}
	if (station != nullptr && station->getEnterIntoStationComponent() != nullptr &&
		station->getEnterIntoStationComponent() != this) {
		traceError("Station \"" + station->getName() + "\" is already associated with another Enter component");
		return;
	}
	Station* previousStation = _station;
	_station = station;
	if (previousStation != nullptr && previousStation->getEnterIntoStationComponent() == this) {
		previousStation->setEnterIntoStationComponent(nullptr);
	}
	if (_station != nullptr) {
		_station->setEnterIntoStationComponent(this);
	}
}

void Enter::setStationName(std::string stationName) {
	ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName);
	setStation(dynamic_cast<Station*> (data));
}

Station* Enter::getStation() const {
	return _station;
}

void Enter::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	if (_station == nullptr) {
		traceError("Enter \"" + getName() + "\" has no Station configured");
		return;
	}
	if (_reportStatistics)
		_numberIn->incCountValue();
	_station->enter(entity);
	traceSimulation(this, "Entity entered Station \"" + _station->getName() + "\"", TraceManager::Level::L7_internal);
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool Enter::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		std::string stationName = fields->loadField("station", "");
		Station* station = dynamic_cast<Station*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName));
		if (station != nullptr) {
			setStation(station);
		} else if (!stationName.empty()) {
			// Dangling reference: the named Station is missing from the file being loaded.
			// Report it instead of silently leaving _station null (later dereferenced
			// unconditionally by show()/_onDispatchEvent()).
			this->traceError("Enter could not resolve referenced Station named '" + stationName + "' on loading");
			res = false;
		}
	}
	return res;
}

//void Enter::_initBetweenReplications() {}

void Enter::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	std::string text = "";
	if (_station != nullptr) {
		text = _station->getName();
	}
	fields->saveField("station", text, "", saveDefaultValues);
}

PluginInformation* Enter::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Enter>(), &Enter::LoadInstance, &Enter::NewInstance);
	info->setReceiveTransfer(true);
	info->setCategory("Transfer");
	info->insertDynamicLibFileDependence("station.so");
	info->setDescriptionHelp("//@TODO");
	return info;
}

//void Enter::_createAttachedAttributes() {}

bool Enter::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
	if (_station != nullptr && _station->getEnterIntoStationComponent() != this) {
		errorMessage += "Station is associated with another Enter component. ";
		resultAll = false;
	}
	return resultAll;
}

void Enter::_createInternalStatisticReporters() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
		}
		if (_numberIn != nullptr) {
			_statisticReporterInsert("CountNumberIn", _numberIn);
		}
	} else {
			_statisticReportersClear();
		_numberIn = nullptr;
	}
}

void Enter::_createEditableDataDefinitions() {
	if (_station != nullptr) {
		_optionalEditableDataDefinitionInsert("Station", _station);
	} else {
		_optionalEditableDataDefinitionRemove("Station");
	}

}
