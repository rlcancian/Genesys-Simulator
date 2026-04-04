/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Storage.cpp
 * Author: rlcancian
 *
 * Created on 20 de Storageembro de 2019, 20:06
 */

#include "Storage.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Storage::GetPluginInformation;
}
#endif

ModelDataDefinition* Storage::NewInstance(Model* model, std::string name) {
	return new Storage(model, name);
}

Storage::Storage(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Storage>(), name) {
	//controls
	_parentModel->getControls()->insert(new SimulationControlDouble(
					 std::bind(&Storage::getCapacity, this),
					 std::bind(&Storage::setCapacity, this, std::placeholders::_1),
					 Util::TypeOf<Storage>(), getName(), "Capacity"));
	_parentModel->getControls()->insert(new SimulationControlDouble(
					 std::bind(&Storage::getTotalArea, this),
					 std::bind(&Storage::setTotalArea, this, std::placeholders::_1),
					 Util::TypeOf<Storage>(), getName(), "TotalArea"));
	_parentModel->getControls()->insert(new SimulationControlDouble(
					 std::bind(&Storage::getUnitsPerArea, this),
					 std::bind(&Storage::setUnitsPerArea, this, std::placeholders::_1),
					 Util::TypeOf<Storage>(), getName(), "UnitsPerArea"));
}

std::string Storage::show() {
	return ModelDataDefinition::show() +
			"";
}

void Storage::setTotalArea(double _totalArea) {
	this->_totalArea = _totalArea;
}

double Storage::getTotalArea() const {
	return _totalArea;
}

void Storage::setCapacity(unsigned int _capacity) {
	this->_capacity = _capacity;
}

unsigned int Storage::getCapacity() const {
	return _capacity;
}

void Storage::setUnitsPerArea(double _unitsPerArea) {
	this->_unitsPerArea = _unitsPerArea;
}

double Storage::getUnitsPerArea() const {
	return _unitsPerArea;
}

PluginInformation* Storage::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Storage>(), &Storage::LoadInstance, &Storage::NewInstance);
	return info;
}

ModelDataDefinition* Storage::LoadInstance(Model* model, PersistenceRecord *fields) {
	Storage* newElement = new Storage(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Storage::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			this->_capacity = fields->loadField("capacity", DEFAULT.capacity);
			this->_totalArea = fields->loadField("totalArea", DEFAULT.totalArea);
			this->_unitsPerArea = fields->loadField("unitPerArea", DEFAULT.unitsPerArea);
		} catch (...) {
		}
	}
	return res;
}

void Storage::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Persist Storage parameters using symmetric keys from _loadInstance.
	 */
	fields->saveField("capacity", _capacity, DEFAULT.capacity, saveDefaultValues);
	fields->saveField("totalArea", _totalArea, DEFAULT.totalArea, saveDefaultValues);
	fields->saveField("unitPerArea", _unitsPerArea, DEFAULT.unitsPerArea, saveDefaultValues);
}

bool Storage::_check(std::string& errorMessage) {
	bool resultAll = true;
	// resultAll |= ...
	errorMessage += "";
	return resultAll;
}

ParserChangesInformation* Storage::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}
