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

#include "Variable.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Variable::GetPluginInformation;
}
#endif

ModelDataDefinition* Variable::NewInstance(Model* model, std::string name) {
	return new Variable(model, name);
}

Variable::Variable(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Variable>(), name) {
	setName(name);
	SimulationControlDouble* propInitialValue = new SimulationControlDouble(
			std::bind(&Variable::getInitialValue, this, ""),
			std::bind(&Variable::setInitialValue, this, std::placeholders::_1, ""),
			Util::TypeOf<Variable>(), getName(), "InitialValue", "");
	// this Control was infered from getters and setters
	SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>* propScope =
			new SimulationControlGenericClass<ModelDataDefinition*, Model*, ModelDataDefinition>(
					_parentModel,
					std::bind(&Variable::get_scope, this),
					std::bind(&Variable::set_scope, this, std::placeholders::_1),
					Util::TypeOf<Variable>(), getName(), "Scope", "");
	_parentModel->getControls()->insert(propInitialValue);
	_parentModel->getControls()->insert(propScope);
	_addProperty(propInitialValue);
	_addProperty(propScope);
}

Variable::~Variable() {
	delete _values;
	_values = nullptr;
	delete _initialValues;
	_initialValues = nullptr;
}

std::string Variable::show() {
	return ModelDataDefinition::show() + ", values:" + this->_values->showValues();
}

PluginInformation* Variable::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Variable>(), &Variable::LoadInstance, &Variable::NewInstance);
	return info;
}

//double Variable::getValue() {
//	return getValue("");
//}

double Variable::getValue(std::string index) {
	return _values->value(index);
}

//void Variable::setValue(double value) {
//	setValue("", value);
//}

void Variable::setValue(double value,std::string index) {
	_values->setValue(value, index);
}

/*
double Variable::getInitialValue() {
	return getInitialValue("");
}

void Variable::setInitialValue(double value) {
	setInitialValue("", value);
}
*/

double Variable::getInitialValue(std::string index) {
	return _initialValues->value(index);
}

void Variable::setInitialValue(double value, std::string index) {
	_initialValues->setValue(value, index);
}

void Variable::setInitialValues(const std::vector<std::pair<std::string, double>> values) {
	for(std::pair<std::string,double> pair: values) {
		setInitialValue(pair.second, pair.first);
	}
}

void Variable::insertDimentionSize(unsigned int size) {
	// Dimension metadata applies both to initial values and runtime values.
	_initialValues->insertDimensionSize(size);
	_values->insertDimensionSize(size);
}

std::list<unsigned int>* Variable::getDimensionSizes() const {
	return _initialValues->dimensionSizes();
}

ModelDataDefinition* Variable::LoadInstance(Model* model, PersistenceRecord *fields) {
	Variable* newElement = new Variable(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Variable::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		// Preserve the legacy persistence layout while centralizing sparse parsing.
		this->_initialValues->loadDimensions(fields);
		this->_initialValues->loadValues(fields, "values", "valuePos", "value");
		*this->_values = *this->_initialValues;
	}
	return res;
}

void Variable::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	_initialValues->saveDimensions(fields, saveDefaultValues);
	_initialValues->saveValues(fields, "values", "valuePos", "value", saveDefaultValues);
}

bool Variable::_check(std::string& errorMessage) {
	errorMessage += "";
	return true;
}

void Variable::_initBetweenReplications() {
	*this->_values = *this->_initialValues;
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

SparseValueStore* Variable::getInitialValueStore() {
	return _initialValues;
}
