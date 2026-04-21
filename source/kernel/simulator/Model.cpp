/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SimulationModel.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 21 de Junho de 2018, 15:01
 */

#include "Model.h"
#include <typeinfo>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <fstream>
#include <filesystem>
#include <unordered_set>
//#include <stdio.h>

#include "SourceModelComponent.h"
#include "Simulator.h"
#include "OnEventManager.h"
#include "StatisticsCollector.h"
#include "../TraitsKernel.h"
//#include "Access.h"

//using namespace GenesysKernel;

bool EventCompare(const Event* a, const Event* b) {
    return a->getTime() < b->getTime();
}

namespace {
void SetDataDefinitionChanged(ModelDataDefinition* dataDefinition,
                              bool hasChanged,
                              std::unordered_set<ModelDataDefinition*>* visited) {
    if (dataDefinition == nullptr || visited == nullptr || visited->find(dataDefinition) != visited->end()) {
        return;
    }

    visited->insert(dataDefinition);
    dataDefinition->setHasChanged(hasChanged);

    for (const auto& child : *dataDefinition->getInternalData()) {
        SetDataDefinitionChanged(child.second, hasChanged, visited);
    }
    for (const auto& child : *dataDefinition->getAttachedData()) {
        SetDataDefinitionChanged(child.second, hasChanged, visited);
    }
}

bool IsRegisteredDataDefinition(ModelDataManager* manager, ModelDataDefinition* dataDefinition) {
    if (manager == nullptr || dataDefinition == nullptr) {
        return false;
    }
    List<ModelDataDefinition*>* typedList = manager->getDataDefinitionList(dataDefinition->getClassname());
    if (typedList == nullptr) {
        return false;
    }
    return typedList->find(dataDefinition) != typedList->list()->end();
}

bool HasAttachedReferenceOutsideRemoval(ModelDataDefinition* candidate,
                                        ModelDataManager* dataManager,
                                        ComponentManager* componentManager,
                                        const std::unordered_set<ModelDataDefinition*>& removedDataDefinitions,
                                        const std::unordered_set<ModelComponent*>& removedComponents) {
    if (candidate == nullptr || dataManager == nullptr || componentManager == nullptr) {
        return false;
    }

    // Attached references are non-owning shares; only other live attached references keep a candidate alive.
    const std::list<std::string> dataTypes = dataManager->getDataDefinitionClassnames();
    for (const std::string& dataType : dataTypes) {
        List<ModelDataDefinition*>* dataDefinitionList = dataManager->getDataDefinitionList(dataType);
        if (dataDefinitionList == nullptr) {
            continue;
        }
        for (ModelDataDefinition* owner : *dataDefinitionList->list()) {
            if (owner == nullptr || removedDataDefinitions.find(owner) != removedDataDefinitions.end()) {
                continue;
            }
            for (const auto& attachedData : *owner->getAttachedData()) {
                if (attachedData.second == candidate) {
                    return true;
                }
            }
        }
    }

    for (ModelComponent* component : *componentManager->getAllComponents()) {
        if (component == nullptr || removedComponents.find(component) != removedComponents.end()) {
            continue;
        }
        for (const auto& attachedData : *component->getAttachedData()) {
            if (attachedData.second == candidate) {
                return true;
            }
        }
    }

    return false;
}
}

