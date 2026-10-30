#include "plugins/components/Continuous/ContinuousModelComponent.h"

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/essentialPlugins/Entity.h"
#include "kernel/simulator/Event.h"

#include <algorithm>
#include <iterator>
#include <utility>

ContinuousModelComponent::ContinuousModelComponent(Model* model, std::string concreteType,
                                                   std::string name)
    : ModelComponent(model, std::move(concreteType), std::move(name)) {}

ContinuousModelComponent::~ContinuousModelComponent() {
    for (auto& [definition, pending] : _pendingExecutions) {
        if (definition != nullptr) definition->removeCompletionHandler(pending.completionId);
        if (definition != nullptr && pending.continuationId != 0)
            definition->removeContinuationHandler(pending.continuationId);
        _removeContinuationEvents(pending.entityIds);
        for (Util::identification id : pending.entityIds) {
            auto* entity = dynamic_cast<Entity*>(_parentModel->getDataManager()->getDataDefinition(
                Util::TypeOf<Entity>(), id));
            if (entity != nullptr) _parentModel->removeEntity(entity);
        }
    }
}

bool ContinuousModelComponent::_requestContinuousExecution(
        ContinuousModelDataDefinition* definition,
        const ContinuousModelDataDefinition::ExecutionRequest& request,
        Entity* entity, std::string& errorMessage) {
    if (definition == nullptr) {
        errorMessage = "Continuous model component has no data definition.";
        return false;
    }
    auto it = _pendingExecutions.find(definition);
    if (it == _pendingExecutions.end()) {
        PendingExecution pending{};
        pending.completionId = definition->addCompletionHandler(
            [this, definition](const ContinuousModelDataDefinition::ExecutionResult& result) {
                _onContinuousExecutionCompleted(definition, result);
            });
        pending.continuationId = definition->addContinuationHandler(
            [this, definition](double absoluteTime) {
                _scheduleContinuousContinuation(definition, absoluteTime);
            });
        it = _pendingExecutions.emplace(definition, std::move(pending)).first;
    }
    const Util::identification entityId = entity != nullptr ? entity->getId() : 0;
    if (entityId != 0) it->second.entityIds.push_back(entityId);

    const bool accepted = definition->requestSimulation(it->second.completionId, request, errorMessage);
    if (!accepted && entityId != 0) {
        auto& ids = it->second.entityIds;
        auto entityIt = std::find(ids.rbegin(), ids.rend(), entityId);
        if (entityIt != ids.rend()) ids.erase(std::next(entityIt).base());
    }
    if (!accepted && it->second.entityIds.empty()) {
        definition->removeCompletionHandler(it->second.completionId);
        _pendingExecutions.erase(it);
    }
    return accepted;
}

void ContinuousModelComponent::_onContinuousExecutionCompleted(
        ContinuousModelDataDefinition* definition,
        const ContinuousModelDataDefinition::ExecutionResult& result) {
    auto it = _pendingExecutions.find(definition);
    if (it == _pendingExecutions.end()) return;
    const auto ids = std::move(it->second.entityIds);
    definition->removeCompletionHandler(it->second.completionId);
    if (it->second.continuationId != 0)
        definition->removeContinuationHandler(it->second.continuationId);
    _removeContinuationEvents(ids);
    _pendingExecutions.erase(it);
    _onContinuousExecutionResult(result);

    _dispatchCompletedEntities(ids, nullptr, result);
}

