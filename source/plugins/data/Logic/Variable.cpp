/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Variable.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 4 de Setembro de 2018, 18:28
 */

#include "plugins/data/Logic/Variable.h"
#include "../../../kernel/simulator/model/Model.h"

#include <exception>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Variable::GetPluginInformation;
}
#endif

ModelDataDefinition* Variable::NewInstance(Model* model, std::string name) {
	return new Variable(model, name);
}

Variable::Variable(Model* model, std::string name)
		: Attribute(model, name, Util::TypeOf<Variable>()) {
	// This control is specific to Variable and keeps the global runtime values.
	SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>* propScope =
			new SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>(
					_parentModel,
					std::bind(&Variable::get_scope, this),
					std::bind(&Variable::set_scope, this, std::placeholders::_1),
					Util::TypeOf<Variable>(), getName(), "Scope", "");
	_parentModel->getControls()->insert(propScope);
	_addSimulationControl(propScope);
}

Variable::~Variable() {
	delete _values;
	_values = nullptr;
}

std::string Variable::show() {
	return Attribute::show() + ", values:" + this->_values->showValues();
}

PluginInformation* Variable::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Variable>(), &Variable::LoadInstance, &Variable::NewInstance);
	info->setCategory("Logic");
	return info;
}

double Variable::getValue(std::string index) {
	return _values->value(index);
}

void Variable::setValue(double value, std::string index) {
	_values->setValue(value, index);
}

std::string Variable::getInitialValuesText() const {
	return Attribute::getInitialValuesText();
}

void Variable::setInitialValuesText(std::string valuesText) {
	Attribute::setInitialValuesText(valuesText);
	if (isInitialValuesTextValid()) {
		*this->_values = *getInitialValueStore();
	}
}

void Variable::insertDimentionSize(unsigned int size) {
	// Dimension metadata applies both to initial values and runtime values.
	Attribute::insertDimentionSize(size);
	_values->insertDimensionSize(size);
}

ModelDataDefinition* Variable::LoadInstance(Model* model, PersistenceRecord *fields) {
	Variable* newElement = new Variable(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load Variable instance: " + std::string(e.what()));
	}
	return newElement;
}

bool Variable::_loadInstance(PersistenceRecord *fields) {
	bool res = Attribute::_loadInstance(fields);
	if (res) {
		// Preserve the legacy runtime copy while reusing the attribute initial-values store.
		*this->_values = *getInitialValueStore();
	}
	return res;
}

void Variable::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	Attribute::_saveInstance(fields, saveDefaultValues);
}

bool Variable::_check(std::string& errorMessage) {
	return Attribute::_check(errorMessage);
}

void Variable::_initBetweenReplications() {
	*this->_values = *getInitialValueStore();
}

ModelDataDefinition* Variable::get_scope() {
	return scope;
}

void Variable::set_scope(ModelDataDefinition* const scope) {
	this->scope = scope;
}

std::map<std::string, double> *Variable::getValues() const {
	return _values->values();
}

SparseValueStore* Variable::getValueStore() {
	return _values;
}

// void Variable::_createInternalStatisticReporters() { }

// void Variable::_createEditableDataDefinitions() { }

// void Variable::_createAttachedAttributes() { }
