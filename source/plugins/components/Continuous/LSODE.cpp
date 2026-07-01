/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   LSODE.cpp
 * Author: rlcancian
 *
 * Created on 22 de Outubro de 2019, 22:28
 */

#include <fstream>
#include <vector>
#include "plugins/components/Continuous/LSODE.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &LSODE::GetPluginInformation;
}
#endif

ModelDataDefinition* LSODE::NewInstance(Model* model, std::string name) {
	return new LSODE(model, name);
}

LSODE::LSODE(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<LSODE>(), name) {
	SimulationControlGenericClass<Variable*, Model*, Variable>* propTimeVariable = new SimulationControlGenericClass<Variable*, Model*, Variable>(
									_parentModel,
									std::bind(&LSODE::getTimeVariable, this), std::bind(&LSODE::setTimeVariable, this, std::placeholders::_1),
									Util::TypeOf<LSODE>(), getName(), "TimeVariable", "");
	SimulationControlGeneric<double>* propStep = new SimulationControlGeneric<double>(
									std::bind(&LSODE::getStep, this), std::bind(&LSODE::setStep, this, std::placeholders::_1),
									Util::TypeOf<LSODE>(), getName(), "Step", "");
	SimulationControlGenericClass<Variable*, Model*, Variable>* propVariable = new SimulationControlGenericClass<Variable*, Model*, Variable>(
									_parentModel,
									std::bind(&LSODE::getVariable, this), std::bind(&LSODE::setVariable, this, std::placeholders::_1),
									Util::TypeOf<LSODE>(), getName(), "Variable", "");
	SimulationControlGeneric<std::string>* propFileName = new SimulationControlGeneric<std::string>(
									std::bind(&LSODE::getFileName, this), std::bind(&LSODE::setFilename, this, std::placeholders::_1),
									Util::TypeOf<LSODE>(), getName(), "FileName", "");
	SimulationControlGenericList<std::string, Model*, std::string>* propDiffEquations = new SimulationControlGenericList<std::string, Model*, std::string> (
									_parentModel,
                                    std::bind(&LSODE::getDiffEquations, this), std::bind(&LSODE::addDiffEquation, this, std::placeholders::_1), std::bind(&LSODE::removeDiffEquation, this, std::placeholders::_1),
									Util::TypeOf<LSODE>(), getName(), "DiffEquations", "");								

	_parentModel->getControls()->insert(propTimeVariable);
	_parentModel->getControls()->insert(propStep);
	_parentModel->getControls()->insert(propVariable);
	_parentModel->getControls()->insert(propFileName);
	_parentModel->getControls()->insert(propDiffEquations);

	// setting properties
	_addSimulationControl(propTimeVariable);
	_addSimulationControl(propStep);
	_addSimulationControl(propVariable);
	_addSimulationControl(propFileName);
	_addSimulationControl(propDiffEquations);
}

std::string LSODE::show() {
	return ModelComponent::show() + "";
}

