/*
 * File:   ODESolver.cpp
 * Author: GenESyS
 *
 * Created on 13 de maio de 2026
 */

#include "plugins/data/Continuous/ODESolver.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelSimulation.h"
#include "kernel/simulator/Event.h"
#include "plugins/data/Logic/Variable.h"
#include "kernel/util/Util.h"

#include <sstream>
#include <fstream>
#include <ios>
#include <list>
#include <map>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ODESolver::GetPluginInformation;
}
#endif

ModelDataDefinition* ODESolver::NewInstance(Model* model, std::string name) {
	return new ODESolver(model, name);
}

ODESolver::ODESolver(Model* model, std::string name) : ContinuousModelDataDefinition(model, Util::TypeOf<ODESolver>(), name, 60.0, 0.01, 1u) {
	// Initialize state values vector to match state variable names size
	_stateValues.resize(_stateVariableNames.size(), 0.0);
	_initialStateValues.resize(_stateVariableNames.size(), 0.0);
}

// static

ModelDataDefinition* ODESolver::LoadInstance(Model* model, PersistenceRecord *fields) {
	ODESolver* newElement = new ODESolver(model);
	try {
		const bool loaded = newElement->_loadInstance(fields);
		if (!loaded && fields->size() != 0) {
			delete newElement;
			return nullptr;
		}
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load ODESolver instance: " + std::string(e.what()));
		delete newElement;
		return nullptr;
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
			",step=" + std::to_string(getStep()) +
			",precision=" + std::to_string(_precision) +
			",maxSteps=" + std::to_string(_maxSteps) +
			",currentTime=" + std::to_string(getCurrentTime());
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
	_confirmedStateValues.resize(stateVariableNames.size(), 0.0);
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


void ODESolver::setStateValues(std::vector<double> stateValues) {
	this->_stateValues = stateValues;
	_confirmedStateValues = std::move(stateValues);
	_stateValues = _confirmedStateValues;
	writeStateToVariables();
}

std::vector<double> ODESolver::getStateValues() const {
	return _confirmedStateValues;
}

void ODESolver::setInitialStateValues(std::vector<double> initialStateValues) {
	this->_initialStateValues = initialStateValues;
	_initialStateValues.resize(_stateVariableNames.size(), 0.0);
}

std::vector<double> ODESolver::getInitialStateValues() const {
	return _initialStateValues;
}


void ODESolver::setOutputFile(std::string outputFile) {
	this->_outputFile = outputFile;
}

std::string ODESolver::getOutputFile() const {
	return _outputFile;
}

// new methods

void ODESolver::integrate(double targetTime) {
    std::string errorMessage;
    if (!integrate(targetTime, errorMessage) && !errorMessage.empty()) {
        traceError("ODESolver integrate failed: " + errorMessage);
    }
}

bool ODESolver::integrate(double targetTime, std::string& errorMessage) {
    _stepsPerformed = 0;
    return advanceTo(targetTime, errorMessage);
}

bool ODESolver::evolveState(double currentTime, double dt, std::string& errorMessage) {
    if (_stateVariableNames.empty() || _equationExpressions.empty()) {
        errorMessage = "ODESolver equations and state variables must not be empty.";
        return false;
    }
    if (_stepsPerformed >= static_cast<unsigned int>(_maxSteps)) {
        errorMessage = "ODESolver maximum number of steps exceeded.";
        return false;
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
    std::vector<double> next(_stateValues.size());
    if (!_solver.advance(odeSystem, currentTime, dt, _stateValues.data(), next.data())) {
        errorMessage = "ODESolver numerical integration step failed.";
        return false;
    }
    _stateValues.swap(next);
    ++_stepsPerformed;
    return true;
}

void ODESolver::onContinuousStepCompleted() {
    writeStateToVariables(); // Keep ordinary model variables at the confirmed state during lookahead.
    if (!_outputFile.empty()) _appendOutputRow();
}

std::any ODESolver::internalStateView() const {
	return StateView{_stateVariableNames, _stateValues};
}

void ODESolver::confirmContinuousState() {
	_confirmedStateValues = _stateValues;
	writeStateToVariables();
}

void ODESolver::onInternalStateObserversCompleted() {
	for (std::size_t i = 0; i < _stateVariableNames.size() && i < _stateValues.size(); ++i) {
		Variable* variable = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(
			Util::TypeOf<Variable>(), _stateVariableNames[i]));
		if (variable != nullptr && i < _confirmedStateValues.size() &&
			variable->getValue() != _confirmedStateValues[i]) _stateValues[i] = variable->getValue();
	}
}

void ODESolver::resetState() {
	_currentTime = 0.0;
	_stateValues = _initialStateValues;
	_confirmedStateValues = _stateValues;
	writeStateToVariables();
}

double ODESolver::getStateValue(unsigned int index) const {
	if (index < _confirmedStateValues.size()) {
		return _confirmedStateValues[index];
	}
	return 0.0;
}

void ODESolver::setStateValue(unsigned int index, double value) {
	if (index < _stateValues.size()) {
		_stateValues[index] = value;
		_confirmedStateValues[index] = value;

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
	bool res = ContinuousModelDataDefinition::_loadInstance(fields);
	if (res) {
		_timeVariableName = fields->loadField("timeVariableName", DEFAULT.timeVariableName);
		setStepSize(fields->loadField("step", getStepSize()));
		_precision = fields->loadField("precision", DEFAULT.precision);
		_maxSteps = fields->loadField("maxSteps", DEFAULT.maxSteps);
		setStepsToSynchronize(fields->loadField("stepsToSynchronize",
			fields->loadField("autoAdvance", 1u) != 0u ? 1u : 0u));
		_outputFile = fields->loadField("outputFile", DEFAULT.outputFile);

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
		_confirmedStateValues = _stateValues;
		_initialStateValues.resize(_stateVariableNames.size(), 0.0);
		writeStateToVariables();
	}
	return res;
}

void ODESolver::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ContinuousModelDataDefinition::_saveInstance(fields, saveDefaultValues);

	fields->saveField("timeVariableName", _timeVariableName, DEFAULT.timeVariableName, saveDefaultValues);
	fields->saveField("step", getStep(), 0.01, saveDefaultValues); // legacy format
	fields->saveField("precision", _precision, DEFAULT.precision, saveDefaultValues);
	fields->saveField("maxSteps", _maxSteps, DEFAULT.maxSteps, saveDefaultValues);
	fields->saveField("outputFile", _outputFile, DEFAULT.outputFile, saveDefaultValues);

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
	bool resultAll = ContinuousModelDataDefinition::_check(errorMessage);

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


	return resultAll;
}

void ODESolver::initializeContinuousState() {
	_currentTime = 0.0;
	_stateValues = _initialStateValues;
	_stateValues.resize(_stateVariableNames.size(), 0.0);
	_confirmedStateValues = _stateValues;
	writeStateToVariables();
	// Start a fresh trajectory file (header + initial state) when export is enabled.
	if (!_outputFile.empty()) {
		_writeOutputHeader();
		_appendOutputRow();
	}
	_stepsPerformed = 0;
}

void ODESolver::_writeOutputHeader() const {
	if (_outputFile.empty()) {
		return;
	}
	std::ofstream out(_outputFile, std::ofstream::out); // truncate: fresh file per replication
	if (!out.is_open()) {
		return;
	}
	out << _timeVariableName;
	for (const std::string& name : _stateVariableNames) {
		out << "," << name;
	}
	out << std::endl;
}

void ODESolver::_appendOutputRow() const {
	if (_outputFile.empty()) {
		return;
	}
	std::ofstream out(_outputFile, std::ofstream::app);
	if (!out.is_open()) {
		return;
	}
	out.setf(std::ios::fixed);
	out.precision(9);
	out << _currentTime;
	for (unsigned int i = 0; i < _stateValues.size(); i++) {
		out << "," << _stateValues[i];
	}
	out << std::endl;
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
	// Equations may reference shared variables, attributes, formulas, or other
	// data definitions that would otherwise be classified as orphans before
	// ModelChecker validates the ODE. Keep every parser-resolved reference
	// attached to this solver so its equations remain executable after check().
	for (const std::string& expression : _equationExpressions) {
		std::map<std::string, std::list<std::string>*> references;
		_parentModel->checkReferencesToDataDefinitions(expression, &references);
		for (auto& [type, names] : references) {
			if (names == nullptr) continue;
			for (const std::string& referencedName : *names) {
				ModelDataDefinition* referenced = _parentModel->getDataManager()->getDataDefinition(type, referencedName);
				if (referenced != nullptr) {
					_optionalEditableDataDefinitionInsert(
						"EquationReference_" + type + "_" + referencedName, referenced);
				}
			}
			delete names;
			names = nullptr;
		}
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
	_confirmedStateValues = _stateValues;
}

void ODESolver::writeStateToVariables() const {
	for (unsigned int i = 0; i < _stateVariableNames.size(); i++) {
		Variable* stateVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _stateVariableNames[i]));
		if (stateVar != nullptr) {
			stateVar->setValue(i < _confirmedStateValues.size() ? _confirmedStateValues[i] : 0.0);
		}
	}
	Variable* timeVar = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), _timeVariableName));
	if (timeVar != nullptr) {
		timeVar->setValue(getCurrentTime());
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