Model::Model(Simulator* simulator, unsigned int level) {
    _parentSimulator = simulator; // a simulator is the "parent" of a model
    _level = level;
    // for process analyser (create this lists before other component add any contyrol or response
    _responses = new List<SimulationResponse*>();
    _controls = new List<SimulationControl*>();
    // 1:1 associations (no Traits)
    _traceManager = simulator->getTraceManager();
    // every model starts with the same tracer, unless a specific one is set
    _modelInfo = new ModelInfo(); //Sampler_if* sampler = new Traits<Sampler_if>::Implementation();

    _eventManager = new OnEventManager(); // should be on .h (all that does not depends on THIS)
    _modeldataManager = new ModelDataManager(this);
    _componentManager = new ComponentManager(this);
    _simulation = new ModelSimulation(this);
    // 1:1 associations (Traits)
    _parser = new TraitsKernel<Parser_if>::Implementation(this, new TraitsKernel<Sampler_if>::Implementation());
    _modelChecker = new TraitsKernel<ModelChecker_if>::Implementation(this);
    _modelPersistence = new TraitsKernel<ModelPersistence_if>::Implementation(this);
    // 1:n associations
    _futureEvents = new List<Event*>(); // The future events list must be chronologicaly sorted
    //_events->setSortFunc(&EventCompare); // It works too
    _futureEvents->setSortFunc([](const Event* a, const Event* b) {
        return a->getTime() < b->getTime(); // Events are sorted chronologically
    });

    // insert controls
    /*
    GetterMemberT<unsigned int>::Getter getter = DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getNumberOfReplications);
    SetterMemberT<unsigned int>::Setter setter = DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setNumberOfReplications);
    SimulationResponseSpecific<unsigned int>* response = new SimulationResponseSpecific<unsigned int>(getter);
                                                         //SimulationResponseSpecific<unsigned int>(
                                                           //"ModelSimulation", "NumberOfReplications", getter);

    SimulationControlSpecific<unsigned int>* control = new SimulationControlSpecific<unsigned int>(
                                                           "ModelSimulation", "NumberOfReplications", getter, setter);
    controls->insert(control);
     */
    /*
    _controls->insert(new SimulationControl("ModelSimulation", "ReplicationLength",
            DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getReplicationLength),
            DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setReplicationLength)) );
    _controls->insert(new SimulationControl("ModelSimulation", "WarmupPeriod",
            DefineGetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::getWarmUpPeriod),
            DefineSetterMember<ModelSimulation>(this->_simulation, &ModelSimulation::setWarmUpPeriod)) );
     */
    //for NEW process analyser

    // insert NEW controls
    //_responsesNew = new List<SimulationResponse*>();
    //_controlsNew = new List<SimulationControl*>();
    /*
    _controls->insert(new PropertyT<unsigned int>("ModelSimulation", "NumberOfReplications",
            DefineGetter<ModelSimulation, unsigned int>(this->_simulation, &ModelSimulation::getNumberOfReplications),
            DefineSetter<ModelSimulation, unsigned int>(this->_simulation, &ModelSimulation::setNumberOfReplications)));
    _controls->insert(new PropertyT<double>("ModelSimulation", "ReplicationLength",
            DefineGetter<ModelSimulation, double>(this->_simulation, &ModelSimulation::getReplicationLength),
            DefineSetter<ModelSimulation, double>(this->_simulation, &ModelSimulation::setReplicationLength)));
    _controls->insert(new PropertyT<double>("ModelSimulation", "WarmupPeriod",
            DefineGetter<ModelSimulation, double>(this->_simulation, &ModelSimulation::getWarmUpPeriod),
            DefineSetter<ModelSimulation, double>(this->_simulation, &ModelSimulation::setWarmUpPeriod)));
     */
}

// Explicitly destroys model-owned runtime and manager infrastructure in a safe order.
Model::~Model() {
    // Releases pending heap events owned by the model calendar before managers are destroyed.
    _destroyFutureEvents();
    // Releases transient entity instances still alive in the model runtime list.
    _destroyTransientEntities();
    // Releases remaining components while letting component destructors update manager state.
    _destroyComponents();
    // Releases non-entity model data definitions that were not already released by components.
    _destroyModelDataDefinitions();

    // Destroys runtime services owned by Model (non-owned pointers are intentionally not deleted).
    delete _simulation;
    _simulation = nullptr;
    delete _modelPersistence;
    _modelPersistence = nullptr;
    delete _modelChecker;
    _modelChecker = nullptr;
    delete _parser;
    _parser = nullptr;

    // Destroys infrastructure containers owned by Model after contained objects were released.
    delete _futureEvents;
    _futureEvents = nullptr;
    delete _controls;
    _controls = nullptr;
    delete _responses;
    _responses = nullptr;
    delete _componentManager;
    _componentManager = nullptr;
    delete _modeldataManager;
    _modeldataManager = nullptr;
    delete _eventManager;
    _eventManager = nullptr;
    delete _modelInfo;
    _modelInfo = nullptr;
}

