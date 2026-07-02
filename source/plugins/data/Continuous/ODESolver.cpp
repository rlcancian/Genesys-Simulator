/*
 * File:   ODESolver.cpp
 * Author: GenESyS
 *
 * Created on 13 de maio de 2026
 */

#include "plugins/data/Continuous/ODESolver.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/Model.h"
#include "plugins/data/Logic/Variable.h"
#include "kernel/util/Util.h"

#include <sstream>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ODESolver::GetPluginInformation;
}
#endif

ModelDataDefinition* ODESolver::NewInstance(Model* model, std::string name) {
	return new ODESolver(model, name);
}

ODESolver::ODESolver(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<ODESolver>(), name) {
	// Initialize state values vector to match state variable names size
	_stateValues.resize(_stateVariableNames.size(), 0.0);
	_initialStateValues.resize(_stateVariableNames.size(), 0.0);
}

// static

ModelDataDefinition* ODESolver::LoadInstance(Model* model, PersistenceRecord *fields) {
	ODESolver* newElement = new ODESolver(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		// Handle exception
	}
	return newElement;
}

PluginInformation* ODESolver::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ODESolver>(), &ODESolver::LoadInstance, &ODESolver::NewInstance);
	info->setCategory("Continuous");
	info->setDescriptionHelp("ODE System Solver - Integrates systems of ordinary differential equations using Runge-Kutta 4th order method.");
	return info;
}

void ODESolver::SaveInstance(PersistenceRecord *fields, ModelDataDefinition* dataDef) {
	ODESolver* solver = dynamic_cast<ODESolver*>(dataDef);
	if (solver != nullptr) {
		solver->_saveInstance(fields, false);
	}
}

bool ODESolver::Check(ModelDataDefinition* dataDef, std::string& errorMessage) {
	ODESolver* solver = dynamic_cast<ODESolver*>(dataDef);
	if (solver != nullptr) {
		return solver->_check(errorMessage);
	}
	return false;
}

void ODESolver::InitBetweenReplications(ModelDataDefinition* dataDef) {
	ODESolver* solver = dynamic_cast<ODESolver*>(dataDef);
	if (solver != nullptr) {
		solver->_initBetweenReplications();
	}
}

void ODESolver::CreateInternalData(ModelDataDefinition* dataDef) {
	ODESolver* solver = dynamic_cast<ODESolver*>(dataDef);
	if (solver != nullptr) {
		solver->_createEditableDataDefinitions();
	}
}

//

std::string ODESolver::show() {
	std::string stateVars = "";
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		stateVars += _stateVariableNames[i] + (i < _stateVariableNames.size() - 1 ? "," : "");
	}
	return ModelDataDefinition::show() +
			",timeVariableName=\"" + _timeVariableName + "\"" +
			",stateVariables=[" + stateVars + "]" +
			",step=" + std::to_string(_step) +
			",precision=" + std::to_string(_precision) +
			",maxSteps=" + std::to_string(_maxSteps) +
			",currentTime=" + std::to_string(_currentTime);
}

// gets & sets

void ODESolver::setTimeVariableName(std::string timeVariableName) {
	this->_timeVariableName = timeVariableName;
}

std::string ODESolver::getTimeVariableName() const {
	return _timeVariableName;
}

void ODESolver::setStateVariableNames(std::vector<std::string> stateVariableNames) {
	this->_stateVariableNames = stateVariableNames;
	_stateValues.resize(stateVariableNames.size(), 0.0);
	_initialStateValues.resize(stateVariableNames.size(), 0.0);
}

std::vector<std::string> ODESolver::getStateVariableNames() const {
	return _stateVariableNames;
}

void ODESolver::setEquationExpressions(std::vector<std::string> equationExpressions) {
	this->_equationExpressions = equationExpressions;
}

std::vector<std::string> ODESolver::getEquationExpressions() const {
	return _equationExpressions;
}

void ODESolver::setStep(double step) {
	this->_step = step;
}

double ODESolver::getStep() const {
	return _step;
}

void ODESolver::setPrecision(double precision) {
	this->_precision = precision;
}

double ODESolver::getPrecision() const {
	return _precision;
}