ModelComponent* LSODE::LoadInstance(Model* model, PersistenceRecord *fields) {
	LSODE* newComponent = new LSODE(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void LSODE::setTimeVariable(Variable* timeVariable) {
	_timeVariable = timeVariable;
}

Variable* LSODE::getTimeVariable() const {
	return _timeVariable;
}

void LSODE::setStep(double step) {
	_step = step;
}

double LSODE::getStep() const {
	return _step;
}

void LSODE::setVariable(Variable* variables) {
	_variable = variables;
}

Variable* LSODE::getVariable() const {
	return _variable;
}

List<std::string>* LSODE::getDiffEquations() const {
	return _diffEquations;
}

void LSODE::addDiffEquation(std::string newDiffEquation) {
	_diffEquations->insert(newDiffEquation);
}

void LSODE::removeDiffEquation(std::string diffEquation) {
	_diffEquations->remove(diffEquation);
}

void LSODE::setFilename(std::string filename) {
	this->_filename = filename;
}

std::string LSODE::getFileName() const {
	return _filename;
}

bool LSODE::_doStep() {
	double initTime, time, tnow, eqResult, halfStep;
	std::vector<std::string> expressions;
	for (const std::string& equation : *_diffEquations->list()) {
		expressions.push_back(equation);
	}
	unsigned int i, numEqs = expressions.size();
	// valVar holds the state at the start of the step; it is the base for all four stages
	std::vector<double> k1(numEqs), k2(numEqs), k3(numEqs), k4(numEqs), valVar(numEqs);
	time = _timeVariable->getValue();
	initTime = time; // save t0 — needed to restore the time variable after midpoint evaluations
	std::string expression;
	tnow = _parentModel->getSimulation()->getSimulatedTime();
	// 1e-15 guard against floating-point rounding when time + step ≈ tnow
	bool res = time + _step <= tnow + 1e-15;
	if (res) { // if simulatedTime has not reached a single step, do not solve
		halfStep = _step * 0.5;
		// k1: slope at t0, y0
		for (i = 0; i < numEqs; i++) {
			expression = expressions[i];
			valVar[i] = _variable->getValue(std::to_string(i));
			eqResult = _parentModel->parseExpression(expression);
			k1[i] = eqResult;
		}
		// advance to midpoint for k2 evaluation
		time += halfStep;
		_timeVariable->setValue(time);
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(valVar[i] + k1[i] * halfStep, std::to_string(i));
		}
		// k2: slope at t0+h/2, y0+h/2·k1
		for (i = 0; i < numEqs; i++) {
			expression = expressions[i];
			eqResult = _parentModel->parseExpression(expression);
			k2[i] = eqResult;
		}
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(valVar[i] + k2[i] * halfStep, std::to_string(i));
		}
		// k3: slope at t0+h/2, y0+h/2·k2 (time stays at midpoint)
		for (i = 0; i < numEqs; i++) {
			expression = expressions[i];
			eqResult = _parentModel->parseExpression(expression);
			k3[i] = eqResult;
		}
		// advance to end of step for k4 evaluation
		time = initTime + _step;
		_timeVariable->setValue(time);
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(valVar[i] + k3[i] * _step, std::to_string(i));
		}
		// k4: slope at t0+h, y0+h·k3
		for (i = 0; i < numEqs; i++) {
			expression = expressions[i];
			eqResult = _parentModel->parseExpression(expression);
			k4[i] = eqResult;
		}
		// combine: use valVar (not the current variable value) as base to avoid accumulated error
		for (i = 0; i < numEqs; i++) {
			eqResult = valVar[i] + (_step / 6) * (k1[i] + 2 * (k2[i] + k3[i]) + k4[i]);
			_variable->setValue(eqResult, std::to_string(i));
		}
		time = initTime + _step;
		_timeVariable->setValue(time);
	}
	return res;
}

void LSODE::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	// open file
	std::ofstream savefile;
	if (_filename != "") {
		savefile.open(_filename, std::ofstream::app);
	}
	while (_doStep()) {// execute solve ODE step by step until reach TNOW
		std::string message = "time=" + std::to_string(_timeVariable->getValue());
		for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
			message += " ," + _variable->getName() + "[" + std::to_string(i) + "]=" + std::to_string(_variable->getValue(std::to_string(i)));
		}
		traceSimulation(this, message, TraceManager::Level::L8_detailed);
		if (_filename != "") {
			message = std::to_string(_timeVariable->getValue());
			for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
				message += "\t" + std::to_string(_variable->getValue(std::to_string(i)));
			}
			savefile << message << std::endl;
		}
	}
	if (_filename != "") {
		savefile.close();
	}
	_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
}

bool LSODE::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
	}

	return res;
}

void LSODE::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
}

bool LSODE::_check(std::string& errorMessage) {
	bool resultAll = true;
	std::ofstream savefile;
	// @TODO: not implemented yet
	errorMessage += "";
	if (resultAll) {
		if (_filename != "") {
			try {
				savefile.open(_filename, std::ofstream::out);
				std::string message = _timeVariable->getName();
				for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
					message += "\t" + _variable->getName() + "[" + std::to_string(i) + "]";
				}
				savefile << message << std::endl;
				//@TODO: It should save the initial values only AFTER variables are initialized. For now, initial values may be wrong
				message = std::to_string(_timeVariable->getValue());
				for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
					message += "\t" + std::to_string(_variable->getValue(std::to_string(i)));
				}
				savefile << message << std::endl;

				savefile.close();
			} catch (const std::exception& e) {
				resultAll = false;
				errorMessage += "Error creating file";
			}
		}

	}

	return resultAll;
}

PluginInformation* LSODE::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<LSODE>(), &LSODE::LoadInstance, &LSODE::NewInstance);
	info->setCategory("Continuous");
	// ...
	return info;
}

// void LSODE::_createInternalStatisticReporters() { }

// void LSODE::_createEditableDataDefinitions() { }

// void LSODE::_createAttachedAttributes() { }
