#ifndef CONTINUOUSMODELCOMPONENT_H
#define CONTINUOUSMODELCOMPONENT_H

#include <map>
#include <set>
#include <vector>

#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/data/Continuous/ContinuousModelDataDefinition.h"

// Shared entity-lifecycle bridge for components that request continuous runs.
// Concrete plugins retain their own identity, persistence, and domain settings.
class ContinuousModelComponent : public ModelComponent {
public:
    static constexpr unsigned int CONTINUATION_INPUT_PORT = 0xFFFFFFFFu;
    ContinuousModelComponent(Model* model, std::string concreteType, std::string name = "");
    ~ContinuousModelComponent() override;

protected:
    bool _requestContinuousExecution(ContinuousModelDataDefinition* definition,
                                      const ContinuousModelDataDefinition::ExecutionRequest& request,
                                      Entity* entity, std::string& errorMessage);
    void _completeEntityImmediately(
        Entity* entity, const ContinuousModelDataDefinition::ExecutionResult& result);
    bool _handleContinuousContinuation(Entity* entity, unsigned int inputPortNumber);
    virtual void _onContinuousExecutionResult(
        const ContinuousModelDataDefinition::ExecutionResult& result) = 0;

private:
    struct PendingExecution {
        ContinuousModelDataDefinition::CompletionId completionId;
        ContinuousModelDataDefinition::ContinuationId continuationId = 0;
        std::vector<Util::identification> entityIds;
        std::set<Util::identification> continuationEntityIds;
    };
    void _onContinuousExecutionCompleted(
        ContinuousModelDataDefinition* definition,
        const ContinuousModelDataDefinition::ExecutionResult& result);
    void _dispatchCompletedEntities(const std::vector<Util::identification>& ids,
                                    Entity* immediateEntity,
                                    const ContinuousModelDataDefinition::ExecutionResult& result);
    void _scheduleContinuousContinuation(ContinuousModelDataDefinition* definition,
                                         double absoluteTime);
    void _removeContinuationEvents(const std::vector<Util::identification>& entityIds);

    std::map<ContinuousModelDataDefinition*, PendingExecution> _pendingExecutions;
    std::map<Util::identification, ContinuousModelDataDefinition*> _continuationsByEntity;
};

#endif
