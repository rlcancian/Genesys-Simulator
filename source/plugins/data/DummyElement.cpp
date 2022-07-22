/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   DummyElement.cpp
 * Author: rlcancian
 * 
 * Created on 11 de janeiro de 2022, 22:26
 */

#include "DummyElement.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DummyElement::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition* DummyElement::NewInstance(Model* model, std::string name) {
	return new DummyElement(model, name);
}

DummyElement::DummyElement(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<DummyElement>(), name) {
}

//public

std::string DummyElement::show() {
	return ModelDataDefinition::show();
}

// public static 

ModelDataDefinition* DummyElement::LoadInstance(Model* model, PersistenceRecord *fields) {
	DummyElement* newElement = new DummyElement(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

PluginInformation* DummyElement::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DummyElement>(), &DummyElement::LoadInstance, &DummyElement::NewInstance);
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

bool DummyElement::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			this->_someString = fields->loadField("someString", DEFAULT.someString);
			this->_someUint = fields->loadField("someUint", DEFAULT.someUint);
		} catch (...) {
		}
	}
	return res;
}

void DummyElement::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("someUint", _someUint, DEFAULT.someUint);
	fields->saveField("someString", _someString, DEFAULT.someString);
}

// protected virtual -- could be overriden 

//ParserChangesInformation* DummyElement::_getParserChangesInformation() {}

bool DummyElement::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}

void DummyElement::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}

void DummyElement::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		//if (_internal == nullptr) {
		//	_internal = new StatisticsCollector(_parentModel, getName() + "." + "NumberInQueue", this); 
		//	_internelElementsInsert("NumberInQueue", _internal);
		//}
	} else { //if (_cstatNumberInQueue != nullptr) {
		this->_internalDataClear();
	}
}

// private
