/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SeizableItemRequest.cpp
 * Author: rlcancian
 * 
 * Created on 10 de abril de 2021, 08:45
 */

#include "SeizableItemRequest.h"
#include "ModelElement.h"
#include "Model.h"

SeizableItemRequest::SeizableItemRequest(ModelElement* resourceOrSet, std::string quantityExpression, SeizableItemRequest::SeizableType resourceType, SeizableItemRequest::SelectionRule selectionRule, std::string saveAttribute, std::string index) {
	_seizableType = resourceType;
	_resourceOrSet = resourceOrSet;
	_quantityExpression = quantityExpression;
	_selectionRule = selectionRule;
	_saveAttribute = saveAttribute;
	_index = index;
}

bool SeizableItemRequest::loadInstance(std::map<std::string, std::string>* fields) {
	bool res = true;
	try {
		_seizableType = static_cast<SeizableItemRequest::SeizableType> (LoadField(fields, "seizableType", static_cast<int> (DEFAULT.seizableType)));
		_seizableName = LoadField(fields, "seizable", "");
		_quantityExpression = LoadField(fields, "quantityExpression", DEFAULT.quantityExpression);
		_selectionRule = static_cast<SeizableItemRequest::SelectionRule> (LoadField(fields, "selectionRule", static_cast<int> (DEFAULT.selectionRule)));
		_saveAttribute = LoadField(fields, "saveAttribute", DEFAULT.saveAttribute);
		_index = LoadField(fields, "index", DEFAULT.index);
		if (_componentManager != nullptr) {
			this->_resourceOrSet = _componentManager->find(_seizableName);
		}
	} catch (...) {
		res = false;
	}
	return res;
}

bool SeizableItemRequest::loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex) {
	bool res = true;
	std::string num = std::to_string(parentIndex);
	try {
		_seizableType = static_cast<SeizableItemRequest::SeizableType> (LoadField(fields, "seizableType" + num, static_cast<int> (DEFAULT.seizableType)));
		_seizableName = LoadField(fields, "seizable" + num, "");
		_quantityExpression = LoadField(fields, "quantityExpression" + num, DEFAULT.quantityExpression);
		_selectionRule = static_cast<SeizableItemRequest::SelectionRule> (LoadField(fields, "selectionRule" + num, static_cast<int> (DEFAULT.selectionRule)));
		_saveAttribute = LoadField(fields, "saveAttribute" + num, DEFAULT.saveAttribute);
		_index = LoadField(fields, "index" + num, DEFAULT.index);
		if (_componentManager != nullptr) {
			this->_resourceOrSet = _componentManager->find(_seizableName);
		}
	} catch (...) {
		res = false;
	}
	return res;
}

std::map<std::string, std::string>* SeizableItemRequest::saveInstance(unsigned int parentIndex) {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	std::string num = std::to_string(parentIndex);
	SaveField(fields, "seizableType" + num, static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType));
	//SaveField(fields, "resourceId" + num, _resourceOrSet->getId());
	SaveField(fields, "seizable" + num, _resourceOrSet->getName(), "");
	SaveField(fields, "quantityExpression" + num, _quantityExpression, DEFAULT.quantityExpression);
	SaveField(fields, "selectionRule" + num, static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule));
	SaveField(fields, "saveAttribute" + num, _saveAttribute, DEFAULT.saveAttribute);
	SaveField(fields, "index" + num, _index, DEFAULT.index);
	return fields;
}

std::map<std::string, std::string>* SeizableItemRequest::saveInstance() {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	SaveField(fields, "seizableType", static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType));
	SaveField(fields, "resourceId", _resourceOrSet->getId());
	SaveField(fields, "seizable", _resourceOrSet->getName(), "");
	SaveField(fields, "quantityExpression", _quantityExpression, DEFAULT.quantityExpression);
	SaveField(fields, "selectionRule", static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule));
	SaveField(fields, "saveAttribute", _saveAttribute, DEFAULT.saveAttribute);
	SaveField(fields, "index", _index, DEFAULT.index);
	return fields;
}

std::string SeizableItemRequest::show() {
	return "resourceType=" + std::to_string(static_cast<int> (_seizableType)) + ",resource=\"" + _resourceOrSet->getName() + "\",quantityExpression=\"" + _quantityExpression + "\", selectionRule=" + std::to_string(static_cast<int> (_selectionRule)) + ", _saveAttribute=\"" + _saveAttribute + "\",index=\"" + _index + "\"";
}

void SeizableItemRequest::setIndex(std::string index) {
	this->_index = _index;
}

std::string SeizableItemRequest::getIndex() const {
	return _index;
}

void SeizableItemRequest::setSaveAttribute(std::string saveAttribute) {
	this->_saveAttribute = _saveAttribute;
}

std::string SeizableItemRequest::getSaveAttribute() const {
	return _saveAttribute;
}

void SeizableItemRequest::setSelectionRule(SeizableItemRequest::SelectionRule selectionRule) {
	this->_selectionRule = _selectionRule;
}

SeizableItemRequest::SelectionRule SeizableItemRequest::getSelectionRule() const {
	return _selectionRule;
}

void SeizableItemRequest::setQuantityExpression(std::string quantityExpression) {
	this->_quantityExpression = _quantityExpression;
}

std::string SeizableItemRequest::getQuantityExpression() const {
	return _quantityExpression;
}

std::string SeizableItemRequest::getResourceName() const {
	return _seizableName;
}

void SeizableItemRequest::setResource(Resource* resource) {
	this->_resourceOrSet = resource;
	_seizableName = resource->getName();
}

Resource* SeizableItemRequest::getResource() const {
	return static_cast<Resource*> (_resourceOrSet);
}

void SeizableItemRequest::setSet(Set* set) {
	this->_resourceOrSet = set;
	_seizableName = set->getName();
}

Set* SeizableItemRequest::getSet() const {
	return static_cast<Set*> (_resourceOrSet);
}

void SeizableItemRequest::setSeizableType(SeizableItemRequest::SeizableType resourceType) {
	this->_seizableType = _seizableType;
}

SeizableItemRequest::SeizableType SeizableItemRequest::getSeizableType() const {
	return _seizableType;
}

void SeizableItemRequest::setLastMemberSeized(unsigned int lastMemberSeized) {
	this->_lastMemberSeized = lastMemberSeized;
}

unsigned int SeizableItemRequest::getLastMemberSeized() const {
	return _lastMemberSeized;
}

ModelElement* SeizableItemRequest::getSeizable() const {
	return _resourceOrSet;
}

void SeizableItemRequest::setComponentManager(ComponentManager* _componentManager) {
	this->_componentManager = _componentManager;
}