void Model::sendEntityToComponent(Entity* entity, Connection* connection, double timeDelay) {
    if (connection == nullptr) {
        this->getTracer()->traceSimulation(this, TraceManager::Level::L3_errorRecover,
                                           "Model::sendEntityToComponent skipped: null connection");
        return;
    }
    if (connection->component == nullptr) {
        this->getTracer()->traceSimulation(this, TraceManager::Level::L3_errorRecover,
                                           "Model::sendEntityToComponent skipped: null destination component");
        return;
    }
    this->sendEntityToComponent(entity, connection->component, timeDelay, connection->channel.portNumber);
}

void Model::sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay,
                                  unsigned int componentinputPortNumber) {
    auto se = _simulation->_createSimulationEvent();
    se->setDestinationComponent(component);
    se->setEntityMoveTimeDelay(timeDelay);
    // Log emitted move-event endpoints before notifying GUI/observer handlers.
    ModelComponent* sourceComponent = (se->getCurrentEvent() != nullptr
                                           ? se->getCurrentEvent()->getComponent()
                                           : nullptr);
    std::string sourceName = (sourceComponent != nullptr ? sourceComponent->getName() : "<null>");
    std::string destinationName = (component != nullptr ? component->getName() : "<null>");
    std::string message = "Entity move event emitted sourceId=" + std::to_string(
            sourceComponent != nullptr ? sourceComponent->getId() : 0)
        + " sourceName=" + sourceName
        + " destinationId=" + std::to_string(component != nullptr ? component->getId() : 0)
        + " destinationName=" + destinationName;
    this->getTracer()->traceSimulation(this, TraceManager::Level::L8_detailed, message);
    this->getOnEventManager()->NotifyEntityMoveHandlers(se.get()); // it's my friend
    Event* newEvent = new Event(this->getSimulation()->getSimulatedTime() + timeDelay, entity, component,
                                componentinputPortNumber);
    this->getFutureEvents()->insert(newEvent);
}

bool Model::save(std::string filename) {
    bool res = this->_modelPersistence->save(filename);
    if (res) {
        this->_traceManager->trace("Model successfully saved", TraceManager::Level::L2_results);
        // @ToDo: (pequena alteração): Create a onModelSave event handler
    }
    else {
        this->_traceManager->trace("Model could not be saved", TraceManager::Level::L2_results);
    }
    return res;
}

bool Model::load(std::string filename) {
    this->clear();
    bool res = this->_modelPersistence->load(filename);
    if (res)
        this->_traceManager->trace("Model successfully loaded", TraceManager::Level::L2_results);
        // @ToDo: (pequena alteração): Create a onModelLoad event handler
    else
        this->_traceManager->trace("Model could not be loaded", TraceManager::Level::L2_results);
    return res;
}

double Model::parseExpression(const std::string expression) {
    try {
        double res = _parser->parse(expression);
        //yy::location l;
        //std::string m;
        //_parser->getParser().error(l, m);
        //std::cout << "l:" <<l<<", m:"<<m<<std::endl;
        return res;
    }
    catch (const std::exception& e) {
        // @ToDo: (importante): Create a onParserError event handler
        return 0.0; // @ToDo: (importante): HOW SAY THERE WAS AN ERROR?
    }
}

bool Model::checkExpression(const std::string expression, const std::string expressionName, std::string& errorMessage) {
    bool result = false;
    getTracer()->trace("Checking expression \"" + expression + "\"", TraceManager::Level::L8_detailed);
    try {
        parseExpression(expression, result, errorMessage);
    }
    catch (const std::exception& e) {
        result = false;
    }
    if (!result) {
        std::string msg = "Expression \"" + expression + "\" for '" + expressionName + "' is incorrect. ";
        this->_traceManager->trace(msg, TraceManager::Level::L3_errorRecover);
        errorMessage.append(msg);
    }
    return result;
}

