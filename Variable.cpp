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
#include "Plugin.h"

Variable::Variable(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Variable>(), name) {
	setName(name);
}

std::string Variable::show() {
	return ModelElement::show(); // \todo: include values
}

PluginInformation* Variable::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Variable>(), &Variable::LoadInstance);
	return info;
}

double Variable::value() {
	return value("");
}

double Variable::value(std::string index) {
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

double Variable::initialValue() {
	return initialValue("");
}

void Variable::setInitialValue(double value) {
	setInitialValue("", value);
}

double Variable::initialValue(std::string index) {
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

List<unsigned int>* Variable::dimensionSizes() const {
	return _dimensionSizes;
}

ModelElement* Variable::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Variable* newElement = new Variable(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Variable::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelElement::_loadInstance(fields);
	if (res) {
		///////////this->_numCols=std::stoi(LoadField(fields, "numCols"))).second);
		///////////this->_numRows=std::stoi(LoadField(fields, "numRows"))).second);
		unsigned int nv = LoadField(fields, "numValues", 0);
		std::string pos;
		double value;
		for (unsigned int i = 0; i < nv; i++) {
			pos = LoadField(fields, "pos" + std::to_string(i), 0);
			value = LoadField(fields, "value" + std::to_string(i), 0);
			this->_initialValues->emplace(pos, value);
		}
	}
	return res;
}

std::map<std::string, std::string>* Variable::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance(); //Util::TypeOf<Variable>());
	///////////SaveField(fields, "numCols" , std::to_string(this->_numCols));
	///////////SaveField(fields, "numRows" , std::to_string(this->_numRows));
	SaveField(fields, "numValues", _initialValues->size(), 0);
	unsigned int i = 0;
	for (std::map<std::string, double>::iterator it = this->_initialValues->begin(); it != _initialValues->end(); it++, i++) {
		SaveField(fields, "pos" + std::to_string(i), (*it).first, 0);
		SaveField(fields, "value" + std::to_string(i), (*it).second, 0.0);
	}
	return fields;
}

bool Variable::_check(std::string* errorMessage) {
	return true;
}

void Variable::_initBetweenReplications() {
	this->_values->clear();
	this->_values = this->_initialValues;
}
