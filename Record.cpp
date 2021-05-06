/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Record.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 9 de Agosto de 2018, 13:52
 */

#include "Record.h"
#include "Model.h"
#include <fstream>
#include <cstdio>
#include <iostream>

Record::Record(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Record>(), name) {
	_cstatExpression = new StatisticsCollector(_parentModel, _expressionName, this);
	_parentModel->getElements()->insert(_cstatExpression);
}

Record::~Record() {
	_parentModel->getElements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatExpression);
}

std::string Record::show() {
	return ModelComponent::show() +
			",expressionName=\"" + this->_expressionName + "\"" +
			",expression=\"" + _expression + "\"" +
			"filename=\"" + _filename + "\"";
}

void Record::setExpressionName(std::string expressionName) {
	this->_expressionName = expressionName;
	this->_cstatExpression->setName(expressionName);
}

std::string Record::getExpressionName() const {
	return _expressionName;
}

StatisticsCollector* Record::getCstatExpression() const {
	return _cstatExpression;
}

void Record::setFilename(std::string filename) {
	this->_filename = filename;
}

std::string Record::getFilename() const {
	return _filename;
}

void Record::setExpression(std::string expression) {
	this->_expression = expression;
}

std::string Record::getExpression() const {
	return _expression;
}

void Record::_execute(Entity* entity) {
	double value = _parentModel->parseExpression(_expression);
	_cstatExpression->getStatistics()->getCollector()->addValue(value);
	std::ofstream file;
	file.open(_filename, std::ofstream::out | std::ofstream::app);
	file << value << std::endl;
	file.close(); // \todo: open and close for every data is not a good idea. Should open when replication starts and close when it finishes.
	_parentModel->getTracer()->traceSimulation(_parentModel->getSimulation()->getSimulatedTime(), entity, this, "Recording value " + std::to_string(value));
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);

}

std::map<std::string, std::string>* Record::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance(); //Util::TypeOf<Record>());
	SaveField(fields, "expression0", this->_expression, "");
	SaveField(fields, "expressionName0", this->_expressionName, "");
	SaveField(fields, "fileName0", this->_filename, "");
	return fields;
}

bool Record::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_expression = LoadField(fields, "expression0", "");
		this->_expressionName = LoadField(fields, "expressionName0", "");
		this->_filename = LoadField(fields, "fileName0", "");
	}
	return res;
}

void Record::_initBetweenReplications() {
}

bool Record::_check(std::string* errorMessage) {
	// when cheking the model (before simulating it), remove the file if exists
	std::remove(_filename.c_str());
	return _parentModel->checkExpression(_expression, "expression", errorMessage);
}

PluginInformation* Record::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Record>(), &Record::LoadInstance);
	return info;
}

ModelComponent* Record::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Record* newComponent = new Record(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;

}