void ODESolver::setMaxSteps(int maxSteps) {
	this->_maxSteps = maxSteps;
}

int ODESolver::getMaxSteps() const {
	return _maxSteps;
}

void ODESolver::setCurrentTime(double currentTime) {
	this->_currentTime = currentTime;
}

double ODESolver::getCurrentTime() const {
	return _currentTime;
}

void ODESolver::setStateValues(std::vector<double> stateValues) {
	this->_stateValues = stateValues;
}

std::vector<double> ODESolver::getStateValues() const {
	return _stateValues;
}

void ODESolver::setInitialStateValues(std::vector<double> initialStateValues) {
	this->_initialStateValues = initialStateValues;
	_initialStateValues.resize(_stateVariableNames.size(), 0.0);
}

std::vector<double> ODESolver::getInitialStateValues() const {
	return _initialStateValues;
}

// new methods

void ODESolver::integrate(double targetTime) {
	if (_stateVariableNames.empty() || _equationExpressions.empty()) {
		return;
	}
	
	// Local ODE system implementation
	struct ODESystem : public OdeSystem_if {
		ODESolver* _solver;
		
		ODESystem(ODESolver* solver) : _solver(solver) {}
		
		unsigned int dimension() const override {
			return _solver->_stateVariableNames.size();
		}
		
		void evaluate(double t, const double* y, double* dydt) const override {
			Variable* timeVar = dynamic_cast<Variable*>(_solver->_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _solver->_timeVariableName));
			if (timeVar != nullptr) {
				timeVar->setValue(t);
			}

			// All state variables must be written before any expression is parsed:
			// the parser reads the global Variable store, so any stale value from
			// the previous k-stage would silently corrupt the derivative evaluation.
			for (unsigned int i = 0; i < _solver->_stateVariableNames.size(); i++) {
				Variable* stateVar = dynamic_cast<Variable*>(_solver->_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _solver->_stateVariableNames[i]));
				if (stateVar != nullptr) {
					stateVar->setValue(y[i]);
				}
			}

			for (unsigned int i = 0; i < _solver->_stateVariableNames.size(); i++) {
				try {
					dydt[i] = _solver->_parentModel->parseExpression(_solver->_equationExpressions[i]);
				} catch (const std::exception& e) {
					dydt[i] = 0.0;
				}
			}
		}
	};
	
	ODESystem odeSystem(this);
	std::vector<double> y = _stateValues;
	std::vector<double> yNext(_stateValues.size());
	int steps = 0;
	
	// Main integration loop with fixed step size
	while (_currentTime + _step <= targetTime && steps < _maxSteps) {
		bool success = _solver.advance(odeSystem, _currentTime, _step, y.data(), yNext.data());
		if (!success) {
			break;
		}
		
		_currentTime += _step;
		y = yNext;
		steps++;
	}
	
	// Final partial step if needed
	if (_currentTime < targetTime && steps < _maxSteps) {
		double lastStep = targetTime - _currentTime;
		bool success = _solver.advance(odeSystem, _currentTime, lastStep, y.data(), yNext.data());
		if (success) {
			_currentTime = targetTime;
			y = yNext;
		}
	}
	
	// Update internal state and model variables
	_stateValues = y;
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		Variable* stateVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _stateVariableNames[i]));
		if (stateVar != nullptr) {
			stateVar->setValue(y[i]);
		}
	}
	
	// Update time variable in model
	Variable* timeVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _timeVariableName));
	if (timeVar != nullptr) {
		timeVar->setValue(_currentTime);
	}
}

void ODESolver::resetState() {
	_currentTime = 0.0;
	_stateValues = _initialStateValues;
	writeStateToVariables();
}

double ODESolver::getStateValue(unsigned int index) const {
	if (index < _stateValues.size()) {
		return _stateValues[index];
	}
	return 0.0;
}

void ODESolver::setStateValue(unsigned int index, double value) {
	if (index < _stateValues.size()) {
		_stateValues[index] = value;
		
		// Update model variable
		if (index < _stateVariableNames.size()) {
			Variable* stateVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _stateVariableNames[index]));
			if (stateVar != nullptr) {
				stateVar->setValue(value);
			}
		}
	}
}

