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
	// SEG FAULT??????
	for (PairSignalDataEventHandler* sreh : *_signalDataEventHandlers->list()) {
		compHandler = sreh->second;
		if (compHandler == component) {
			return; // already exists. Do not insert again
		}
	}
	PairSignalDataEventHandler* pairEventHandler = new PairSignalDataEventHandler(eventHandler, component);
	_signalDataEventHandlers->insert(pairEventHandler);
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

bool SignalData::_check(std::string* errorMessage) {
	bool resultAll = true;
	///@TODO
	//resultAll &= _quantityExpression != "";
	return resultAll;
}

void SignalData::_initBetweenReplications() {
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
