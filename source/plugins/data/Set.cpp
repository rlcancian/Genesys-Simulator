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
#include <vector>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &Set::GetPluginInformation;
}
#endif

ModelDataDefinition* Set::NewInstance(Model* model, std::string name) {
    return new Set(model, name);
}

Set::Set(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Set>(), name) {
    SimulationControlGeneric<std::string>* propSetOfType = new SimulationControlGeneric<std::string>(
                std::bind(&Set::getSetOfType, this),
                std::bind(&Set::setSetOfType, this, std::placeholders::_1),
                Util::TypeOf<Set>(), getName(), "SetOfType", "");
    SimulationControlGenericListPointer<ModelDataDefinition*, Model*, ModelDataDefinition>* propElementSet = new SimulationControlGenericListPointer<ModelDataDefinition*, Model*, ModelDataDefinition> (
                _parentModel,
                std::bind(&Set::getElementSet, this), std::bind(&Set::addElementSet, this, std::placeholders::_1), std::bind(&Set::removeElementSet, this, std::placeholders::_1),
                Util::TypeOf<Set>(), getName(), "ElementSet", "");

    _parentModel->getControls()->insert(propSetOfType);
    _parentModel->getControls()->insert(propElementSet);

    // setting property
    _addProperty(propSetOfType);
    _addProperty(propElementSet);
}

Set::~Set() {
    delete _elementSet;
    _elementSet = nullptr;
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

void Set::addElementSet(ModelDataDefinition* newElement) {
    _elementSet->insert(newElement);
    if (_elementSet->size() == 1 && (_setOfType.empty() || _setOfType == DEFAULT.setOfType)) {
        _setOfType = newElement->getClassname();
    }
}

void Set::removeElementSet(ModelDataDefinition* element) {
    _elementSet->remove(element);
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
            _elementSet->clear();
            _setOfType = fields->loadField("type", DEFAULT.setOfType);
            unsigned int memberSize = fields->loadField("members", DEFAULT.membersSize);
            for (unsigned int i = 0; i < memberSize; i++) {
                std::string memberName = fields->loadField("member" + Util::StrIndex(i));
                ModelDataDefinition* member = _parentModel->getDataManager()->getDataDefinition(_setOfType, memberName);
                if (member == nullptr) { // not found. That's a problem. Resource not loaded yet (or mismatch name
                    traceError("ERROR: Could not found " + _setOfType + " set member named \"" + memberName + "\"", TraceManager::Level::L1_errorFatal);
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
    fields->saveField("type", _setOfType, DEFAULT.setOfType, saveDefaultValues);
    fields->saveField("members", _elementSet->size(), DEFAULT.membersSize, saveDefaultValues);
    unsigned int i = 0;
    for (ModelDataDefinition* modeldatum : *_elementSet->list()) {
        fields->saveField("member" + Util::StrIndex(i), modeldatum->getName());
        i++;
    }
}

bool Set::_check(std::string& errorMessage) {
    bool resultAll = true;
    std::vector<std::string> previousMemberKeys;
    for (const auto& pair : *getAttachedData()) {
        if (pair.first.rfind("Member", 0) == 0) {
            previousMemberKeys.push_back(pair.first);
        }
    }
    for (const std::string& key : previousMemberKeys) {
        _attachedDataRemove(key);
    }

    if (_elementSet->size() > 0) {
        unsigned int i = 0;
        for (ModelDataDefinition* data : *_elementSet->list()) {
            std::string currentType = data->getClassname();
            if (_setOfType.empty()) {
                _setOfType = currentType;
            } else if (_setOfType != currentType) {
                resultAll = false;
                errorMessage += "Set is of type \"" + _setOfType + "\" but member[" + Util::StrIndex(i) + "]=\"" + data->getName() + "\" is of type \"" + currentType + "\"";
            }
            _attachedDataInsert("Member" + Util::StrIndex(i), data);
            ++i;
        }
    }
    return resultAll;
}

ParserChangesInformation* Set::_getParserChangesInformation() {
    ParserChangesInformation* changes = new ParserChangesInformation();
    //changes->getProductionToAdd()->insert(...);
    //changes->getTokensToAdd()->insert(...);
    return changes;
}

void Set::_createInternalAndAttachedData() {
    std::vector<std::string> previousSetMemberKeys;
    const std::string setMemberPrefix = getName() + ".";
    for (const auto& pair : *getAttachedData()) {
        if (pair.first.rfind(setMemberPrefix, 0) == 0) {
            previousSetMemberKeys.push_back(pair.first);
        }
    }
    for (const std::string& key : previousSetMemberKeys) {
        _attachedDataRemove(key);
    }

    for(ModelDataDefinition* dd: *_elementSet->list()) {
        _attachedDataInsert(getName()+"."+dd->getName(), dd);
    }
}
