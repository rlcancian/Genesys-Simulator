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
#include <algorithm>
#include <cctype>
#include <fstream>
#include <cstdio>
#include <iomanip>

namespace {
	static std::string trim(const std::string& text) {
		const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		});
		const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		}).base();
		if (first >= last) {
			return "";
		}
		return std::string(first, last);
	}

	static std::string lower(std::string text) {
		std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		return text;
	}

	static std::string canonicalVariableType(const std::string& variableType) {
		const std::string normalized = lower(trim(variableType));
		if (normalized == "discrete numeric") {
			return "Discrete numeric";
		}
		return "Continuous numeric";
	}

	static std::string quotedRecordValue(const std::string& value) {
		std::string escaped = "\"";
		for (char ch : value) {
			if (ch == '"') {
				escaped += "\"\"";
			} else {
				escaped += ch;
			}
		}
		escaped += "\"";
		return escaped;
	}
}

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
	SimulationControlGeneric<std::string>* propDatasetName = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getDatasetName, this), std::bind(&Record::setDatasetName, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "DatasetName", "");
	SimulationControlGeneric<std::string>* propRandomVariableName = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getRandomVariableName, this), std::bind(&Record::setRandomVariableName, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "RandomVariableName", "");
	SimulationControlGeneric<std::string>* propVariableType = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getVariableType, this), std::bind(&Record::setVariableType, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "VariableType", "");
	SimulationControlGeneric<std::string>* propDescription = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getDatasetDescription, this), std::bind(&Record::setDatasetDescription, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "Description", "");
	SimulationControlGeneric<std::string>* propFilename = new SimulationControlGeneric<std::string>(
									std::bind(&Record::getFileName, this), std::bind(&Record::setFilename, this, std::placeholders::_1),
									Util::TypeOf<Record>(), getName(), "Filename", "");

	_parentModel->getControls()->insert(propTime);
	_parentModel->getControls()->insert(propExpression);
	_parentModel->getControls()->insert(propExpressionName);
	_parentModel->getControls()->insert(propDatasetName);
	_parentModel->getControls()->insert(propRandomVariableName);
	_parentModel->getControls()->insert(propVariableType);
	_parentModel->getControls()->insert(propDescription);
	_parentModel->getControls()->insert(propFilename);

	// setting properties
	_addSimulationControl(propTime);
	_addSimulationControl(propExpression);
	_addSimulationControl(propExpressionName);
	_addSimulationControl(propDatasetName);
	_addSimulationControl(propRandomVariableName);
	_addSimulationControl(propVariableType);
	_addSimulationControl(propDescription);
	_addSimulationControl(propFilename);
}

Record::~Record() {
	_parentModel->getDataManager()->remove(Util::TypeOf<StatisticsCollector>(), _cstatExpression);
}

std::string Record::show() {
	return ModelComponent::show() +
			",expressionName=\"" + this->_expressionName + "\"" +
			",datasetName=\"" + this->_datasetName + "\"" +
			",randomVariableName=\"" + this->_randomVariableName + "\"" +
			",variableType=\"" + this->_variableType + "\"" +
			",description=\"" + this->_description + "\"" +
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

void Record::setDatasetName(std::string datasetName) {
	this->_datasetName = datasetName;
}

std::string Record::getDatasetName() const {
	return _datasetName;
}

void Record::setRandomVariableName(std::string randomVariableName) {
	this->_randomVariableName = randomVariableName;
}

std::string Record::getRandomVariableName() const {
	return _randomVariableName;
}

void Record::setVariableType(std::string variableType) {
	this->_variableType = canonicalVariableType(variableType);
}

std::string Record::getVariableType() const {
	return _variableType;
}

void Record::setDatasetDescription(std::string description) {
	this->_description = description;
}

std::string Record::getDatasetDescription() const {
	return _description;
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
	fields->saveField("datasetName", this->_datasetName, DEFAULT.datasetName, saveDefaultValues);
	fields->saveField("randomVariableName", this->_randomVariableName, DEFAULT.randomVariableName, saveDefaultValues);
	fields->saveField("variableType", this->_variableType, DEFAULT.variableType, saveDefaultValues);
	fields->saveField("description", this->_description, DEFAULT.description, saveDefaultValues);
	fields->saveField("fileName", this->_filename, "", saveDefaultValues);
	fields->saveField("timeDependent", static_cast<int>(this->_timeDependent), static_cast<int>(DEFAULT.timeDependent), saveDefaultValues);
}

bool Record::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_expression = fields->loadField("expression", "");
		this->_expressionName = fields->loadField("expressionName", "");
		this->_datasetName = fields->loadField("datasetName", DEFAULT.datasetName);
		this->_randomVariableName = fields->loadField("randomVariableName", DEFAULT.randomVariableName);
		this->_variableType = canonicalVariableType(fields->loadField("variableType", DEFAULT.variableType));
		this->_description = fields->loadField("description", DEFAULT.description);
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
					const std::string datasetName = _datasetName.empty() ? _expressionName : _datasetName;
					const std::string randomVariableName = _randomVariableName.empty() ? _expressionName : _randomVariableName;
					// Record remains append-friendly text instead of CSV; these metadata lines make the dataset semantic.
					file << "#Format=" << quotedRecordValue("GenesysRecordDataset") << std::endl;
					file << "#FormatVersion=" << quotedRecordValue("1") << std::endl;
					file << "#DatasetName=" << quotedRecordValue(datasetName) << std::endl;
					file << "#RandomVariableName=" << quotedRecordValue(randomVariableName) << std::endl;
					file << "#VariableType=" << quotedRecordValue(_variableType) << std::endl;
					file << "#Description=" << quotedRecordValue(_description) << std::endl;
					file << "#Source=" << quotedRecordValue("Genesys Record") << std::endl;
					file << "#Expression=" << quotedRecordValue(_expression) << std::endl;
					file << "#ExpressionName=" << quotedRecordValue(_expressionName) << std::endl; // Legacy compatibility field.
					file << "#TimeDependent=" << (_timeDependent ? "true" : "false") << std::endl;
					file << "#Columns=" << quotedRecordValue(_timeDependent ? "time value" : "value") << std::endl;
				}
			if (file.is_open()) {
				file << "#ReplicationNumber=" <<numRep << std::endl; //"/" << _parentModel->getSimulation()->getNumberOfReplications() << std::endl;
			}
			file.close();
		} catch (...) {

		}
}

bool Record::_check(std::string& errorMessage) {
	// Checking must validate the expression only. Deleting the output file here is destructive because
	// users may be checking old Record datasets before importing them into the Data Analyzer.
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