// must be overriden

bool ODESolver::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_timeVariableName = fields->loadField("timeVariableName", DEFAULT.timeVariableName);
		_step = fields->loadField("step", DEFAULT.step);
		_precision = fields->loadField("precision", DEFAULT.precision);
		_maxSteps = fields->loadField("maxSteps", DEFAULT.maxSteps);
		_currentTime = fields->loadField("currentTime", DEFAULT.currentTime);
		
		// Load state variable names as ";" delimited string
		std::string stateVarsStr = fields->loadField("stateVariableNames", "");
		if (!stateVarsStr.empty()) {
			_stateVariableNames.clear();
			size_t pos = 0;
			size_t delimPos;
			while ((delimPos = stateVarsStr.find(";", pos)) != std::string::npos) {
				_stateVariableNames.push_back(stateVarsStr.substr(pos, delimPos - pos));
				pos = delimPos + 1;
			}
			if (pos < stateVarsStr.length()) {
				_stateVariableNames.push_back(stateVarsStr.substr(pos));
			}
		}
		
		// Load equation expressions as ";" delimited string
		std::string equationsStr = fields->loadField("equationExpressions", "");
		if (!equationsStr.empty()) {
			_equationExpressions.clear();
			size_t pos = 0;
			size_t delimPos;
			while ((delimPos = equationsStr.find(";", pos)) != std::string::npos) {
				_equationExpressions.push_back(equationsStr.substr(pos, delimPos - pos));
				pos = delimPos + 1;
			}
			if (pos < equationsStr.length()) {
				_equationExpressions.push_back(equationsStr.substr(pos));
			}
		}
		
		_stateValues = _loadStateVector(fields, "stateValue");
		_initialStateValues = _loadStateVector(fields, "initialStateValue");
		if (_initialStateValues.empty()) {
			_initialStateValues = _stateValues;
		}
		if (_stateValues.empty()) {
			synchronizeStateFromVariables();
		}
		if (_initialStateValues.empty()) {
			_initialStateValues = _stateValues;
		}
		_stateValues.resize(_stateVariableNames.size(), 0.0);
		_initialStateValues.resize(_stateVariableNames.size(), 0.0);
		writeStateToVariables();
	}
	return res;
}

void ODESolver::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	
	fields->saveField("timeVariableName", _timeVariableName, DEFAULT.timeVariableName, saveDefaultValues);
	fields->saveField("step", _step, DEFAULT.step, saveDefaultValues);
	fields->saveField("precision", _precision, DEFAULT.precision, saveDefaultValues);
	fields->saveField("maxSteps", _maxSteps, DEFAULT.maxSteps, saveDefaultValues);
	fields->saveField("currentTime", _currentTime, DEFAULT.currentTime, saveDefaultValues);
	
	// Save state variable names as ";" delimited string
	std::string stateVarsStr = "";
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		stateVarsStr += _stateVariableNames[i];
		if (i < _stateVariableNames.size() - 1) {
			stateVarsStr += ";";
		}
	}
	fields->saveField("stateVariableNames", stateVarsStr, "", saveDefaultValues);
	
	// Save equation expressions as ";" delimited string
	std::string equationsStr = "";
	for (unsigned int i = 0; i < _equationExpressions.size(); i++) {
		equationsStr += _equationExpressions[i];
		if (i < _equationExpressions.size() - 1) {
			equationsStr += ";";
		}
	}
	fields->saveField("equationExpressions", equationsStr, "", saveDefaultValues);

	std::vector<double> stateValues = _stateValues;
	if (stateValues.empty()) {
		stateValues.reserve(_stateVariableNames.size());
		for (const std::string& variableName : _stateVariableNames) {
			stateValues.push_back(_readVariableValue(variableName));
		}
	}
	std::vector<double> initialStateValues = _initialStateValues.empty() ? stateValues : _initialStateValues;
	_saveStateVector(fields, "stateValue", stateValues, saveDefaultValues);
	_saveStateVector(fields, "initialStateValue", initialStateValues, saveDefaultValues);
}

// could be overriden

