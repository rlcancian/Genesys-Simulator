/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SeizableItem.cpp
 * Author: rlcancian
 *
 * Created on 10 de abril de 2021, 08:45
 */

#include "SeizableItem.h"
#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include <cassert>

SeizableItem::SeizableItem(ModelDataDefinition* resourceOrSet, std::string quantityExpression, SeizableItem::SelectionRule selectionRule, std::string saveAttribute, std::string index) {
	SeizableItem::SeizableType resourceType;
	if (dynamic_cast<Resource*> (resourceOrSet) != nullptr) {
		resourceType = SeizableItem::SeizableType::RESOURCE;
	} else if (dynamic_cast<Set*> (resourceOrSet) != nullptr) {
		resourceType = SeizableItem::SeizableType::SET;
	} else {
		//assert(false);//@TODO
	}
	_seizableType = resourceType;
	_resourceOrSet = resourceOrSet;
	_quantityExpression = quantityExpression;
	_selectionRule = selectionRule;
	_saveAttribute = saveAttribute;
	_index = index;
}

SeizableItem::SeizableItem(Model* model, std::string resourceName, std::string quantityExpression, SeizableItem::SelectionRule selectionRule, std::string saveAttribute, std::string index) {
	ModelDataDefinition* resource = model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), resourceName);
	if (resource == nullptr && model->isAutomaticallyCreatesModelDataDefinitions()) {
		resource = model->getParentSimulator()->getPlugins()->newInstance<Resource>(model, resourceName);
	}
	SeizableItem(resource, quantityExpression, selectionRule, saveAttribute, index);
}

SeizableItem::SeizableItem(SeizableItem* original) {
	_seizableType = original->getSeizableType();
	if (original->getSeizableType() == SeizableItem::SeizableType::RESOURCE)
		_resourceOrSet = original->getResource();
	else
		_resourceOrSet = original->getSet();
	_quantityExpression = original->getQuantityExpression();
	_selectionRule=original->getSelectionRule();
	_saveAttribute=original->getSaveAttribute();
	_index = original->getIndex();
}

bool SeizableItem::loadInstance(PersistenceRecord *fields) {
	bool res = true;
	try {
		_seizableType = static_cast<SeizableItem::SeizableType> (fields->loadField("SeizableType", static_cast<int> (DEFAULT.seizableType)));
		_seizableName = fields->loadField("ResourceName", "");
		_quantityExpression = fields->loadField("QuantityExpression", DEFAULT.quantityExpression);
		_selectionRule = static_cast<SeizableItem::SelectionRule> (fields->loadField("SelectionRule", static_cast<int> (DEFAULT.selectionRule)));
		_saveAttribute = fields->loadField("SaveAttribute", DEFAULT.saveAttribute);
		_index = fields->loadField("Index", DEFAULT.index);
		if (_modeldataManager != nullptr) {
			if (_seizableType == SeizableItem::SeizableType::RESOURCE) {
				_resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Resource>(), _seizableName);
			} else if (_seizableType == SeizableItem::SeizableType::SET) {
				_resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Set>(), _seizableName);
			}
			//assert(_resourceOrSet != nullptr); // @TODO TraceError
		}
	} catch (...) {
		res = false;
	}
	return res;
}

bool SeizableItem::loadInstance(PersistenceRecord *fields, unsigned int parentIndex) {
	bool res = true;
	std::string num = Util::StrIndex(parentIndex);
	try {
		_seizableType = static_cast<SeizableItem::SeizableType> (fields->loadField("SeizableType" + num, static_cast<int> (DEFAULT.seizableType)));
		_seizableName = fields->loadField("ResourceName" + num, "");
		_quantityExpression = fields->loadField("QuantityExpression" + num, DEFAULT.quantityExpression);
		_selectionRule = static_cast<SeizableItem::SelectionRule> (fields->loadField("SelectionRule" + num, static_cast<int> (DEFAULT.selectionRule)));
		_saveAttribute = fields->loadField("SaveAttribute" + num, DEFAULT.saveAttribute);
		_index = fields->loadField("Index" + num, DEFAULT.index);
		if (_modeldataManager != nullptr) {
			if (_seizableType == SeizableItem::SeizableType::RESOURCE) {
				_resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Resource>(), _seizableName);
				if (_resourceOrSet == nullptr) {
					auto model = _modeldataManager->getParentModel();
					_resourceOrSet = model->getParentSimulator()->getPlugins()->newInstance<Resource>(model, _seizableName);
				}
			} else if (_seizableType == SeizableItem::SeizableType::SET) {
				_resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Set>(), _seizableName);
				if (_resourceOrSet == nullptr) {
					auto model = _modeldataManager->getParentModel();
					_resourceOrSet = model->getParentSimulator()->getPlugins()->newInstance<Set>(model, _seizableName);
				}
			} else {
				_resourceOrSet = nullptr;
				_modeldataManager->getParentModel()->getTracer()->traceError("SeizableItem named '" + _seizableName + "' could not be found on loading");
			}
		}
	} catch (...) {
		res = false;
	}
	return res;
}