void Model::checkReferencesToDataDefinitions(std::string expression,
                                             std::map<std::string, std::list<std::string>*>*
                                             referencedDataDefinitions) {
    genesyspp_driver wrapper = _parser->getParser();
    wrapper.setRegisterReferedDataElements(true);
    wrapper.clearReferedDataElements();
    wrapper.parse_str(expression);
    std::map<std::string, std::list<std::string>*>* refs = wrapper.getReferedDataElements();
    // Deep-copy referred element names so output map owns stable lists beyond parser wrapper lifetime.
    for (const auto& typeAndNames : *refs) {
        const std::string& typeName = typeAndNames.first;
        const std::list<std::string>* sourceNames = typeAndNames.second;
        if (sourceNames == nullptr) {
            continue;
        }
        std::list<std::string>*& destinationNames = (*referencedDataDefinitions)[typeName];
        if (destinationNames == nullptr) {
            destinationNames = new std::list<std::string>();
        }
        for (const std::string& referencedName : *sourceNames) {
            if (std::find(destinationNames->begin(), destinationNames->end(), referencedName) == destinationNames->
                end()) {
                destinationNames->push_back(referencedName);
            }
        }
    }
    wrapper.setRegisterReferedDataElements(false);
}

double Model::parseExpression(const std::string expression, bool& success, std::string& errorMessage) {
    double value = _parser->parse(expression, success, errorMessage);
    //yy::location l/
    //std::string m;
    //_parser->getParser().error(l, m);
    //std::cout << "l:" <<l<<", m:"<<m<<std::endl;
    return value;
}

std::string Model::showLanguage() {
    const std::string tempfilename = ".temp.gen";
    std::string result = "";
    TraceManager::Level oldLevel = getTracer()->getTraceLevel();
    getTracer()->setTraceLevel(TraceManager::Level::L0_noTraces);
    save(tempfilename);
    getTracer()->setTraceLevel(oldLevel);
    std::ifstream file(tempfilename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            result += line + '\n';
        }
        file.close();
        Util::FileDelete(tempfilename);
    }
    return result;
}

void Model::show() {
    getTracer()->trace("Simulation Model:", TraceManager::Level::L2_results);
    Util::IncIndent();
    {
        getTracer()->trace("Information:", TraceManager::Level::L2_results);
        Util::IncIndent();
        getTracer()->trace(this->getInfos()->show(), TraceManager::Level::L2_results);
        Util::DecIndent();
        _showConnections();
        _showComponents();
        _showElements();
        _showSimulationControls();
        _showSimulationResponses();
    }
    Util::DecIndent();
    getTracer()->trace("End of Simulation Model", TraceManager::Level::L2_results);
}

bool Model::insert(ModelDataDefinition* elemOrComp) {
    elemOrComp->setModelLevel(_level);
    ModelComponent* comp = dynamic_cast<ModelComponent*>(elemOrComp);
    if (comp == nullptr) // it's a ModelDataDefinition
        return this->getDataManager()->insert(elemOrComp);
    else // it's a ModelComponent
        return this->getComponentManager()->insert(comp);
}

void Model::remove(ModelDataDefinition* elemOrComp) {
    ModelComponent* comp = dynamic_cast<ModelComponent*>(elemOrComp);
    if (comp == nullptr) // it's a ModelDataDefinition
        this->getDataManager()->remove(elemOrComp);
    else // it's a ModelComponent
        this->getComponentManager()->remove(comp);
}

