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

#include "plugins/data/DiscreteProcessing/Set.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include <algorithm>
#include <vector>

namespace {
std::vector<std::string> uniqueNonEmptyTypes(const std::vector<std::string>& typeNames) {
    std::vector<std::string> result;
    for (const std::string& typeName : typeNames) {
        if (!typeName.empty() && std::find(result.begin(), result.end(), typeName) == result.end()) {
            result.push_back(typeName);
        }
    }
    return result;
}

bool containsTypeName(const std::vector<std::string>& typeNames, const std::string& typeName) {
    return std::find(typeNames.begin(), typeNames.end(), typeName) != typeNames.end();
}
}

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &Set::GetPluginInformation;
}
#endif

ModelDataDefinition* Set::NewInstance(Model* model, std::string name) {
    return new Set(model, name);
}

Set::Set(Model* model, std::string name, std::vector<std::string> allowedElementTypes) :
    ModelDataDefinition(model, Util::TypeOf<Set>(), name),
    _allowedElementTypes(uniqueNonEmptyTypes(allowedElementTypes)) {
    //SimulationControlGeneric<std::string>* propSetOfType = new SimulationControlGeneric<std::string>(
    //            std::bind(&Set::getSetOfType, this),
    //            std::bind(&Set::setSetOfType, this, std::placeholders::_1),
    //            Util::TypeOf<Set>(), getName(), "SetOfType", "");
    SimulationControlGenericListPointer<ModelDataDefinition*, Model*, ModelDataDefinition>* propElementSet = new SimulationControlGenericListPointer<ModelDataDefinition*, Model*, ModelDataDefinition> (
                _parentModel,
                std::bind(&Set::getElementSet, this),
                std::bind(&Set::addElementSet, this, std::placeholders::_1),
                std::bind(&Set::removeElementSet, this, std::placeholders::_1),
                Util::TypeOf<Set>(), getName(), "ElementSet", "",
                true, true, false,
                nullptr, nullptr,
                std::bind(&Set::getAllowedElementTypes, this),
                std::bind(&Set::getSetOfType, this),
                std::bind(&Set::setSetOfType, this, std::placeholders::_1),
                [this](Model*, const std::string& typeName, const std::string& name) {
                    return createElementSetOfType(typeName, name);
                },
                [this](int index) {
                    if (index < 0 || _elementSet == nullptr) {
                        return static_cast<ModelDataDefinition*>(nullptr);
                    }
                    return _elementSet->getAtRank(static_cast<unsigned int>(index));
                });

    //_parentModel->getControls()->insert(propSetOfType);
    _parentModel->getControls()->insert(propElementSet);

    // setting property
    //_addSimulationControl(propSetOfType);
    _addSimulationControl(propElementSet);
}

Set::~Set() {
    delete _elementSet;
    _elementSet = nullptr;
}

std::string Set::show() {
    return ModelDataDefinition::show() +
            "";
}

std::string Set::getSetOfType() const {
    return _setOfType;
}

bool Set::setSetOfType(const std::string& setOfType) {
    if (setOfType.empty()) {
        if (!canChangeSetOfType()) {
            traceError("Could not clear Set type after members have already been inserted", TraceManager::Level::L3_errorRecover);
            return false;
        }
        _setOfType.clear();
        return true;
    }

    // Only an explicit Set contract restricts the type.  The plugin list returned by
    // getAllowedElementTypes() is an editor discovery aid and must not reject programmatic inserts
    // when the Set owner has not narrowed the allowed types.
    if (!_allowedElementTypes.empty() && !containsTypeName(_allowedElementTypes, setOfType)) {
        traceError("Could not configure Set \"" + getName() + "\" as a Set of \"" + setOfType + "\" because this type is not allowed", TraceManager::Level::L3_errorRecover);
        return false;
    }

    if (!canChangeSetOfType() && _setOfType != setOfType) {
        traceError("Could not change Set \"" + getName() + "\" type from \"" + _setOfType + "\" to \"" + setOfType + "\" after members have already been inserted", TraceManager::Level::L3_errorRecover);
        return false;
    }

    _setOfType = setOfType;
    return true;
}