void SeizableItem::saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaults) {
	std::string num = Util::StrIndex(parentIndex);
	fields->saveField("SeizableType" + num, static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType), saveDefaults);
	fields->saveField("ResourceName" + num, _resourceOrSet->getName(), "", saveDefaults);
	fields->saveField("QuantityExpression" + num, _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
	fields->saveField("SelectionRule" + num, static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
	fields->saveField("SaveAttribute" + num, _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
	fields->saveField("Index" + num, _index, DEFAULT.index, saveDefaults);
}

void SeizableItem::saveInstance(PersistenceRecord *fields, bool saveDefaults) {
	fields->saveField("SeizableType", static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType), saveDefaults);
	//fields->saveField("resourceId", _resourceOrSet->getId());
	fields->saveField("ResourceName", _resourceOrSet->getName(), "", saveDefaults);
	fields->saveField("QuantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
	fields->saveField("SelectionRule", static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
	fields->saveField("SaveAttribute", _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
	fields->saveField("Index", _index, DEFAULT.index, saveDefaults);
}

std::string SeizableItem::show() {
	return "resourceType=" + std::to_string(static_cast<int> (_seizableType)) + ",resource=\"" + _resourceOrSet->getName() + "\",quantityExpression=\"" + _quantityExpression + "\", selectionRule=" + std::to_string(static_cast<int> (_selectionRule)) + ", _saveAttribute=\"" + _saveAttribute + "\",index=\"" + _index + "\"";
}

void SeizableItem::setIndex(std::string index) {
	_index = index;
}

std::string SeizableItem::getIndex() const {
	return _index;
}

void SeizableItem::setSaveAttribute(std::string saveAttribute) {
	_saveAttribute = saveAttribute;
}

std::string SeizableItem::getSaveAttribute() const {
	return _saveAttribute;
}

void SeizableItem::setSelectionRule(SeizableItem::SelectionRule selectionRule) {
	_selectionRule = selectionRule;
}

SeizableItem::SelectionRule SeizableItem::getSelectionRule() const {
	return _selectionRule;
}

void SeizableItem::setQuantityExpression(std::string quantityExpression) {
	_quantityExpression = quantityExpression;
}

std::string SeizableItem::getQuantityExpression() const {
	return _quantityExpression;
}

std::string SeizableItem::getResourceName() const {
	return _seizableName;
}

void SeizableItem::setResource(Resource* resource) {
	_resourceOrSet = resource;
	_seizableName = resource->getName();
}

Resource* SeizableItem::getResource() const {
	return static_cast<Resource*> (_resourceOrSet);
}

void SeizableItem::setSet(Set* set) {
	_resourceOrSet = set;
	_seizableName = set->getName();
}

Set* SeizableItem::getSet() const {
	return static_cast<Set*> (_resourceOrSet);
}

void SeizableItem::setSeizableType(SeizableItem::SeizableType resourceType) {
	_seizableType = resourceType;
}

SeizableItem::SeizableType SeizableItem::getSeizableType() const {
	return _seizableType;
}

void SeizableItem::setLastMemberSeized(unsigned int lastMemberSeized) {
	_lastMemberSeized = lastMemberSeized;
}

unsigned int SeizableItem::getLastMemberSeized() const {
	return _lastMemberSeized;
}

ModelDataDefinition* SeizableItem::getSeizable() const {
	return _resourceOrSet;
}

void SeizableItem::setElementManager(ModelDataManager* _modeldataManager) {
	_modeldataManager = _modeldataManager;
}

void SeizableItem::setLastPreferedOrder(unsigned int _lastPreferedOrder) {
	_lastPreferedOrder = _lastPreferedOrder;
}

unsigned int SeizableItem::getLastPreferedOrder() const {
	return _lastPreferedOrder;
}

//void SeizableItem::setComponentManager(ComponentManager* _componentManager) {
//	_componentManager = _componentManager;
//}