bool ContinuousModelComponent::_handleContinuousContinuation(Entity* entity,
                                                               unsigned int inputPortNumber) {
    if (inputPortNumber != CONTINUATION_INPUT_PORT || entity == nullptr) return false;
    const Util::identification entityId = entity->getId();
    auto continuation = _continuationsByEntity.find(entityId);
    if (continuation == _continuationsByEntity.end()) {
        traceSimulation(this, TraceManager::Level::L1_errorFatal,
                        "Continuous continuation event has no active execution context.");
        _parentModel->removeEntity(entity);
        return true;
    }
    ContinuousModelDataDefinition* definition = continuation->second;
    _continuationsByEntity.erase(continuation);
    auto pending = _pendingExecutions.find(definition);
    if (pending == _pendingExecutions.end()) {
        _parentModel->removeEntity(entity);
        return true;
    }
    pending->second.continuationEntityIds.erase(entityId);
    if (!definition->resumeExecution() &&
        _pendingExecutions.find(definition) != _pendingExecutions.end()) {
        traceSimulation(this, TraceManager::Level::L1_errorFatal,
                        "Continuous execution could not resume from its entity event.");
    }
    return true;
}

void ContinuousModelComponent::_scheduleContinuousContinuation(
        ContinuousModelDataDefinition* definition, double absoluteTime) {
    auto pending = _pendingExecutions.find(definition);
    if (pending == _pendingExecutions.end() || !pending->second.continuationEntityIds.empty()) return;
    for (Util::identification id : pending->second.entityIds) {
        auto* entity = dynamic_cast<Entity*>(_parentModel->getDataManager()->getDataDefinition(
            Util::TypeOf<Entity>(), id));
        if (entity == nullptr) continue;
        auto* event = new Event(absoluteTime, entity, this, CONTINUATION_INPUT_PORT);
        pending->second.continuationEntityIds.insert(id);
        _continuationsByEntity[id] = definition;
        _parentModel->getFutureEvents()->insert(event);
        return;
    }
    // No entity remains to carry the continuation. Resume through an internal
    // event so that the data definition does not stay suspended forever.
    auto* event = new InternalEvent(absoluteTime, "ContinuousComponentContinuation");
    event->setEventHandler(definition, &ContinuousModelDataDefinition::resumeExecutionFromEvent, nullptr);
    _parentModel->getFutureEvents()->insert(event);
}

void ContinuousModelComponent::_removeContinuationEvents(
        const std::vector<Util::identification>& entityIds) {
    if (_parentModel == nullptr || _parentModel->getFutureEvents() == nullptr) return;
    const std::set<Util::identification> ids(entityIds.begin(), entityIds.end());
    auto* events = _parentModel->getFutureEvents()->list();
    for (auto event = events->begin(); event != events->end();) {
        Event* candidate = *event;
        if (candidate != nullptr && candidate->getComponent() == this &&
            candidate->getComponentinputPortNumber() == CONTINUATION_INPUT_PORT &&
            candidate->getEntity() != nullptr && ids.contains(candidate->getEntity()->getId())) {
            _continuationsByEntity.erase(candidate->getEntity()->getId());
            event = events->erase(event);
            delete candidate;
        } else ++event;
    }
}

void ContinuousModelComponent::_completeEntityImmediately(
        Entity* entity, const ContinuousModelDataDefinition::ExecutionResult& result) {
    _onContinuousExecutionResult(result);
    _dispatchCompletedEntities({}, entity, result);
}

void ContinuousModelComponent::_dispatchCompletedEntities(
        const std::vector<Util::identification>& ids, Entity* immediateEntity,
        const ContinuousModelDataDefinition::ExecutionResult& result) {
    Connection* connection = getConnectionManager()->getFrontConnection();
    std::vector<Entity*> entities;
    if (immediateEntity != nullptr) entities.push_back(immediateEntity);
    for (Util::identification id : ids) {
        auto* entity = dynamic_cast<Entity*>(_parentModel->getDataManager()->getDataDefinition(
            Util::TypeOf<Entity>(), id));
        if (entity == nullptr) continue; // Entity was reclaimed during replication teardown.
        entities.push_back(entity);
    }
    for (Entity* entity : entities) {
        if (!result.succeeded()) {
            _parentModel->removeEntity(entity);
        } else if (connection == nullptr || connection->component == nullptr) {
            traceSimulation(this, "Continuous model completion skipped: invalid front connection");
            _parentModel->removeEntity(entity);
        } else {
            _parentModel->sendEntityToComponent(entity, connection);
        }
    }
}