std::list<ModelDataDefinition*> Model::collectDataDefinitionsRemovedWith(
    const std::list<ModelDataDefinition*>& roots) const {
    std::list<ModelDataDefinition*> removableDataDefinitions;
    std::unordered_set<ModelDataDefinition*> removedDataDefinitions;
    std::unordered_set<ModelDataDefinition*> processedOwners;
    std::unordered_set<ModelDataDefinition*> attachedCandidates;
    std::unordered_set<ModelComponent*> removedComponents;

    auto collectOwner = [&](ModelDataDefinition* owner, auto&& collectOwnerRef) -> void {
        if (owner == nullptr || processedOwners.find(owner) != processedOwners.end()) {
            return;
        }
        processedOwners.insert(owner);

        if (ModelComponent* component = dynamic_cast<ModelComponent*>(owner)) {
            removedComponents.insert(component);
        }
        if (IsRegisteredDataDefinition(_modeldataManager, owner) &&
            removedDataDefinitions.insert(owner).second) {
            removableDataDefinitions.push_back(owner);
        }

        // Internal data definitions are composed by the owner and leave with it.
        for (const auto& internalData : *owner->getInternalData()) {
            collectOwnerRef(internalData.second, collectOwnerRef);
        }
        // Attached data definitions become candidates; they are removed only if no other live owner shares them.
        for (const auto& attachedData : *owner->getAttachedData()) {
            if (attachedData.second != nullptr) {
                attachedCandidates.insert(attachedData.second);
            }
        }
    };

    for (ModelDataDefinition* root : roots) {
        collectOwner(root, collectOwner);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        const std::unordered_set<ModelDataDefinition*> currentCandidates = attachedCandidates;
        for (ModelDataDefinition* candidate : currentCandidates) {
            if (candidate == nullptr || processedOwners.find(candidate) != processedOwners.end()) {
                continue;
            }
            if (!IsRegisteredDataDefinition(_modeldataManager, candidate)) {
                continue;
            }
            if (!HasAttachedReferenceOutsideRemoval(candidate,
                                                   _modeldataManager,
                                                   _componentManager,
                                                   removedDataDefinitions,
                                                   removedComponents)) {
                collectOwner(candidate, collectOwner);
                changed = true;
            }
        }
    }

    return removableDataDefinitions;
}

void Model::_showElements() const {
    getTracer()->trace("DataDefinitions:", TraceManager::Level::L2_results);
    Util::IncIndent();
    {
        std::string elementType;
        ModelDataDefinition* modeldatum;
        // Iterate over a value snapshot of class names to avoid manual delete semantics.
        std::list<std::string> elementTypes = getDataManager()->getDataDefinitionClassnames();
        for (std::list<std::string>::iterator typeIt = elementTypes.begin(); typeIt != elementTypes.end(); typeIt++) {
            elementType = (*typeIt);
            List<ModelDataDefinition*>* em = getDataManager()->getDataDefinitionList(elementType);
            getTracer()->trace(elementType + ":", TraceManager::Level::L2_results);
            Util::IncIndent();
            {
                for (std::list<ModelDataDefinition*>::iterator it = em->list()->begin(); it != em->list()->end(); it
                     ++) {
                    modeldatum = (*it);
                    getTracer()->trace(modeldatum->show(), TraceManager::Level::L2_results);
                }
            }
            Util::DecIndent();
        }
    }
    Util::DecIndent();
}

void Model::_showConnections() const {
    // @ToDo: (pequena alteração): show model connections
}

void Model::_showComponents() const {
    getTracer()->trace("Components:", TraceManager::Level::L2_results);
    Util::IncIndent();
    for (ModelComponent* component : *getComponentManager()) {
        getTracer()->trace(component->show(), TraceManager::Level::L2_results); ////
    }
    Util::DecIndent();
}

void Model::_showSimulationControls() const {
    getTracer()->trace("Simulation Controls:", TraceManager::Level::L2_results);
    Util::IncIndent();
    for (SimulationControl* control : *_controls->list()) {
        getTracer()->trace(control->show(), TraceManager::Level::L2_results); ////
    }
    Util::DecIndent();
}

void Model::_showSimulationResponses() const {
    getTracer()->trace("Simulation Responses:", TraceManager::Level::L2_results);
    Util::IncIndent();
    for (SimulationResponse* response : *_responses->list()) {
        getTracer()->trace(response->show(), TraceManager::Level::L2_results); ////
    }
    Util::DecIndent();
}

void Model::clear() {
    // Clears and destroys pending runtime events to avoid leaking queued Event objects.
    _destroyFutureEvents();
    // Clears and destroys transient entities that may still exist between runs.
    _destroyTransientEntities();
    // Clears and destroys components currently owned by the model.
    _destroyComponents();
    // Clears and destroys remaining non-entity data definitions tracked by the model.
    _destroyModelDataDefinitions();
    Util::ResetAllIds();
}

// Iterates over the future event list and deletes each pending heap-allocated event.
void Model::_destroyFutureEvents() {
    if (_futureEvents == nullptr) {
        return;
    }
    while (!_futureEvents->empty()) {
        Event* event = _futureEvents->front();
        _futureEvents->pop_front();
        delete event;
    }
}

