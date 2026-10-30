#ifndef CONTINUOUSMODELDATADEFINITION_H
#define CONTINUOUSMODELDATADEFINITION_H

#include "kernel/simulator/model/ModelDataDefinition.h"

#include <cstdint>
#include <any>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Base for time-stepped model data definitions. Local time starts at zero;
// absolute calendar times are anchored to the global clock at run start.
class ContinuousModelDataDefinition : public ModelDataDefinition {
public:
    enum class ExecutionState {
        Idle,
        Running,
        WaitingForSynchronization,
        WaitingForContinuation,
        Completed,
        Failed,
        Cancelled
    };

    struct ExecutionResult {
        ExecutionState state;
        std::string message;

        bool succeeded() const { return state == ExecutionState::Completed; }
    };

    struct ExecutionRequest {
        bool useDefinitionTimeWindow = true;
        double startTime = 0.0;
        double stopTime = 0.0;
        double stepSize = 0.0;
    };

    using CompletionId = std::uint64_t;
    using CompletionHandler = std::function<void(const ExecutionResult&)>;
    using ContinuationId = std::uint64_t;
    using ContinuationHandler = std::function<void(double)>;
    using InternalStateEventHandlerId = std::uint64_t;
    struct InternalStateEvent {
        const ContinuousModelDataDefinition* source = nullptr;
        std::uint64_t step = 0;
        double localTime = 0.0;
        double absoluteTime = 0.0;
        std::any state;
        template<class StateView>
        const StateView* stateAs() const { return std::any_cast<StateView>(&state); }
    };
    using InternalStateEventHandler = std::function<void(const InternalStateEvent&)>;

    ContinuousModelDataDefinition(Model* model, std::string concreteType,
                                  std::string name, double stopTime,
                                  double stepSize, unsigned int stepsToSynchronize);
    ~ContinuousModelDataDefinition() override;

    void setStopTime(double stopTime);
    double getStopTime() const;
    void setStepSize(double stepSize);
    double getStepSize() const;
    void setStep(double step);
    double getStep() const;
    double getCurrentTime() const;
    void setCurrentTime(double currentTime);
    void setStepsToSynchronize(unsigned int steps);
    unsigned int getStepsToSynchronize() const;
    void setLastStatus(std::string status);
    std::string getLastStatus() const;
    void setLastErrorMessage(std::string error);
    std::string getLastErrorMessage() const;
    ExecutionState getExecutionState() const;
    CompletionId addCompletionHandler(CompletionHandler handler);
    void removeCompletionHandler(CompletionId id);
    ContinuationId addContinuationHandler(ContinuationHandler handler);
    void removeContinuationHandler(ContinuationId id);
    bool resumeExecution();
    void resumeExecutionFromEvent(void*);
    InternalStateEventHandlerId addInternalStateEventHandler(
        ModelDataDefinition* observer, InternalStateEventHandler handler, int priority = 0);
    template<class Observer>
    InternalStateEventHandlerId addInternalStateEventHandler(
            Observer* observer,
            void (Observer::*method)(const InternalStateEvent&), int priority = 0) {
        static_assert(std::is_base_of_v<ModelDataDefinition, Observer>);
        if (observer == nullptr || method == nullptr) return 0;
        return addInternalStateEventHandler(observer,
            [observer, method](const InternalStateEvent& event) { (observer->*method)(event); }, priority);
    }
    void removeInternalStateEventHandler(InternalStateEventHandlerId id);
    bool requestSimulation(CompletionId id, const ExecutionRequest& request,
                           std::string& errorMessage);
    void cancelExecution(std::string reason);

    // Compatibility only: no start-time property/state exists in this class.
    // Nonzero starts are rejected rather than silently changing the time origin.
    void setStartTime(double startTime);
    double getStartTime() const;

    bool simulate(std::string& errorMessage);
    bool simulate(double stopTime, double stepSize, std::string& errorMessage);
    bool simulate(double startTime, double stopTime, double stepSize,
                  std::string& errorMessage);
    bool advanceOneStep(std::string& errorMessage);

