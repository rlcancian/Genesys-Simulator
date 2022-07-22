/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Signal.cpp
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#include "Signal.h"

#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/simulator/PluginManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Signal::GetPluginInformation;
}
#endif

// constructor

ModelDataDefinition* Signal::NewInstance(Model* model, std::string name) {
	return new Signal(model, name);
}

Signal::Signal(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Signal>(), name) {
}

// public virtual

std::string Signal::show() {
	return ModelComponent::show() + "";
}

//public

void Signal::setSignalData(SignalData* signal) {
	_signalData = signal;
}

// public static

ModelComponent* Signal::LoadInstance(Model* model, PersistenceRecord *fields) {
	Signal* newComponent = new Signal(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

PluginInformation* Signal::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Signal>(), &Signal::LoadInstance, &Signal::NewInstance);
	info->setCategory("Decisions");
	// ...
	return info;
}

// protected must override

void Signal::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	unsigned int limit = _parentModel->parseExpression(_limitExpression);
	_parentModel->getTracer()->trace("Triggering signal \""+_signalData->getName()+"\" with limit \""+_limitExpression+"\"="+std::to_string(limit));
	unsigned int freed = _signalData->generateSignal(_signalData->getId(),limit);
	this->_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Signal::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		// @TODO: not implemented yet
		this->_limitExpression = fields->loadField("limitExpression", DEFAULT.limitExpression);
	}
	return res;
}

void Signal::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// @TODO: not implemented yet
	fields->saveField("limitExpression", _limitExpression, DEFAULT.limitExpression);
}

// protected should override

bool Signal::_check(std::string* errorMessage) {
	bool resultAll = true;
	// @TODO: not implemented yet
	*errorMessage += "";
	return resultAll;
}

void Signal::_createInternalAndAttachedData() {
	// internal
	PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
	//attached
	if (_signalData == nullptr) {
		if (_parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
			_signalData = pm->newInstance<SignalData>(_parentModel);
		}
	}
	_attachedDataInsert("SignalData", _signalData);
}

const std::string&Signal::limitExpression() const
{
	return _limitExpression;
}

void Signal::setLimitExpression(const std::string&newLimitExpression)
{
	_limitExpression = newLimitExpression;
}

void Signal::_initBetweenReplications() {

}
