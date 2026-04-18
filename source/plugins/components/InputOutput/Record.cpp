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

#include "plugins/components/InputOutput/Record.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include <fstream>
#include <cstdio>
#include <iomanip>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Record::GetPluginInformation;
}
#endif

ModelDataDefinition* Record::NewInstance(Model* model, std::string name) {
	return new Record(model, name);
}

Record::Record(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Record>(), name) {
	SimulationControlGeneric<bool>* propTime = new SimulationControlGeneric<bool>(
									std::bind(&Record::getTimeDependent, this), std::bind(&Record::setTimeDependent, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "TimeDependent", "");
	SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getExpression, this), std::bind(&Record::setExpression, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "Expression", "");
	SimulationControlGeneric<std::string>* propExpressionName = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getExpressionName, this), std::bind(&Record::setExpressionName, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "ExpressionName", "");
	SimulationControlGeneric<std::string>* propFilename = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getFileName, this), std::bind(&Record::setFilename, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "Filename", "");

	_parentModel->getControls()->insert(propTime);
	_parentModel->getControls()->insert(propExpression);
	_parentModel->getControls()->insert(propExpressionName);
	_parentModel->getControls()->insert(propFilename);

	// setting properties
	_addProperty(propTime);
	_addProperty(propExpression);
	_addProperty(propExpressionName);
	_addProperty(propFilename);
}

Record::~Record() {
	_parentModel->getDataManager()->remove(Util::TypeOf<StatisticsCollector>(), _cstatExpression);
}

std::string Record::show() {
	return ModelComponent::show() +
			",expressionName=\"" + this->_expressionName + "\"" +
			",expression=\"" + _expression + "\"" +
			",filename=\"" + _filename + "\"" +
			",timeDependent=" + std::string(_timeDependent ? "true" : "false");
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

std::string Record::getFileName() const {
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
	if (_cstatExpression != nullptr) {
		_cstatExpression->getStatistics()->getCollector()->addValue(value);
	}
	if (_filename != "") {
		// @TODO: open and close for every data is not a good idea. Should open when replication starts and close when it finishes.
		std::ofstream file;
		file.open(_filename, std::ofstream::out | std::ofstream::app);
		if (file.is_open()) {
			file << std::setprecision(17);
			if (_timeDependent)
				file << _parentModel->getSimulation()->getSimulatedTime() << _separator << value << std::endl;
			else
				file << value << std::endl;
		}
		file.close();
	}
	traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "Recording value " + std::to_string(value));
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());

}

void Record::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("expression", this->_expression, "", saveDefaultValues);
	fields->saveField("expressionName", this->_expressionName, "", saveDefaultValues);
	fields->saveField("fileName", this->_filename, "", saveDefaultValues);
	fields->saveField("timeDependent", static_cast<int>(this->_timeDependent), static_cast<int>(DEFAULT.timeDependent), saveDefaultValues);
}

bool Record::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_expression = fields->loadField("expression", "");
		this->_expressionName = fields->loadField("expressionName", "");
		this->_filename = fields->loadField("fileName", "");
		if (this->_filename.empty()) {
			this->_filename = fields->loadField("filename", "");
		}
		if (this->_filename.empty()) {
			this->_filename = fields->loadField("Filename", "");
		}
		this->_timeDependent = fields->loadField("timeDependent", static_cast<int>(DEFAULT.timeDependent)) != 0;
	}
	return res;
}

void Record::_initBetweenReplications() {
	if (_filename.empty()) {
		return;
	}
		try {
			unsigned int numRep =  _parentModel->getSimulation()->getCurrentReplicationNumber();
			std::ofstream file;
			file.open(_filename, std::ofstream::app);
				if (file.is_open() && numRep==1) { // header
					file << "#RecordFormat=GenesysRecordV1" << std::endl;
					file << "#Expression=\""+_expression+"\", ExpressionName=\""+_expressionName+"\"" << std::endl;
					file << "#TimeDependent=" << (_timeDependent ? "true" : "false") << std::endl;
					file << "#Columns=" << (_timeDependent ? "time value" : "value") << std::endl;
				}
			if (file.is_open()) {
				file << "#ReplicationNumber=" <<numRep << std::endl; //"/" << _parentModel->getSimulation()->getNumberOfReplications() << std::endl;
			}
			file.close();
		} catch (...) {

		}
}

bool Record::_check(std::string& errorMessage) {
	// when cheking the model (before simulating it), remove the file if exists
	if (!_filename.empty()) {
		std::remove(_filename.c_str());
	}
	return _parentModel->checkExpression(_expression, "expression", errorMessage);
}

void Record::_createInternalAndAttachedData() {
	if (_reportStatistics && _cstatExpression == nullptr) {
		_cstatExpression = new StatisticsCollector(_parentModel, getName() + "." + _expressionName, this);
		//_parentModel->getDataDefinition()->insert(_cstatExpression);
	}
	if (_reportStatistics && _cstatExpression != nullptr) {
		_internalDataInsert(_expressionName, _cstatExpression);
	} else if (!_reportStatistics && _cstatExpression != nullptr) {
		this->_internalDataClear();
		_cstatExpression = nullptr;
	}
}

bool Record::getTimeDependent() const {
	return _timeDependent;
}

void Record::setTimeDependent(bool timeDependent) {
	_timeDependent = timeDependent;
}

PluginInformation* Record::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Record>(), &Record::LoadInstance, &Record::NewInstance);
	info->setCategory("InputOutput");
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
