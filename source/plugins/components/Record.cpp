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
#include "../../kernel/simulator/Model.h"
#include <fstream>
#include <cstdio>
#include <iostream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Record::GetPluginInformation;
}
#endif

ModelDataDefinition* Record::NewInstance(Model* model, std::string name) {
	return new Record(model, name);
}

Record::Record(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Record>(), name) {
}

Record::~Record() {
	_parentModel->getDataManager()->remove(Util::TypeOf<StatisticsCollector>(), _cstatExpression);
}

std::string Record::show() {
	return ModelComponent::show() +
			",expressionName=\"" + this->_expressionName + "\"" +
			",expression=\"" + _expression + "\"" +
			"filename=\"" + _filename + "\"";
}

void Record::setExpressionName(std::string expressionName) {
	this->_expressionName = expressionName;
	if (_cstatExpression != nullptr)
		this->_cstatExpression->setName(getName() + "." + expressionName);
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

void Record::setExpression(const std::string expression) {
	this->_expression = expression;
}

std::string Record::getExpression() const {
	return _expression;
}

void Record::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double value = _parentModel->parseExpression(_expression);
	_cstatExpression->getStatistics()->getCollector()->addValue(value);
	if (_filename != "") {
		// @TODO: open and close for every data is not a good idea. Should open when replication starts and close when it finishes.
		std::ofstream file;
		file.open(_filename, std::ofstream::out | std::ofstream::app);
		file << value << std::endl;
		file.close();
	}
	_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "Recording value " + std::to_string(value));
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());

}

void Record::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("expression", this->_expression, "", saveDefaultValues);
	fields->saveField("expressionName", this->_expressionName, "", saveDefaultValues);
	fields->saveField("fileName", this->_filename, "", saveDefaultValues);
}

bool Record::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_expression = fields->loadField("expression", "");
		this->_expressionName = fields->loadField("expressionName", "");
		this->_filename = fields->loadField("fileName", "");
	}
	return res;
}

//void Record::_initBetweenReplications() {}

bool Record::_check(std::string* errorMessage) {
	// when cheking the model (before simulating it), remove the file if exists
	std::remove(_filename.c_str());
	return _parentModel->checkExpression(_expression, "expression", errorMessage);
}

void Record::_createInternalAndAttachedData() {
	if (_reportStatistics && _cstatExpression == nullptr) {
		_cstatExpression = new StatisticsCollector(_parentModel, getName() + "." + _expressionName, this);
		//_parentModel->getDataDefinition()->insert(_cstatExpression);
		_internalDataInsert(_expressionName, _cstatExpression);
	} else if (!_reportStatistics && _cstatExpression != nullptr) {
		this->_internalDataClear();
		_cstatExpression = nullptr;
	}
}

PluginInformation* Record::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Record>(), &Record::LoadInstance, &Record::NewInstance);
	info->setCategory("Input Output");
	return info;
}

ModelComponent* Record::LoadInstance(Model* model, PersistenceRecord *fields) {
	Record* newComponent = new Record(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;

}

