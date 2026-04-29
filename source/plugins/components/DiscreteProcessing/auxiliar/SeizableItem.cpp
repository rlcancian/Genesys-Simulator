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

#include "plugins/components/DiscreteProcessing/auxiliar/SeizableItem.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
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

void SeizableItem::_ensureSimulationControls(Model* model) {
    if (model == nullptr || _simulationControls == nullptr || _simulationControls->size() > 0) {
        return;
    }

    _modeldataManager = model->getDataManager();

    auto* propIndex = new SimulationControlGeneric<std::string>(
        std::bind(&SeizableItem::getIndex, this), std::bind(&SeizableItem::setIndex, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "Index", "");
    auto* propRule = new SimulationControlGenericEnum<SeizableItem::SelectionRule, SeizableItem>(
        std::bind(&SeizableItem::getSelectionRule, this),
        std::bind(&SeizableItem::setSelectionRule, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "SelectionRule", "");
    auto* propSaveAttribute = new SimulationControlGeneric<std::string>(
        std::bind(&SeizableItem::getSaveAttribute, this), std::bind(&SeizableItem::setSaveAttribute, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "SaveAttribute", "");
    auto* propQuantityExpression = new SimulationControlGeneric<std::string>(
        std::bind(&SeizableItem::getQuantityExpression, this), std::bind(&SeizableItem::setQuantityExpression, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "QuantityExpression", "");
    auto* propResource = new SimulationControlGenericClass<Resource*, Model*, Resource>(
        model,
        std::bind(&SeizableItem::getResource, this), std::bind(&SeizableItem::setResource, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), "", "Resource", "");
    auto* propSet = new SimulationControlGenericClass<Set*, Model*, Set>(
        model,
        std::bind(&SeizableItem::getSet, this), std::bind(&SeizableItem::setSet, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), "", "Set", "");
    auto* propLastMemberSeized = new SimulationControlGeneric<unsigned int>(
        std::bind(&SeizableItem::getLastMemberSeized, this), std::bind(&SeizableItem::setLastMemberSeized, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "LastMemberSeized", "");
    auto* propLastPreferedOrder = new SimulationControlGeneric<unsigned int>(
        std::bind(&SeizableItem::getLastPreferedOrder, this), std::bind(&SeizableItem::setLastPreferedOrder, this, std::placeholders::_1),
        Util::TypeOf<SeizableItem>(), getName(), "LastPreferedOrder", "");

    _addSimulationControl(propIndex);
    _addSimulationControl(propRule);
    _addSimulationControl(propSaveAttribute);
    _addSimulationControl(propQuantityExpression);
    _addSimulationControl(propResource);
    _addSimulationControl(propSet);
    _addSimulationControl(propLastMemberSeized);
    _addSimulationControl(propLastPreferedOrder);
}

SeizableItem::SeizableItem(ModelDataDefinition* resourceOrSet, std::string quantityExpression, SeizableItem::SelectionRule selectionRule, std::string saveAttribute, std::string index) {
    SeizableItem::SeizableType resourceType = DEFAULT.seizableType;
    if (dynamic_cast<Resource*> (resourceOrSet) != nullptr) {
        resourceType = SeizableItem::SeizableType::RESOURCE;
    } else if (dynamic_cast<Set*> (resourceOrSet) != nullptr) {
        resourceType = SeizableItem::SeizableType::SET;
        // A SeizableItem consumes resources; therefore, a referenced Set should default to
        // Resource members while still preserving any existing members already stored in the Set.
        dynamic_cast<Set*> (resourceOrSet)->setAllowedElementTypes({Util::TypeOf<Resource>()});
        dynamic_cast<Set*> (resourceOrSet)->setSetOfType(Util::TypeOf<Resource>());
    } else {
        //assert(false);//@TODO
    }
    _seizableType = resourceType;
    _resourceOrSet = resourceOrSet;
    _seizableName = resourceOrSet != nullptr ? resourceOrSet->getName() : "";
    _quantityExpression = quantityExpression;
    _selectionRule = selectionRule;
    _saveAttribute = saveAttribute;
    _index = index;
    if (resourceOrSet != nullptr) {
        _modeldataManager = resourceOrSet->getParentModel()->getDataManager();
        _ensureSimulationControls(resourceOrSet->getParentModel());
    }
}

SeizableItem::SeizableItem(Model* model, std::string resourceName, std::string quantityExpression, SeizableItem::SelectionRule selectionRule, std::string saveAttribute, std::string index) {
    ModelDataDefinition* resource = nullptr;
    if (!resourceName.empty()) {
        resource = model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), resourceName);
        if (resource == nullptr) {
            resource = model->getParentSimulator()->getPluginManager()->newInstance<Resource>(model, resourceName);
        }
    }
    _seizableName = resourceName;
    _resourceOrSet = resource;
    _quantityExpression = quantityExpression;
    _selectionRule = selectionRule;
    _saveAttribute = saveAttribute;
    _index = index;
    _modeldataManager = model != nullptr ? model->getDataManager() : nullptr;
    _ensureSimulationControls(model);
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
    _seizableName = original->getName();
    _lastMemberSeized = original->getLastMemberSeized();
    _lastPreferedOrder = original->getLastPreferedOrder();
    if (original->_modeldataManager != nullptr) {
        _modeldataManager = original->_modeldataManager;
        _ensureSimulationControls(_modeldataManager->getParentModel());
    } else if (_resourceOrSet != nullptr) {
        _modeldataManager = _resourceOrSet->getParentModel()->getDataManager();
        _ensureSimulationControls(_resourceOrSet->getParentModel());
    }
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
            if (Set* set = dynamic_cast<Set*>(_resourceOrSet)) {
                // Loading a SeizableItem re-applies the contextual contract for the GUI editor:
                // this Set reference is intended to create/select Resource members.
                set->setAllowedElementTypes({Util::TypeOf<Resource>()});
                set->setSetOfType(Util::TypeOf<Resource>());
            }
            _ensureSimulationControls(_modeldataManager->getParentModel());
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
                if (Set* set = dynamic_cast<Set*>(_resourceOrSet)) {
                    // Loading indexed SeizableItems uses the same Set contract as direct GUI editing.
                    set->setAllowedElementTypes({Util::TypeOf<Resource>()});
                    set->setSetOfType(Util::TypeOf<Resource>());
                }
            } else {
                _resourceOrSet = nullptr;
                _modeldataManager->getParentModel()->getTracer()->traceError("SeizableItem named '" + _seizableName + "' could not be found on loading");
            }
            _ensureSimulationControls(_modeldataManager->getParentModel());
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
    fields->saveField("requestSeizable" + num, _resourceOrSet != nullptr ? _resourceOrSet->getName() : "", "", saveDefaults);
    fields->saveField("requestQuantityExpression" + num, _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
    fields->saveField("requestSelectionRule" + num, static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
    fields->saveField("requestSaveAttribute" + num, _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
    fields->saveField("requestIndex" + num, _index, DEFAULT.index, saveDefaults);
}

void SeizableItem::saveInstance(PersistenceRecord *fields, bool saveDefaults) {
    fields->saveField("requestSeizableType", static_cast<int> (_seizableType), static_cast<int> (DEFAULT.seizableType), saveDefaults);
    //fields->saveField("resourceId", _resourceOrSet->getId());
    fields->saveField("requestSeizable", _resourceOrSet != nullptr ? _resourceOrSet->getName() : "", "", saveDefaults);
    fields->saveField("requestQuantityExpression", _quantityExpression, DEFAULT.quantityExpression, saveDefaults);
    fields->saveField("requestSelectionRule", static_cast<int> (_selectionRule), static_cast<int> (DEFAULT.selectionRule), saveDefaults);
    fields->saveField("requestSaveAttribute", _saveAttribute, DEFAULT.saveAttribute, saveDefaults);
    fields->saveField("requestIndex", _index, DEFAULT.index, saveDefaults);
}

std::string SeizableItem::show() {
    return "resourceType=" + std::to_string(static_cast<int> (_seizableType)) + ",resource=\"" + (_resourceOrSet != nullptr ? _resourceOrSet->getName() : "") + "\",quantityExpression=\"" + _quantityExpression + "\", selectionRule=" + std::to_string(static_cast<int> (_selectionRule)) + ", _saveAttribute=\"" + _saveAttribute + "\",index=\"" + _index + "\"";
}

void SeizableItem::_addSimulationControl(SimulationControl* control) {
    _simulationControls->insert(control);
}

List<SimulationControl*>* SeizableItem::getSimulationControls() const {
    return _simulationControls;
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
    _seizableType = SeizableType::RESOURCE;
    _seizableName = resource != nullptr ? resource->getName() : "";
    if (_modeldataManager == nullptr && resource != nullptr) {
        _modeldataManager = resource->getParentModel()->getDataManager();
    }
    if (_simulationControls != nullptr && _simulationControls->size() == 0 && _modeldataManager != nullptr) {
        _ensureSimulationControls(_modeldataManager->getParentModel());
    }
}

Resource* SeizableItem::getResource() const {
    return dynamic_cast<Resource*> (_resourceOrSet);
}

void SeizableItem::setSet(Set* set) {
    _resourceOrSet = set;
    _seizableType = SeizableType::SET;
    _seizableName = set != nullptr ? set->getName() : "";
    if (set != nullptr) {
        // The Set remains a generic data definition, but this association narrows what the
        // property editor should offer when the user creates new members through a SeizableItem.
        set->setAllowedElementTypes({Util::TypeOf<Resource>()});
        set->setSetOfType(Util::TypeOf<Resource>());
        if (_modeldataManager == nullptr) {
            _modeldataManager = set->getParentModel()->getDataManager();
        }
    }
    if (_simulationControls != nullptr && _simulationControls->size() == 0 && _modeldataManager != nullptr) {
        _ensureSimulationControls(_modeldataManager->getParentModel());
    }
}

Set* SeizableItem::getSet() const {
    return dynamic_cast<Set*> (_resourceOrSet);
}

//void SeizableItem::setSeizableType(SeizableItem::SeizableType resourceType) {
//    _seizableType = resourceType;
//}

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
