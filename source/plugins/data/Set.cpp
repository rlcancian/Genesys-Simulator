/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Set.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:11
 */

#include "Set.h"
#include "../../kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Set::GetPluginInformation;
}
#endif

ModelDataDefinition* Set::NewInstance(Model* model, std::string name) {
	return new Set(model, name);
}

Set::Set(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Set>(), name) {
}

std::string Set::show() {
	return ModelDataDefinition::show() +
			"";
}

void Set::setSetOfType(std::string _setOfType) {
	this->_setOfType = _setOfType;
}

std::string Set::getSetOfType() const {
	return _setOfType;
}

List<ModelDataDefinition*>* Set::getElementSet() const {
	return _elementSet;
}

PluginInformation* Set::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Set>(), &Set::LoadInstance, &Set::NewInstance);
	return info;
}

ModelDataDefinition* Set::LoadInstance(Model* model, PersistenceRecord *fields) {
	Set* newElement = new Set(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Set::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			_setOfType = fields->loadField("SetOfType", DEFAULT.setOfType);
			unsigned int memberSize = fields->loadField("ElementSet", DEFAULT.membersSize);
			for (unsigned int i = 0; i < memberSize; i++) {
				std::string memberName = fields->loadField("member" + Util::StrIndex(i));
				ModelDataDefinition* member = _parentModel->getDataManager()->getDataDefinition(_setOfType, memberName);
				if (member == nullptr) { // not found. That's a problem. Resource not loaded yet (or mismatch name
					_parentModel->getTracer()->traceError("ERROR: Could not found " + _setOfType + " set member named \"" + memberName + "\"", TraceManager::Level::L1_errorFatal);
				} else {//found
					_elementSet->insert(member);
				}
			}
		} catch (...) {
		}
	}
	return res;
}

void Set::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("SetOfType", _setOfType, DEFAULT.setOfType, saveDefaultValues);
	fields->saveField("ElementSet", _elementSet->size(), DEFAULT.membersSize, saveDefaultValues);
	unsigned int i = 0;
	for (ModelDataDefinition* modeldatum : *_elementSet->list()) {
		fields->saveField("member" + Util::StrIndex(i), modeldatum->getName());
		i++;
	}
}

bool Set::_check(std::string* errorMessage) {
	bool resultAll = true;
	if (_elementSet->size() > 0) {
		std::string typeOfFirstElement = _elementSet->front()->getClassname();
		if (_setOfType == "") {
			_setOfType = typeOfFirstElement;
		} else if (_setOfType != typeOfFirstElement) {
			resultAll = false;
			*errorMessage += "Set is of type \"" + _setOfType + "\" and first modeldatum is of type \"" + typeOfFirstElement + "\"";
		}
	}
	int i = 0;
	for (ModelDataDefinition* data : *_elementSet->list()) {
		this->_attachedDataInsert("Member" + Util::StrIndex(i), data);
	}
	*errorMessage += "";
	return resultAll;
}

ParserChangesInformation* Set::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void Set::_createInternalAndAttachedData() {
	for(ModelDataDefinition* dd: *_elementSet->list()) {
		_attachedDataInsert(getName()+"."+dd->getName(), dd);
	}	
}