// Iterates over the live entity list and deletes each transient heap-allocated entity.
void Model::_destroyTransientEntities() {
    if (_modeldataManager == nullptr) {
        return;
    }
    List<ModelDataDefinition*>* entities = _modeldataManager->getDataDefinitionList(Util::TypeOf<Entity>());
    while (entities != nullptr && !entities->empty()) {
        ModelDataDefinition* data = entities->front();
        Entity* entity = dynamic_cast<Entity*>(data);
        if (entity == nullptr) {
            entities->pop_front();
            continue;
        }
        delete entity;
    }
}

// Iterates over remaining components and deletes them so their destructors keep manager state consistent.
void Model::_destroyComponents() {
    if (_componentManager == nullptr) {
        return;
    }
    while (_componentManager->getNumberOfComponents() > 0) {
        ModelComponent* component = _componentManager->front();
        if (component == nullptr) {
            break;
        }
        delete component;
    }
}

// Iteratively deletes non-entity data definitions without assuming stable collections during destruction.
void Model::_destroyModelDataDefinitions() {
    if (_modeldataManager == nullptr) {
        return;
    }
    bool hasPendingNonEntity = true;
    while (hasPendingNonEntity) {
        hasPendingNonEntity = false;
        // Re-evaluate the current class-name snapshot each pass while deleting non-entity data definitions.
        std::list<std::string> types = _modeldataManager->getDataDefinitionClassnames();
        for (const std::string& type : types) {
            if (type == Util::TypeOf<Entity>()) {
                continue;
            }
            List<ModelDataDefinition*>* datadefs = _modeldataManager->getDataDefinitionList(type);
            if (datadefs != nullptr && !datadefs->empty()) {
                hasPendingNonEntity = true;
                ModelDataDefinition* data = datadefs->front();
                delete data;
                break;
            }
        }
    }
}

void Model::_createInternalDataDefinitions() {
    getTracer()->trace("Automatically creating internal elements", TraceManager::Level::L7_internal);
    Util::IncIndent();

    for (ModelComponent* component : *_componentManager) {
        getTracer()->trace("Internals for " + component->getClassname() + " \"" + component->getName() + "\"");
        Util::IncIndent();
        ModelComponent::CreateInternalData(component);
        Util::DecIndent();
    }

    std::list<ModelDataDefinition*>* modelElements;
    // Cache a value snapshot of class names and refresh it when dynamic insertions change the type registry.
    std::list<std::string> elementTypes = getDataManager()->getDataDefinitionClassnames();
    unsigned int originalSize = elementTypes.size(), pos = 1;
    std::list<std::string>::iterator itty = elementTypes.begin();
    while (itty != elementTypes.end() && pos <= originalSize) {
        modelElements = getDataManager()->getDataDefinitionList((*itty))->list();
        for (std::list<ModelDataDefinition*>::iterator itel = modelElements->begin(); itel != modelElements->end();
             itel++) {
            getTracer()->trace("Internals for " + (*itel)->getClassname() + " \"" + (*itel)->getName() + "\"");
            // (" + std::to_string(pos) + "/" + std::to_string(originalSize) + ")");
            Util::IncIndent();
            ModelDataDefinition::CreateInternalData((*itel));
            Util::DecIndent();
        }
        // Compare against a fresh value snapshot size to detect registry growth during internal-data creation.
        std::list<std::string> currentTypes = getDataManager()->getDataDefinitionClassnames();
        unsigned int currentSize = currentTypes.size();
        if (originalSize == currentSize) {
            itty++;
            pos++;
        }
        else {
            // Refresh the cached snapshot after dynamic insertions so iteration restarts from a coherent list.
            elementTypes = getDataManager()->getDataDefinitionClassnames();
            originalSize = elementTypes.size();
            itty = elementTypes.begin();
            pos = 1;
            getTracer()->trace("Restarting to create internal elements (due to previous creations)",
                               TraceManager::Level::L7_internal);
        }
    }
    Util::DecIndent();
}

