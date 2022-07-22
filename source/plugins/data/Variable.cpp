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
#include "../../kernel/simulator/Plugin.h"

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
}

std::string Variable::show() {
	std::string text = "values:{";
	for (std::pair<std::string, double> var : * this->_values) {
		text += var.first + "=" + strTruncIfInt(std::to_string(var.second)) + ", ";
	}
	text = text.substr(0, text.length() - 2);
	text += "}";
	return ModelDataDefinition::show();
}

PluginInformation* Variable::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Variable>(), &Variable::LoadInstance, &Variable::NewInstance);
	return info;
}

double Variable::getValue() {
	return getValue("");
}

double Variable::getValue(std::string index) {
	std::map<std::string, double>::iterator it = _values->find(index);
	if (it == _values->end()) {
		return 0.0; // index does not exist. Assuming sparse matrix, it's zero.
	} else {
		return it->second;
	}
}

void Variable::setValue(double value) {
	setValue("", value);
}

void Variable::setValue(std::string index, double value) {
	std::map<std::string, double>::iterator it = _values->find(index);
	if (it == _values->end()) {
		// index does not exist. Create it.
		_values->insert({index, value}); //(std::pair<std::string, double>(index, value));
	} else {
		it->second = value;
	}
}

double Variable::getInitialValue() {
	return getInitialValue("");
}

void Variable::setInitialValue(double value) {
	setInitialValue("", value);
}

double Variable::getInitialValue(std::string index) {
	std::map<std::string, double>::iterator it = _initialValues->find(index);
	if (it == _initialValues->end()) {
		return 0.0; // index does not exist. Assuming sparse matrix, it's zero.
	} else {
		return it->second;
	}
}

void Variable::setInitialValue(std::string index, double value) {
	std::map<std::string, double>::iterator it = _initialValues->find(index);
	if (it == _initialValues->end()) {
		// index does not exist. Create it.
		_initialValues->insert(std::pair<std::string, double>(index, value));
	} else {
		it->second = value;
	}
}

void Variable::insertDimentionSize(unsigned int size) {
	_dimensionSizes->insert(_dimensionSizes->end(), size);
}

std::list<unsigned int>* Variable::getDimensionSizes() const {
	return _dimensionSizes;
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
		std::string pos;
		double value;
		unsigned int nv;
		nv = fields->loadField("dimensions", 0);
		for (unsigned int i = 0; i < nv; i++) {
			value = fields->loadField("dimension" + strIndex(i), 0);
			//this->_dimensionSizes->insert(value);
			this->insertDimentionSize(value);
		}
		nv = fields->loadField("values", 0);
		for (unsigned int i = 0; i < nv; i++) {
			pos = fields->loadField("valuePos" + strIndex(i), 0);
			value = fields->loadField("value" + strIndex(i), 0);
			this->_initialValues->emplace(pos, value);
		}
	}
	return res;
}

void Variable::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	unsigned int i = 0;
	fields->saveField("dimensions", _dimensionSizes->size(), 0u, saveDefaultValues);
	for (unsigned int dimension : *_dimensionSizes) {
		fields->saveField("dimension" + strIndex(i), dimension, 1u, saveDefaultValues);
	}
	fields->saveField("values", _initialValues->size(), 0);
	for (std::map<std::string, double>::iterator it = _initialValues->begin(); it != _initialValues->end(); it++, i++) {
		fields->saveField("valuePos" + strIndex(i), (*it).first, "0", saveDefaultValues);
		fields->saveField("value" + strIndex(i), (*it).second, 0.0, saveDefaultValues);
	}
}

bool Variable::_check(std::string* errorMessage) {
	*errorMessage += "";
	return true;
}

void Variable::_initBetweenReplications() {
	this->_values->clear();
	this->_values = this->_initialValues;
}

std::map<std::string, double> *Variable::getValues() const {
	return _values;
}
