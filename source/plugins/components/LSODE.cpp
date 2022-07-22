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
#include "LSODE.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &LSODE::GetPluginInformation;
}
#endif

ModelDataDefinition* LSODE::NewInstance(Model* model, std::string name) {
	return new LSODE(model, name);
}

LSODE::LSODE(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<LSODE>(), name) {
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

void LSODE::setFilename(std::string filename) {
	this->_filename = filename;
}

std::string LSODE::getFilename() const {
	return _filename;
}

bool LSODE::_doStep() {
	double initTime, time, tnow, eqResult, halfStep;
	//std::list<std::string>* eqs = _diffEquations->formulaExpressions()->list();
	unsigned int i, numEqs = _diffEquations->size();
	double k1[numEqs], k2[numEqs], k3[numEqs], k4[numEqs], valVar[numEqs];
	time = _timeVariable->getValue();
	initTime = time;
	std::string expression;
	tnow = _parentModel->getSimulation()->getSimulatedTime();
	// @TODO: numerical error treatment by just adding 1e-15
	bool res = time + _step <= tnow + 1e-15;
	if (res) {
		halfStep = _step * 0.5;
		for (i = 0; i < numEqs; i++) {//(std::list<std::string>::iterator it = eqs->begin(); it != eqs->end(); it++) {
			expression = _diffEquations->getAtRank(i);
			valVar[i] = _variable->getValue(std::to_string(i));
			eqResult = _parentModel->parseExpression(expression);
			k1[i] = eqResult;
		}
		time += halfStep;
		_timeVariable->setValue(time);
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(std::to_string(i), valVar[i] + k1[i] * halfStep);
		}
		for (i = 0; i < numEqs; i++) {
			expression = _diffEquations->getAtRank(i);
			eqResult = _parentModel->parseExpression(expression);
			k2[i] = eqResult;
		}
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(std::to_string(i), valVar[i] + k2[i] * halfStep);
		}
		for (i = 0; i < numEqs; i++) {
			expression = _diffEquations->getAtRank(i);
			eqResult = _parentModel->parseExpression(expression);
			k3[i] = eqResult;
		}
		for (i = 0; i < numEqs; i++) {
			_variable->setValue(std::to_string(i), valVar[i] + k3[i] * halfStep);
		}
		for (i = 0; i < numEqs; i++) {
			expression = _diffEquations->getAtRank(i);
			eqResult = _parentModel->parseExpression(expression);
			k4[i] = eqResult;
		}
		for (i = 0; i < numEqs; i++) {

			eqResult = _variable->getValue(std::to_string(i)) +(_step / 6) * (k1[i] + 2 * (k2[i] + k3[i]) + k4[i]);
			_variable->setValue(std::to_string(i), eqResult);
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
		_parentModel->getTracer()->traceSimulation(this, message, TraceManager::Level::L8_detailed);
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
	_parentModel->sendEntityToComponent(entity, getConnections()->getFrontConnection());
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

bool LSODE::_check(std::string* errorMessage) {
	bool resultAll = true;
	std::ofstream savefile;
	// @TODO: not implemented yet
	*errorMessage += "";
	if (resultAll) {
		if (_filename != "") {
			try {
				savefile.open(_filename, std::ofstream::out);
				std::string message = _timeVariable->getName();
				for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
					message += "\t" + _variable->getName() + "[" + std::to_string(i) + "]";
				}
				savefile << message << std::endl;
				// TODO: It should save the initial values only AFTER variables are initialized. For now, initial values may be wrong
				message = std::to_string(_timeVariable->getValue());
				for (unsigned int i = 0; i < _variable->getDimensionSizes()->front(); i++) {
					message += "\t" + std::to_string(_variable->getValue(std::to_string(i)));
				}
				savefile << message << std::endl;

				savefile.close();
			} catch (const std::exception& e) {
				resultAll = false;
				*errorMessage += "Error creating file";
			}
		}

	}

	return resultAll;
}

PluginInformation* LSODE::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<LSODE>(), &LSODE::LoadInstance, &LSODE::NewInstance);
	info->setCategory("Physical");
	// ...
	return info;
}
