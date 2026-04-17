/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Attribute.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 25 de Setembro de 2018, 16:37
 */

#include "Attribute.h"
#include "Model.h"

//using namespace GenesysKernel;

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Attribute::GetPluginInformation;
}
#endif

ModelDataDefinition* Attribute::NewInstance(Model* model, std::string name) {
	return new Attribute(model, name);
}

Attribute::Attribute(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Attribute>(), name) {
	if (_parentModel != nullptr) {
		SimulationControlDouble* propInitialValue = new SimulationControlDouble(
				std::bind(&Attribute::getInitialValue, this, ""),
				std::bind(&Attribute::setInitialValue, this, std::placeholders::_1, ""),
				Util::TypeOf<Attribute>(), getName(), "InitialValue", "");
		_parentModel->getControls()->insert(propInitialValue);
		_addProperty(propInitialValue);
	}
}

Attribute::~Attribute() {
	delete _initialValues;
	_initialValues = nullptr;
}

std::string Attribute::show() {
	return ModelDataDefinition::show() + ", initialValues:" + _initialValues->showValues();
}

bool Attribute::_loadInstance(PersistenceRecord* fields) {
	const bool result = ModelDataDefinition::_loadInstance(fields);
	if (result) {
		// Attribute uses the same sparse persistence shape as Variable.
		_initialValues->loadDimensions(fields);
		_initialValues->loadValues(fields, "values", "valuePos", "value");
	}
	return result;
}

PluginInformation* Attribute::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Attribute>(), &Attribute::LoadInstance,
	                                                &Attribute::NewInstance);
	// @ToDo: (pequena alteração): Add Attribute description help
	info->setDescriptionHelp("");
	return info;
}

ModelDataDefinition* Attribute::LoadInstance(Model* model, PersistenceRecord* fields) {
	Attribute* newElement = new Attribute(model);
	try {
		newElement->_loadInstance(fields);
	}
	catch (const std::exception& e) {
	}
	return newElement;
}

void Attribute::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	_initialValues->saveDimensions(fields, saveDefaultValues);
	_initialValues->saveValues(fields, "values", "valuePos", "value", saveDefaultValues);
}

bool Attribute::_check(std::string& errorMessage) {
	errorMessage += "";
	return true;
}

double Attribute::getInitialValue(std::string index) {
	return _initialValues->value(index);
}

void Attribute::setInitialValue(double value, std::string index) {
	_initialValues->setValue(value, index);
}

void Attribute::setInitialValues(const std::vector<std::pair<std::string, double>> values) {
	for (const std::pair<std::string, double>& pair : values) {
		setInitialValue(pair.second, pair.first);
	}
}

void Attribute::insertDimentionSize(unsigned int size) {
	_initialValues->insertDimensionSize(size);
}

std::list<unsigned int>* Attribute::getDimensionSizes() const {
	return _initialValues->dimensionSizes();
}

std::map<std::string, double>* Attribute::getInitialValues() const {
	return _initialValues->values();
}

SparseValueStore* Attribute::getInitialValueStore() {
	return _initialValues;
}