void Model::_clearOrphanedDataDefinitions() {
    //bool res = true;
    _traceManager->trace("Checking Orphaned DataDefinitions", TraceManager::Level::L7_internal);
    Util::IncIndent();
    {
        // Track orphan candidates by pointer identity to make pruning deterministic and iteration-safe.
        std::unordered_set<ModelDataDefinition*> orphaned;
        // Start by including all elements as orphaned
        // Use a value snapshot of type names when enumerating initial orphan candidates.
        std::list<std::string> allTypes = _modeldataManager->getDataDefinitionClassnames();
        for (std::string ddtypename : allTypes) {
            for (ModelDataDefinition* element : *_modeldataManager->getDataDefinitionList(ddtypename)->list()) {
                orphaned.insert(element);
            }
        }
        // Remove every referenced data definition from orphan candidates while preserving trace semantics.
        auto removeReferenced = [&](ModelDataDefinition* owner) {
            for (std::pair<std::string, ModelDataDefinition*> pairInternal : *owner->getInternalData()) {
                ModelDataDefinition* mdd = pairInternal.second;
                orphaned.erase(mdd);
                _traceManager->trace(
                    "(" + owner->getClassname() + ") " + owner->getName() + " <#>--> " + "(" + mdd->getClassname() +
                    ") " + mdd->getName());
            }
            for (std::pair<std::string, ModelDataDefinition*> pairAttached : *owner->getAttachedData()) {
                ModelDataDefinition* mdd = pairAttached.second;
                orphaned.erase(mdd);
                _traceManager->trace(
                    "(" + owner->getClassname() + ") " + owner->getName() + " < >--> " + "(" + mdd->getClassname() +
                    ") " + mdd->getName());
            }
        };
        // ... by someone (ModelDataDefinition).
        // Use another value snapshot because orphan pruning may observe changes made during checking.
        std::list<std::string> referencedTypes = _modeldataManager->getDataDefinitionClassnames();
        for (std::string ddtypename : referencedTypes) {
            for (ModelDataDefinition* element : *_modeldataManager->getDataDefinitionList(ddtypename)->list()) {
                removeReferenced(element);
            }
        }
        // ... by someone (ModelComponent).
        for (ModelComponent* component : *_componentManager->getAllComponents()) {
            // Remove all component-owned references from orphan candidates before final deletion pass.
            for (std::pair<std::string, ModelDataDefinition*> pairInternal : *component->getInternalData()) {
                ModelDataDefinition* mdd = pairInternal.second;
                orphaned.erase(mdd);
                _traceManager->trace(
                    "(" + component->getClassname() + ") " + component->getName() + " <#>--> " + "(" + mdd->
                    getClassname() + ") " + mdd->getName());
            }
            for (std::pair<std::string, ModelDataDefinition*> pairAttached : *component->getAttachedData()) {
                ModelDataDefinition* mdd = pairAttached.second;
                orphaned.erase(mdd);
                _traceManager->trace(
                    "(" + component->getClassname() + ") " + component->getName() + " < >--> " + "(" + mdd->
                    getClassname() + ") " + mdd->getName());
            }
        }
        // every one in orphaned list now is really orphaned
        if (orphaned.size() > 0) {
            _traceManager->trace("Orphaned DataDefinitions found and will be removed:",
                                 TraceManager::Level::L7_internal);
            Util::IncIndent();
            {
                for (ModelDataDefinition* orphanElem : orphaned) {
                    _traceManager->trace(
                        "Orphan (" + orphanElem->getClassname() + ") " + orphanElem->getName() + "(id=" +
                        std::to_string(orphanElem->getId()) + ") removed");
                    _modeldataManager->remove(orphanElem);
                }
            }
            Util::DecIndent();
            // inoke again, recursivelly (removing some datadefinitions may create some other orphans)
            Util::IncIndent();
            {
                _clearOrphanedDataDefinitions();
            }
            Util::DecIndent();
        }
        else {
            _traceManager->trace("No orphaned DataDefinitions found", TraceManager::Level::L7_internal);
        }
    }
    Util::DecIndent();
}

List<SimulationControl*>* Model::getControls() const {
    return _controls;
}

List<SimulationResponse*>* Model::getResponses() const {
    return _responses;
}