bool ODESolver::_check(std::string& errorMessage) {
	bool resultAll = true;
	
	if (_timeVariableName.empty()) {
		errorMessage += "TimeVariableName must not be empty. ";
		resultAll = false;
	}
	
	if (_stateVariableNames.empty()) {
		errorMessage += "StateVariableNames must not be empty. ";
		resultAll = false;
	}
	
	if (_equationExpressions.size() != _stateVariableNames.size()) {
		errorMessage += "EquationExpressions size must match StateVariableNames size. ";
		resultAll = false;
	}
	
	if (_step <= 0) {
		errorMessage += "Step must be greater than 0. ";
		resultAll = false;
	}
	
	return resultAll;
}

void ODESolver::_initBetweenReplications() {
	_currentTime = 0.0;
	_stateValues = _initialStateValues;
	_stateValues.resize(_stateVariableNames.size(), 0.0);
	writeStateToVariables();
}

void ODESolver::_createEditableDataDefinitions() {
	// Create time variable if it doesn't exist
	Variable* timeVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _timeVariableName));
	if (timeVar == nullptr) {
		timeVar = new Variable(_parentModel, _timeVariableName);
		_parentModel->getDataManager()->insert(timeVar);
	}
	_optionalEditableDataDefinitionInsert("TimeVariable", timeVar);

	// Create state variables if they don't exist
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		Variable* stateVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _stateVariableNames[i]));
		if (stateVar == nullptr) {
			stateVar = new Variable(_parentModel, _stateVariableNames[i]);
			_parentModel->getDataManager()->insert(stateVar);
		}
		_optionalEditableDataDefinitionInsert("StateVariable_" + _stateVariableNames[i], stateVar);
	}
	if (_initialStateValues.empty()) {
		synchronizeStateFromVariables();
		_initialStateValues = _stateValues;
	}
	writeStateToVariables();
}

void ODESolver::synchronizeStateFromVariables() {
	_stateValues.clear();
	_stateValues.reserve(_stateVariableNames.size());
	for (const std::string& variableName : _stateVariableNames) {
		_stateValues.push_back(_readVariableValue(variableName));
	}
	_stateValues.resize(_stateVariableNames.size(), 0.0);
}

void ODESolver::writeStateToVariables() const {
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		Variable* stateVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _stateVariableNames[i]));
		if (stateVar != nullptr) {
			stateVar->setValue(i < _stateValues.size() ? _stateValues[i] : 0.0);
		}
	}
	Variable* timeVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _timeVariableName));
	if (timeVar != nullptr) {
		timeVar->setValue(_currentTime);
	}
}

std::vector<double> ODESolver::_loadStateVector(PersistenceRecord* fields, const std::string& prefix) const {
	std::vector<double> values;
	const unsigned int count = fields->loadField(prefix + "s", 0u);
	values.reserve(count);
	for (unsigned int i = 0; i < count; i++) {
		values.push_back(fields->loadField(prefix + Util::StrIndex(i), 0.0));
	}

	if (!values.empty()) {
		return values;
	}

	const std::string serialized = fields->loadField(prefix + "sText", "");
	if (serialized.empty()) {
		return values;
	}

	std::stringstream stream(serialized);
	std::string token;
	while (std::getline(stream, token, ';')) {
		if (!token.empty()) {
			values.push_back(std::stod(token));
		}
	}
	return values;
}

void ODESolver::_saveStateVector(PersistenceRecord* fields,
                                 const std::string& prefix,
                                 const std::vector<double>& values,
                                 bool saveDefaultValues) const {
	fields->saveField(prefix + "s", static_cast<unsigned int>(values.size()), 0u, saveDefaultValues);
	std::string serialized;
	for (unsigned int i = 0; i < values.size(); i++) {
		fields->saveField(prefix + Util::StrIndex(i), values[i], 0.0, saveDefaultValues);
		serialized += std::to_string(values[i]);
		if (i + 1 < values.size()) {
			serialized += ";";
		}
	}
	fields->saveField(prefix + "sText", serialized, "", saveDefaultValues);
}

double ODESolver::_readVariableValue(const std::string& variableName) const {
	Variable* variable = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), variableName));
	if (variable == nullptr) {
		return 0.0;
	}
	return variable->getValue();
}
