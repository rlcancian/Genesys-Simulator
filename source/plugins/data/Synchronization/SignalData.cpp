/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   SignalData.cpp
 * Author: rlcancian
 *
 * Created on 25 de maio de 2022, 14:37
 */

#include "SignalData.h"
#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &SignalData::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition* SignalData::NewInstance(Model* model, std::string name) {
	return new SignalData(model, name);
}

SignalData::SignalData(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<SignalData>(), name) {
}

SignalData::~SignalData() {
	for (PairSignalDataEventHandler* handler : *_signalDataEventHandlers->list()) {
		delete handler;
	}
	_signalDataEventHandlers->clear();
	delete _signalDataEventHandlers;
	_signalDataEventHandlers = nullptr;
}

//public

std::string SignalData::show() {
	return ModelDataDefinition::show();
}

unsigned int SignalData::generateSignal(double signalValue, unsigned int limit) {
	_remainsToLimit = limit;
	unsigned int freed = _notifySignalDataEventHandlers();
	return freed;
}

void SignalData::addSignalDataEventHandler(SignalDataEventHandler eventHandler, ModelComponent* component) {
	ModelComponent* compHandler;
	for (PairSignalDataEventHandler* sreh : *_signalDataEventHandlers->list()) {
		compHandler = sreh->second;
		if (compHandler == component) {
			return; // already exists. Do not insert again
		}
	}
	PairSignalDataEventHandler* pairEventHandler = new PairSignalDataEventHandler(eventHandler, component);
	_signalDataEventHandlers->insert(pairEventHandler);
}

void SignalData::removeSignalDataEventHandler(ModelComponent* component) {
	if (_signalDataEventHandlers == nullptr) {
		return;
	}
	std::list<PairSignalDataEventHandler*>* handlers = _signalDataEventHandlers->list();
	for (std::list<PairSignalDataEventHandler*>::iterator it = handlers->begin(); it != handlers->end(); ++it) {
		PairSignalDataEventHandler* pairEventHandler = *it;
		if (pairEventHandler->second == component) {
			delete pairEventHandler;
			handlers->erase(it);
			return;
		}
	}
}

bool SignalData::hasSignalDataEventHandler(ModelComponent* component) const {
	for (PairSignalDataEventHandler* pairEventHandler : *_signalDataEventHandlers->list()) {
		if (pairEventHandler->second == component) {
			return true;
		}
	}
	return false;
}

// public static

ModelDataDefinition* SignalData::LoadInstance(Model* model, PersistenceRecord *fields) {
	SignalData* newElement = new SignalData(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

PluginInformation* SignalData::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<SignalData>(), &SignalData::LoadInstance, &SignalData::NewInstance);
	info->setDescriptionHelp("//@TODO");
	//info->setDescriptionHelp("");
	//info->setObservation("");
	//info->setMinimumOutputs();
	//info->setDynamicLibFilenameDependencies();
	//info->setFields();
	// ...
	return info;
}

// protected virtual -- must be overriden

bool SignalData::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			//this->_someUint = fields->loadField("someUint", DEFAULT.someUint);
		} catch (...) {
		}
	}
	return res;
}

void SignalData::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
}

// protected virtual -- could be overriden

bool SignalData::_check(std::string& errorMessage) {
	if (_signalDataEventHandlers->size() == 0) {
		errorMessage = "SignalData \"" + this->getName() + "\" requires at least one event handler";
		traceError(errorMessage);
		return false;
	}
	return true;
}

void SignalData::_initBetweenReplications() {
	_remainsToLimit = 0;
}

//void SignalData::_createInternalAndAttachedData() {}

// private

unsigned int SignalData::_notifySignalDataEventHandlers() {
	unsigned int freed, sumFreed = 0;
	for (PairSignalDataEventHandler* sortedHandler : *_signalDataEventHandlers->list()) {
		if (_remainsToLimit>0) {
			SignalDataEventHandler handler = sortedHandler->first;
			freed = handler(this);
			sumFreed += freed;
			//_remainsToLimit -= freed;
		}
	}
	return sumFreed;
}

unsigned int SignalData::remainsToLimit() const {
	return _remainsToLimit;
}

void SignalData::decreaseRemainLimit() {
	_remainsToLimit--;
}

void SignalData::_createReportStatisticsDataDefinitions() {
}

void SignalData::_createEditableDataDefinitions() {
}

void SignalData::_createOthersDataDefinitions() {
}
