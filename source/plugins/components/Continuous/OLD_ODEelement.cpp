/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ODE.cpp
 * Author: rlcancian
 * 
 * Created on 13 de Junho de 2019, 19:12
 */

#include "plugins/components/Continuous/OLD_ODEelement.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &OLD_ODEelement::GetPluginInformation;
}
#endif

ModelDataDefinition* OLD_ODEelement::NewInstance(Model* model, std::string name) {
	return new OLD_ODEelement(model, name);
}

OLD_ODEelement::OLD_ODEelement(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<OLD_ODEelement>(), name) {
	//_elems = elems;
	SimulationControlGeneric<double>* propStepH = new SimulationControlGeneric<double>(
									std::bind(&OLD_ODEelement::getStepH, this), std::bind(&OLD_ODEelement::setStepH, this, std::placeholders::_1),
									Util::TypeOf<OLD_ODEelement>(), getName(), "StepH", "");
	SimulationControlGeneric<double>* propEndTime = new SimulationControlGeneric<double>(
									std::bind(&OLD_ODEelement::getEndTime, this), std::bind(&OLD_ODEelement::setEndTime, this, std::placeholders::_1),
									Util::TypeOf<OLD_ODEelement>(), getName(), "EndTime", "");
    // SimulationControlGenericListPointer<ODEfunction*, Model*, ODEfunction>* propODEfunctions = new SimulationControlGenericListPointer<ODEfunction*, Model*, ODEfunction> (
                                    // _parentModel,
                                    // std::bind(&OLD_ODEelement::getODEfunctions, this), std::bind(&OLD_ODEelement::addODEfunction, this, std::placeholders::_1), std::bind(&OLD_ODEelement::removeODEfunction, this, std::placeholders::_1),
                                    // Util::TypeOf<OLD_ODEelement>(), getName(), "ODEfunctions", "");

	_parentModel->getControls()->insert(propStepH);
	_parentModel->getControls()->insert(propEndTime);
    // _parentModel->getControls()->insert(propODEfunctions);

	// setting properties
	_addSimulationControl(propStepH);
	_addSimulationControl(propEndTime);
    // _addSimulationControl(propODEfunctions);
}

std::string OLD_ODEelement::show() {
	std::string txt = ModelDataDefinition::show();
	unsigned short i = 0;
	for (std::list<ODEfunction*>::iterator it = _ODEfunctions->list()->begin(); it != _ODEfunctions->list()->end(); it++) {
		txt += ",func[" + std::to_string(i) + "]=\"" + (*it)->expression + "\",(func[" + std::to_string(i) + "][" + std::to_string((*it)->initialPoint) + "]=" + std::to_string((*it)->initialValue) + ")";
	}
	return txt;
}

double OLD_ODEelement::solve() {
	////
	return 0.0; // dummy
}

void OLD_ODEelement::setStepH(double _h) {
	this->_stepH = _h;
}

double OLD_ODEelement::getStepH() const {
	return _stepH;
}

void OLD_ODEelement::setEndTime(double _endTime) {
	this->_endTime = _endTime;
}

double OLD_ODEelement::getEndTime() const {
	return _endTime;
}

List<ODEfunction*>* OLD_ODEelement::getODEfunctions() const {
	return _ODEfunctions;
}

void OLD_ODEelement::addODEfunction(ODEfunction* newFunction) {
	_ODEfunctions->insert(newFunction);
}

void OLD_ODEelement::removeODEfunction(ODEfunction* function) {
	_ODEfunctions->remove(function);
}

PluginInformation* OLD_ODEelement::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<OLD_ODEelement>(), &OLD_ODEelement::LoadInstance, &OLD_ODEelement::NewInstance);
	return info;
}

ModelDataDefinition* OLD_ODEelement::LoadInstance(Model* model, PersistenceRecord *fields) {
	OLD_ODEelement* newElement = new OLD_ODEelement(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool OLD_ODEelement::_loadInstance(PersistenceRecord *fields) {
	return ModelDataDefinition::_loadInstance(fields);
}

void OLD_ODEelement::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	return ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
}

bool OLD_ODEelement::_check(std::string& errorMessage) {
	bool result = true;
	unsigned short i = 0;
	ODEfunction* func;
	for (std::list<ODEfunction*>::iterator it = _ODEfunctions->list()->begin(); it != _ODEfunctions->list()->end(); it++) {
		func = (*it);
		result &= _parentModel->checkExpression(func->expression, "expression[" + std::to_string(i++) + "]", errorMessage);
	}
	return result;
}

void OLD_ODEelement::_createReportStatisticsDataDefinitions() {
}

void OLD_ODEelement::_createEditableDataDefinitions() {
}

void OLD_ODEelement::_createOthersDataDefinitions() {
}