bool Set::canChangeSetOfType() const {
    return _elementSet == nullptr || _elementSet->size() == 0;
}

void Set::setAllowedElementTypes(const std::vector<std::string>& allowedElementTypes) {
    _allowedElementTypes = uniqueNonEmptyTypes(allowedElementTypes);

    // Keep persisted or already-selected Set types visible to the editor.  A Set with existing
    // Resource members must never become uneditable simply because a narrower context was applied later.
    if (!_setOfType.empty() && !containsTypeName(_allowedElementTypes, _setOfType)) {
        _allowedElementTypes.push_back(_setOfType);
    }
}

void Set::addAllowedElementType(const std::string& allowedElementType) {
    if (!allowedElementType.empty() && !containsTypeName(_allowedElementTypes, allowedElementType)) {
        _allowedElementTypes.push_back(allowedElementType);
    }
}

std::vector<std::string> Set::getAllowedElementTypes() const {
    if (!_allowedElementTypes.empty()) {
        return _allowedElementTypes;
    }

    Simulator* simulator = _parentModel != nullptr ? _parentModel->getParentSimulator() : nullptr;
    PluginManager* pluginManager = simulator != nullptr ? simulator->getPluginManager() : nullptr;
    if (pluginManager != nullptr) {
        return uniqueNonEmptyTypes(pluginManager->getDataDefinitionPluginTypenames());
    }

    if (!_setOfType.empty()) {
        return {_setOfType};
    }
    return {};
}

List<ModelDataDefinition*>* Set::getElementSet() const {
    return _elementSet;
}

void Set::addElementSet(ModelDataDefinition* newElement) {
    if (newElement == nullptr) {
        traceError("Could not insert a null element in Set \"" + getName() + "\"", TraceManager::Level::L3_errorRecover);
        return;
    }
    if (_setOfType.empty() && !setSetOfType(newElement->getClassname())) {
        return;
    }
    if (newElement->getClassname()==_setOfType) {
        _elementSet->insert(newElement);
    } else {
        traceError("Cound not insert element type \""+newElement->getClassname()+"\" in a Set of type \""+_setOfType+"\"", TraceManager::Level::L3_errorRecover);
    }
}

void Set::removeElementSet(ModelDataDefinition* element) {
    _elementSet->remove(element);
}

ModelDataDefinition* Set::createElementSetOfType(const std::string& typeName, const std::string& name) {
    const std::string effectiveTypeName = typeName.empty() ? _setOfType : typeName;
    if (effectiveTypeName.empty()) {
        traceError("Could not create a Set member without a selected Set type", TraceManager::Level::L3_errorRecover);
        return nullptr;
    }
    if (!setSetOfType(effectiveTypeName)) {
        return nullptr;
    }

    Simulator* simulator = _parentModel != nullptr ? _parentModel->getParentSimulator() : nullptr;
    PluginManager* pluginManager = simulator != nullptr ? simulator->getPluginManager() : nullptr;
    if (pluginManager == nullptr) {
        traceError("Could not create Set member of type \"" + effectiveTypeName + "\" because the model has no PluginManager", TraceManager::Level::L3_errorRecover);
        return nullptr;
    }

    // Creation stays in the kernel so the GUI only expresses the user's intent; plugin lookup,
    // default naming and model registration keep following the same path as normal data creation.
    ModelDataDefinition* newElement = pluginManager->newInstance(effectiveTypeName, _parentModel, name);
    if (newElement == nullptr) {
        traceError("Could not create Set member of type \"" + effectiveTypeName + "\"", TraceManager::Level::L3_errorRecover);
        return nullptr;
    }
    if (newElement->getClassname() != _setOfType) {
        traceError("Created Set member type \"" + newElement->getClassname() + "\" does not match Set type \"" + _setOfType + "\"", TraceManager::Level::L3_errorRecover);
        return nullptr;
    }
    return newElement;
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

void Set::_createReportStatisticsDataDefinitions() {
}

void Set::_createEditableDataDefinitions() {
}

void Set::_createOthersDataDefinitions() {
}