bool Model::check() {
    getTracer()->trace("Checking model consistency", TraceManager::Level::L7_internal);
    Util::IncIndent();
    // before checking the model, creates all necessary internal ModelDatas and clear orphaned
    _createInternalDataDefinitions();
    _clearOrphanedDataDefinitions();
    bool res = this->_modelChecker->checkAll();
    Util::DecIndent();
    if (res) {
        getTracer()->trace("End of Model checking: Success", TraceManager::Level::L2_results);
    }
    else {
        //std::exception e = new std::exception();
        //getTrace()->traceError() ;
        getTracer()->trace("End of Model checking: Failed", TraceManager::Level::L2_results);
    }
    return res;
}

//bool Model::verifySymbol(std::string componentName, std::string expressionName, std::string expression, std::string expressionResult, bool mandatory) {
//    return this->_modelChecker->verifySymbol(componentName, expressionName, expression, expressionResult, mandatory);
//}

Entity* Model::createEntity(std::string name, bool insertIntoModel) {
    // Entity is my FRIEND, therefore Model can access it
    Entity* newEntity = new Entity(this, name, true);
    auto se = _simulation->_createSimulationEvent(); // it's my friend
    se->setEntityCreated(newEntity);
    //getTracer()->traceSimulation(this, /*"Entity " + entId +*/entity->getName() + " was created");
    getOnEventManager()->NotifyEntityCreateHandlers(se.get());
    return newEntity;
}

void Model::removeEntity(Entity* entity) {
    //, bool collectStatistics) {
    auto se = _simulation->_createSimulationEvent();
    this->_eventManager->NotifyEntityRemoveHandlers(se.get()); // it's my friend
    std::string entId = std::to_string(entity->entityNumber());
    this->getDataManager()->remove(Util::TypeOf<Entity>(), entity);
    getTracer()->traceSimulation(this, /*"Entity " + entId +*/entity->getName() + " was removed from the system");
    delete entity; //->~Entity();
}

List<Event*>* Model::getFutureEvents() const {
    return _futureEvents;
}

void Model::setTracer(TraceManager* _traceManager) {
    this->_traceManager = _traceManager;
}

TraceManager* Model::getTracer() const {
    return _traceManager;
}

ModelPersistence_if* Model::getPersistence() const {
    return _modelPersistence;
}

unsigned int Model::getLevel() const {
    return _level;
}

bool Model::hasChanged() const {
    bool changed = _hasChanged;
    changed = changed || this->_componentManager->hasChanged();
    changed = changed || this->_modeldataManager->hasChanged();
    changed = changed || this->_modelInfo->hasChanged();
    changed = changed || this->_modelPersistence->hasChanged();
    return changed;
}

void Model::setHasChanged(bool hasChanged) {
    _hasChanged = hasChanged;
    _componentManager->setHasChanged(hasChanged);
    _modeldataManager->setHasChanged(hasChanged);
    _modelInfo->setHasChanged(hasChanged);
    _modelPersistence->setHasChanged(hasChanged);

    // Clear or mark every persistent object so aggregate hasChanged() reflects the saved model state.
    std::unordered_set<ModelDataDefinition*> visited;
    for (ModelComponent* component : *_componentManager->getAllComponents()) {
        SetDataDefinitionChanged(component, hasChanged, &visited);
    }
    for (const std::string& classname : _modeldataManager->getDataDefinitionClassnames()) {
        List<ModelDataDefinition*>* dataDefinitions = _modeldataManager->getDataDefinitionList(classname);
        if (dataDefinitions == nullptr) {
            continue;
        }
        for (ModelDataDefinition* dataDefinition : *dataDefinitions->list()) {
            SetDataDefinitionChanged(dataDefinition, hasChanged, &visited);
        }
    }
}

ComponentManager* Model::getComponentManager() const {
    return _componentManager;
}

OnEventManager* Model::getOnEventManager() const {
    return _eventManager;
}

ModelDataManager* Model::getDataManager() const {
    return _modeldataManager;
}

ModelInfo* Model::getInfos() const {
    return _modelInfo;
}

Simulator* Model::getParentSimulator() const {
    return _parentSimulator;
}

ModelSimulation* Model::getSimulation() const {
    return _simulation;
}

Util::identification Model::getId() const {
    return _id;
}
