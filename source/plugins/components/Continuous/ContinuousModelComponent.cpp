#include "plugins/components/Continuous/ContinuousModelComponent.h"

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/essentialPlugins/Entity.h"

#include <algorithm>
#include <iterator>
#include <utility>

ContinuousModelComponent::ContinuousModelComponent(Model* model, std::string concreteType,
                                                   std::string name)
    : ModelComponent(model, std::move(concreteType), std::move(name)) {}

ContinuousModelComponent::~ContinuousModelComponent() {
    for (auto& [definition, pending] : _pendingExecutions) {
        if (definition != nullptr) definition->removeCompletionHandler(pending.completionId);
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
    _pendingExecutions.erase(it);
    _onContinuousExecutionResult(result);

    _dispatchCompletedEntities(ids, nullptr, result);
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