    bool isAwaitingSynchronization() const;

protected:
    bool _loadInstance(PersistenceRecord* fields) override;
    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    bool _check(std::string& errorMessage) override;
    void _initBetweenReplications() override final;

    // The only mandatory specialization: change mathematical state over [t, t+dt].
    // The subclass must not change _currentTime or enqueue an InternalEvent.
    virtual bool evolveState(double currentTime, double dt,
                             std::string& errorMessage) = 0;
    virtual void initializeContinuousState() {}
    virtual void onContinuousReplicationStart() {}
    virtual void onContinuousEvolutionBatchStarted() {}
    virtual void onContinuousStepCompleted() {}
    virtual void onContinuousRunCompleted() {}
    virtual std::any internalStateView() const { return {}; }
    virtual void confirmContinuousState() {}
    virtual void onInternalStateObserversCompleted() {}

    // For reactive integrations whose target is an absolute kernel-clock time.
    // No InternalEvent is generated by this explicit, synchronous operation.
    bool advanceTo(double targetTime, std::string& errorMessage);

    double _stopTime;
    double _stepSize;
    double _currentTime = 0.0;
    double _confirmedCurrentTime = 0.0;
    std::string _lastStatus = "Idle";
    std::string _lastErrorMessage;

private:
    bool _advanceStep(double targetTime, std::string& errorMessage);
    bool _notifyInternalStateEvent(std::string& errorMessage);
    bool _continueEvolution(std::string& errorMessage);
    bool _scheduleSynchronization(std::string& errorMessage);
    double _nextExternalCalendarEventTime() const;
    bool _hasExternalCalendarEventBefore(double absoluteTime) const;
    bool _isExecutionActive() const;
    void _onSynchronization(void*);
    void _onContinuation(void*);
    void _startAutomaticExecution(void*);
    void _beginAtCurrentKernelTime();
    void _completeRun();
    void _failRun(const std::string& errorMessage);
    void _notifyCompletion(const ExecutionResult& result);
    void _notifyCompletion(CompletionId id, const ExecutionResult& result);
    void _setExecutionState(ExecutionState state, std::string message = {});
    void _removePendingSynchronizationEvents();
    void _registerTemporalControls();

    double _originTime = 0.0;
    bool _originInitialized = false;
    unsigned int _stepsToSynchronize;
    unsigned int _stepsSinceSynchronization = 0;
    double _pendingSynchronizationTime = 0.0;
    bool _pendingCalendarWake = false;
    bool _calendarInterruptedBlock = false;
    ExecutionState _executionState = ExecutionState::Idle;
    double _activeStopTime = 0.0;
    double _activeStepSize = 0.0;
    double _definitionStopTimeAtActiveStart = 0.0;
    double _definitionStepSizeAtActiveStart = 0.0;
    double _activeRequestedStartTime = 0.0;
    bool _activeUsesDefinitionTimeWindow = true;
    unsigned int _activeStepsToSynchronize = 0;
    CompletionId _nextCompletionId = 1;
    std::map<CompletionId, CompletionHandler> _completionHandlers;
    ContinuationId _nextContinuationId = 1;
    std::map<ContinuationId, ContinuationHandler> _continuationHandlers;
    struct InternalStateSubscription {
        InternalStateEventHandlerId id = 0;
        int priority = 0;
        std::weak_ptr<void> lifetime;
        InternalStateEventHandler handler;
    };
    std::map<InternalStateEventHandlerId, InternalStateSubscription> _internalStateHandlers;
    InternalStateEventHandlerId _nextInternalStateHandlerId = 1;
    std::uint64_t _continuousStepNumber = 0;
    double _defaultStopTime;
    double _defaultStepSize;
    unsigned int _defaultStepsToSynchronize;
};

#endif // CONTINUOUSMODELDATADEFINITION_H
