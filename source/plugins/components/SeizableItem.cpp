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


std::string SeizableItem::convertEnumToStr(SeizableType type) {
    switch (static_cast<int> (type)) {
        case 0: return "RESOURCE";
        case 1: return "SET";
    }
    return "Unknown";
}

std::string SeizableItem::convertEnumToStr(SelectionRule rule) {
    switch (static_cast<int> (rule)) {
        case 0: return "CYCLICAL";
        case 1: return "RANDOM";
        case 2: return "SPECIFICMEMBER";
        case 3: return "LARGESTREMAININGCAPACITY";
        case 4: return "SMALLESTNUMBERBUSY";
        case 5: return "PREFEREDORDER";
    }
    return "Unknown";
}

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
    if (resource == nullptr) {
        resource = model->getParentSimulator()->getPluginManager()->newInstance<Resource>(model, resourceName);
    }
    _seizableName = resourceName;

    SimulationControlGeneric<std::string>* propIndex = new SimulationControlGeneric<std::string>(
                                    std::bind(&SeizableItem::getIndex, this), std::bind(&SeizableItem::setIndex, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "Index", "");
    SimulationControlGenericEnum<SeizableItem::SelectionRule, SeizableItem>* propRule = new SimulationControlGenericEnum<SeizableItem::SelectionRule, SeizableItem>(
                                    std::bind(&SeizableItem::getSelectionRule, this),
                                    std::bind(&SeizableItem::setSelectionRule, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "SelectionRule", "");
    SimulationControlGenericEnum<SeizableItem::SeizableType, SeizableItem>* propType = new SimulationControlGenericEnum<SeizableItem::SeizableType, SeizableItem>(
                                    std::bind(&SeizableItem::getSeizableType, this),
                                    std::bind(&SeizableItem::setSeizableType, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "SeizableType", "");
    SimulationControlGeneric<std::string>* propSaveAttribute = new SimulationControlGeneric<std::string>(
                                    std::bind(&SeizableItem::getSaveAttribute, this), std::bind(&SeizableItem::setSaveAttribute, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "SaveAttribute", "");
    SimulationControlGeneric<std::string>* propQuantityExpression = new SimulationControlGeneric<std::string>(
                                    std::bind(&SeizableItem::getQuantityExpression, this), std::bind(&SeizableItem::setQuantityExpression, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "QuantityExpression", "");
    SimulationControlGenericClass<Resource*, Model*, Resource>* propResource = new SimulationControlGenericClass<Resource*, Model*, Resource>(
                                        model,
                                        std::bind(&SeizableItem::getResource, this), std::bind(&SeizableItem::setResource, this, std::placeholders::_1),
                                        Util::TypeOf<SeizableItem>(), "", "Resource", "");
    SimulationControlGenericClass<Set*, Model*, Set>* propSet = new SimulationControlGenericClass<Set*, Model*, Set>(
                                        model,
                                        std::bind(&SeizableItem::getSet, this), std::bind(&SeizableItem::setSet, this, std::placeholders::_1),
                                        Util::TypeOf<SeizableItem>(), "", "Set", "");
    SimulationControlGeneric<unsigned int>* propLastMemberSeized = new SimulationControlGeneric<unsigned int>(
                                    std::bind(&SeizableItem::getLastMemberSeized, this), std::bind(&SeizableItem::setLastMemberSeized, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "LastMemberSeized", "");
    SimulationControlGeneric<unsigned int>* propLastPreferedOrder = new SimulationControlGeneric<unsigned int>(
                                    std::bind(&SeizableItem::getLastPreferedOrder, this), std::bind(&SeizableItem::setLastPreferedOrder, this, std::placeholders::_1),
                                    Util::TypeOf<SeizableItem>(), getName(), "LastPreferedOrder", "");

    model->getControls()->insert(propIndex);
    model->getControls()->insert(propRule);
    model->getControls()->insert(propType);
    model->getControls()->insert(propSaveAttribute);
    model->getControls()->insert(propQuantityExpression);
    model->getControls()->insert(propResource);
    model->getControls()->insert(propSet);
    model->getControls()->insert(propLastMemberSeized);
    model->getControls()->insert(propLastPreferedOrder);

    // setting properties
    _addProperty(propIndex);
    _addProperty(propRule);
    _addProperty(propType);
    _addProperty(propSaveAttribute);
    _addProperty(propQuantityExpression);
    _addProperty(propResource);
    _addProperty(propSet);
    _addProperty(propLastMemberSeized);
    _addProperty(propLastPreferedOrder);

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
        _seizableType = static_cast<SeizableItem::SeizableType> (fields->loadField("requestSeizableType", static_cast<int> (DEFAULT.seizableType)));
        _seizableName = fields->loadField("requestSeizable", "");
        _quantityExpression = fields->loadField("requestQuantityExpression", DEFAULT.quantityExpression);
        _selectionRule = static_cast<SeizableItem::SelectionRule> (fields->loadField("requestSelectionRule", static_cast<int> (DEFAULT.selectionRule)));
        _saveAttribute = fields->loadField("requestSaveAttribute", DEFAULT.saveAttribute);
        _index = fields->loadField("requestIndex", DEFAULT.index);
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
        _seizableType = static_cast<SeizableItem::SeizableType> (fields->loadField("requestSeizableType" + num, static_cast<int> (DEFAULT.seizableType)));
        _seizableName = fields->loadField("requestSeizable" + num, "");
        _quantityExpression = fields->loadField("requestQuantityExpression" + num, DEFAULT.quantityExpression);
        _selectionRule = static_cast<SeizableItem::SelectionRule> (fields->loadField("requestSelectionRule" + num, static_cast<int> (DEFAULT.selectionRule)));
        _saveAttribute = fields->loadField("requestSaveAttribute" + num, DEFAULT.saveAttribute);
        _index = fields->loadField("requestIndex" + num, DEFAULT.index);
        if (_modeldataManager != nullptr) {
            if (_seizableType == SeizableItem::SeizableType::RESOURCE) {
                _resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Resource>(), _seizableName);
                if (_resourceOrSet == nullptr) {
                    auto model = _modeldataManager->getParentModel();
                    _resourceOrSet = model->getParentSimulator()->getPluginManager()->newInstance<Resource>(model, _seizableName);
                }
            } else if (_seizableType == SeizableItem::SeizableType::SET) {
                _resourceOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Set>(), _seizableName);
                if (_resourceOrSet == nullptr) {
                    auto model = _modeldataManager->getParentModel();
                    _resourceOrSet = model->getParentSimulator()->getPluginManager()->newInstance<Set>(model, _seizableName);
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
    fields->saveField("requestSeizableType" + num, static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType), saveDefaults);
    //_resourceOrSet->getName();
    fields->saveField("requestSeizable" + num, _resourceOrSet->getName(), "", saveDefaults);
    fields->saveField("requestQuantityExpression" + num, _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
    fields->saveField("requestSelectionRule" + num, static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
    fields->saveField("requestSaveAttribute" + num, _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
    fields->saveField("requestIndex" + num, _index, DEFAULT.index, saveDefaults);
}

void SeizableItem::saveInstance(PersistenceRecord *fields, bool saveDefaults) {
    fields->saveField("requestSeizableType", static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType), saveDefaults);
    //fields->saveField("resourceId", _resourceOrSet->getId());
    fields->saveField("requestSeizable", _resourceOrSet->getName(), "", saveDefaults);
    fields->saveField("requestQuantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
    fields->saveField("requestSelectionRule", static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
    fields->saveField("requestSaveAttribute", _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
    fields->saveField("requestIndex", _index, DEFAULT.index, saveDefaults);
}

std::string SeizableItem::show() {
    return "resourceType=" + std::to_string(static_cast<int> (_seizableType)) + ",resource=\"" + _resourceOrSet->getName() + "\",quantityExpression=\"" + _quantityExpression + "\", selectionRule=" + std::to_string(static_cast<int> (_selectionRule)) + ", _saveAttribute=\"" + _saveAttribute + "\",index=\"" + _index + "\"";
}

void SeizableItem::_addProperty(PropertyBase* property) {
    _properties->insert(property);
}

List<PropertyBase*>* SeizableItem::getProperties() const {
    return _properties;
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

std::string SeizableItem::getName() const {
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

void SeizableItem::setElementManager(ModelDataManager* modeldataManager) {
    _modeldataManager = modeldataManager;
}

void SeizableItem::setLastPreferedOrder(unsigned int lastPreferedOrder) {
    _lastPreferedOrder = lastPreferedOrder;
}

unsigned int SeizableItem::getLastPreferedOrder() const {
    return _lastPreferedOrder;
}

//void SeizableItem::setComponentManager(ComponentManager* _componentManager) {
//	_componentManager = _componentManager;
//}
