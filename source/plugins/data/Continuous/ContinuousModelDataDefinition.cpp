#include "plugins/data/Continuous/ContinuousModelDataDefinition.h"

#include "kernel/simulator/Event.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelSimulation.h"

#include <algorithm>
#include <exception>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

ContinuousModelDataDefinition::ContinuousModelDataDefinition(
        Model* model, std::string concreteType, std::string name,
        double stopTime, double stepSize, unsigned int stepsToSynchronize)
    : ModelDataDefinition(model, std::move(concreteType), std::move(name)),
      _stopTime(stopTime),
      _stepSize(stepSize),
      _stepsToSynchronize(stepsToSynchronize),
      _defaultStopTime(stopTime),
      _defaultStepSize(stepSize),
      _defaultStepsToSynchronize(stepsToSynchronize) {
    _registerTemporalControls();
}

ContinuousModelDataDefinition::~ContinuousModelDataDefinition() {
    cancelExecution("Continuous model definition was destroyed.");
    if (_parentModel == nullptr || _parentModel->getFutureEvents() == nullptr) return;
    auto* events = _parentModel->getFutureEvents()->list();
    for (auto event = events->begin(); event != events->end();) {
        auto* internalEvent = dynamic_cast<InternalEvent*>(*event);
        if (internalEvent != nullptr && internalEvent->object() == this) {
            event = events->erase(event);
            delete internalEvent;
        } else {
            ++event;
        }
    }
}

void ContinuousModelDataDefinition::_registerTemporalControls() {
    auto registerControl = [this](SimulationControl* control) {
        _parentModel->getControls()->insert(control);
        _addSimulationControl(control);
    };
    const std::string type = getClassname();
    const std::string name = getName();
    registerControl(new SimulationControlDouble(
        std::bind(&ContinuousModelDataDefinition::getStopTime, this),
        std::bind(&ContinuousModelDataDefinition::setStopTime, this, std::placeholders::_1),
        type, name, "StopTime", ""));
    registerControl(new SimulationControlDouble(
        std::bind(&ContinuousModelDataDefinition::getStepSize, this),
        std::bind(&ContinuousModelDataDefinition::setStepSize, this, std::placeholders::_1),
        type, name, "StepSize", ""));
    registerControl(new SimulationControlUInt(
        std::bind(&ContinuousModelDataDefinition::getStepsToSynchronize, this),
        std::bind(&ContinuousModelDataDefinition::setStepsToSynchronize, this, std::placeholders::_1),
        type, name, "StepsToSynchronize", ""));
    // CurrentTime is runtime state, not an editable simulation parameter.
    registerControl(new SimulationControlString(
        std::bind(&ContinuousModelDataDefinition::getLastStatus, this),
        std::bind(&ContinuousModelDataDefinition::setLastStatus, this, std::placeholders::_1),
        type, name, "LastStatus", ""));
    registerControl(new SimulationControlString(
        std::bind(&ContinuousModelDataDefinition::getLastErrorMessage, this),
        std::bind(&ContinuousModelDataDefinition::setLastErrorMessage, this, std::placeholders::_1),
        type, name, "LastErrorMessage", ""));
}

void ContinuousModelDataDefinition::setStopTime(double value) {
    if (!_isExecutionActive()) _stopTime = value;
}
double ContinuousModelDataDefinition::getStopTime() const { return _stopTime; }
void ContinuousModelDataDefinition::setStepSize(double value) {
    if (!_isExecutionActive()) _stepSize = value;
}
double ContinuousModelDataDefinition::getStepSize() const { return _stepSize; }
void ContinuousModelDataDefinition::setStep(double value) { setStepSize(value); }
double ContinuousModelDataDefinition::getStep() const { return getStepSize(); }
double ContinuousModelDataDefinition::getCurrentTime() const { return _confirmedCurrentTime; }
void ContinuousModelDataDefinition::setCurrentTime(double value) {
    if (std::isfinite(value) && value >= 0.0 && !_isExecutionActive()) {
        _currentTime = value;
        _confirmedCurrentTime = value;
    }
}
void ContinuousModelDataDefinition::setStepsToSynchronize(unsigned int value) {
    if (!_isExecutionActive()) _stepsToSynchronize = value;
}
unsigned int ContinuousModelDataDefinition::getStepsToSynchronize() const { return _stepsToSynchronize; }
void ContinuousModelDataDefinition::setLastStatus(std::string value) { _lastStatus = std::move(value); }
std::string ContinuousModelDataDefinition::getLastStatus() const { return _lastStatus; }
void ContinuousModelDataDefinition::setLastErrorMessage(std::string value) { _lastErrorMessage = std::move(value); }
std::string ContinuousModelDataDefinition::getLastErrorMessage() const { return _lastErrorMessage; }
ContinuousModelDataDefinition::ExecutionState ContinuousModelDataDefinition::getExecutionState() const {
    return _executionState;
}
ContinuousModelDataDefinition::CompletionId
ContinuousModelDataDefinition::addCompletionHandler(CompletionHandler handler) {
    const CompletionId id = _nextCompletionId++;
    _completionHandlers.emplace(id, std::move(handler));
    return id;
}
void ContinuousModelDataDefinition::removeCompletionHandler(CompletionId id) {
    _completionHandlers.erase(id);
}
ContinuousModelDataDefinition::ContinuationId
ContinuousModelDataDefinition::addContinuationHandler(ContinuationHandler handler) {
    if (!handler) return 0;
    const ContinuationId id = _nextContinuationId++;
    _continuationHandlers.emplace(id, std::move(handler));
    return id;
}
void ContinuousModelDataDefinition::removeContinuationHandler(ContinuationId id) {
    _continuationHandlers.erase(id);
}
bool ContinuousModelDataDefinition::resumeExecution() {
    if (_executionState != ExecutionState::WaitingForContinuation) return false;
    _setExecutionState(ExecutionState::Running);
    std::string errorMessage;
    if (!_continueEvolution(errorMessage)) {
        _failRun(errorMessage);
        return false;
    }
    return true;
}
void ContinuousModelDataDefinition::resumeExecutionFromEvent(void*) {
    (void)resumeExecution();
}
ContinuousModelDataDefinition::InternalStateEventHandlerId
ContinuousModelDataDefinition::addInternalStateEventHandler(
        ModelDataDefinition* observer, InternalStateEventHandler handler, int priority) {
    if (observer == nullptr || !handler) return 0;
    const auto id = _nextInternalStateHandlerId++;
    _internalStateHandlers.emplace(id, InternalStateSubscription{
        id, priority, observer->getLifetimeToken(), std::move(handler)});
    return id;
}
void ContinuousModelDataDefinition::removeInternalStateEventHandler(InternalStateEventHandlerId id) {
    _internalStateHandlers.erase(id);
}
void ContinuousModelDataDefinition::_setExecutionState(ExecutionState state, std::string message) {
    _executionState = state;
    _lastStatus = state == ExecutionState::WaitingForSynchronization ? "WaitingForSynchronization" :
        state == ExecutionState::WaitingForContinuation ? "WaitingForContinuation" :
        state == ExecutionState::Idle ? "Idle" : state == ExecutionState::Running ? "Running" :
        state == ExecutionState::Completed ? "Completed" : state == ExecutionState::Failed ? "Failed" : "Cancelled";
    if (state == ExecutionState::Failed || state == ExecutionState::Cancelled) _lastErrorMessage = std::move(message);
}
void ContinuousModelDataDefinition::_notifyCompletion(const ExecutionResult& result) {
    std::vector<CompletionId> ids;
    ids.reserve(_completionHandlers.size());
    for (const auto& [id, handler] : _completionHandlers) { (void)handler; ids.push_back(id); }
    for (CompletionId id : ids) {
        auto it = _completionHandlers.find(id);
        if (it != _completionHandlers.end() && it->second) {
            CompletionHandler handler = it->second;
            handler(result);
        }
    }
}
void ContinuousModelDataDefinition::_notifyCompletion(CompletionId id, const ExecutionResult& result) {
    auto it = _completionHandlers.find(id);
    if (it != _completionHandlers.end() && it->second) {
        CompletionHandler handler = it->second;
        handler(result);
    }
}
bool ContinuousModelDataDefinition::requestSimulation(CompletionId id, const ExecutionRequest& request,
                                                       std::string& errorMessage) {
    (void)id;
    if (_isExecutionActive()) {
        const double requestedStop = request.useDefinitionTimeWindow
            ? _definitionStopTimeAtActiveStart : request.stopTime;
        const double requestedStep = request.useDefinitionTimeWindow
            ? _definitionStepSizeAtActiveStart : request.stepSize;
        const double activeStop = _activeUsesDefinitionTimeWindow
            ? _definitionStopTimeAtActiveStart : _activeStopTime;
        const double activeStep = _activeUsesDefinitionTimeWindow
            ? _definitionStepSizeAtActiveStart : _activeStepSize;
        const double requestedStart = request.useDefinitionTimeWindow ? 0.0 : request.startTime;
        if (requestedStart != _activeRequestedStartTime || requestedStop != activeStop ||
            requestedStep != activeStep) {
            errorMessage = "A continuous model execution with a different time window is already active.";
            return false;
        }
        return true;
    }
    const double oldStopTime = _stopTime;
    const double oldStepSize = _stepSize;
    if (!request.useDefinitionTimeWindow) {
        if (request.startTime != 0.0 || !std::isfinite(request.stopTime) || request.stopTime < 0.0 ||
            !std::isfinite(request.stepSize) || request.stepSize <= 0.0) {
            errorMessage = "Component time window must have StartTime=0, finite StopTime>=0 and StepSize>0.";
            return false;
        }
        _stopTime = request.stopTime;
        _stepSize = request.stepSize;
    }
    _activeStopTime = _stopTime;
    _activeStepSize = _stepSize;
    _activeStepsToSynchronize = _stepsToSynchronize;
    if (!_check(errorMessage)) {
        _stopTime = oldStopTime;
        _stepSize = oldStepSize;
        _activeStopTime = 0.0;
        _activeStepSize = 0.0;
        _activeStepsToSynchronize = 0u;
        return false;
    }
    _definitionStopTimeAtActiveStart = oldStopTime;
    _definitionStepSizeAtActiveStart = oldStepSize;
    _activeUsesDefinitionTimeWindow = request.useDefinitionTimeWindow;
    _activeRequestedStartTime = request.useDefinitionTimeWindow ? 0.0 : request.startTime;
    _beginAtCurrentKernelTime();
    if (!_continueEvolution(errorMessage)) {
        _failRun(errorMessage);
        return true; // Request was accepted; its asynchronous completion reports failure.
    }
    return true;
}
void ContinuousModelDataDefinition::cancelExecution(std::string reason) {
    if (!_isExecutionActive()) return;
    _removePendingSynchronizationEvents();
    _pendingCalendarWake = false;
    _setExecutionState(ExecutionState::Cancelled, reason);
    _notifyCompletion({ExecutionState::Cancelled, std::move(reason)});
}
void ContinuousModelDataDefinition::setStartTime(double value) {
    if (value != 0.0) {
        throw std::invalid_argument("Continuous model StartTime must be zero");
    }
}
double ContinuousModelDataDefinition::getStartTime() const { return 0.0; }
bool ContinuousModelDataDefinition::isAwaitingSynchronization() const {
    return _executionState == ExecutionState::WaitingForSynchronization;
}
bool ContinuousModelDataDefinition::_isExecutionActive() const {
    return _executionState == ExecutionState::Running ||
           _executionState == ExecutionState::WaitingForSynchronization ||
           _executionState == ExecutionState::WaitingForContinuation;
}

bool ContinuousModelDataDefinition::_check(std::string& errorMessage) {
    bool valid = ModelDataDefinition::_check(errorMessage);
    if (!std::isfinite(_stopTime) || _stopTime < 0.0) {
        errorMessage += "StopTime must be finite and non-negative. ";
        valid = false;
    }
    if (!std::isfinite(_stepSize) || _stepSize <= 0.0) {
        errorMessage += "StepSize must be finite and positive. ";
        valid = false;
    }
    return valid;
}

bool ContinuousModelDataDefinition::_loadInstance(PersistenceRecord* fields) {
    const double legacyStartTime = fields->loadField("startTime", 0.0);
    if (legacyStartTime != 0.0) {
        _lastErrorMessage = "Cannot load continuous model \"" + fields->loadField("name", getName()) +
            "\" with unsupported non-zero legacy StartTime=" + std::to_string(legacyStartTime) + ".";
        traceError(_lastErrorMessage);
        return false;
    }
    if (!ModelDataDefinition::_loadInstance(fields)) return false;
    _stopTime = fields->loadField("stopTime", _defaultStopTime);
    _stepSize = fields->loadField("stepSize", _defaultStepSize);
    _stepsToSynchronize = fields->loadField("stepsToSynchronize", _defaultStepsToSynchronize);
    _currentTime = 0.0;
    _confirmedCurrentTime = 0.0;
    _continuousStepNumber = 0;
    _originInitialized = false;
    _lastStatus = "Idle";
    _lastErrorMessage.clear();
    return true;
}

void ContinuousModelDataDefinition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
    fields->saveField("stopTime", _stopTime, _defaultStopTime, saveDefaultValues);
    fields->saveField("stepSize", _stepSize, _defaultStepSize, saveDefaultValues);
    fields->saveField("stepsToSynchronize", _stepsToSynchronize, _defaultStepsToSynchronize, saveDefaultValues);
}

void ContinuousModelDataDefinition::_beginAtCurrentKernelTime() {
    _originTime = _parentModel->getSimulation()->getSimulatedTime();
    _originInitialized = true;
    _currentTime = 0.0;
    _confirmedCurrentTime = 0.0;
    _continuousStepNumber = 0;
    _stepsSinceSynchronization = 0;
    _pendingCalendarWake = false;
    _activeStopTime = _stopTime;
    _activeStepSize = _stepSize;
    _activeStepsToSynchronize = _stepsToSynchronize;
    _lastStatus = "Running";
    _lastErrorMessage.clear();
    _setExecutionState(ExecutionState::Running);
    initializeContinuousState();
}

bool ContinuousModelDataDefinition::_advanceStep(double targetTime, std::string& errorMessage) {
    const double remainingTime = targetTime - _currentTime;
    const double stepSize = _activeStepSize > 0.0 ? _activeStepSize : _stepSize;
    const double scale = std::max({1.0, std::abs(targetTime), std::abs(_currentTime), std::abs(stepSize)});
    const bool finalStep = remainingTime <= stepSize +
        4.0 * std::numeric_limits<double>::epsilon() * scale;
    const double dt = finalStep ? remainingTime : stepSize;
    const double next = finalStep ? targetTime : _currentTime + dt;
    if (!(dt > 0.0) || !std::isfinite(next) || next <= _currentTime) {
        errorMessage = "Continuous time cannot advance with this StepSize.";
        _lastStatus = "Failed";
        _setExecutionState(ExecutionState::Failed, errorMessage);
        return false;
    }
    if (!evolveState(_currentTime, dt, errorMessage)) {
        _lastErrorMessage = errorMessage.empty() ? "Continuous model step failed." : errorMessage;
        errorMessage = _lastErrorMessage;
        _lastStatus = "Failed";
        _setExecutionState(ExecutionState::Failed, errorMessage);
        return false;
    }
    _currentTime = next;
    ++_stepsSinceSynchronization;
    ++_continuousStepNumber;
    onContinuousStepCompleted();
    if (!_notifyInternalStateEvent(errorMessage)) return false;
    return true;
}

bool ContinuousModelDataDefinition::_notifyInternalStateEvent(std::string& errorMessage) {
    std::vector<InternalStateEventHandlerId> ids;
    ids.reserve(_internalStateHandlers.size());
    for (const auto& [id, subscription] : _internalStateHandlers) {
        (void)subscription;
        ids.push_back(id);
    }
    std::stable_sort(ids.begin(), ids.end(), [this](auto left, auto right) {
        return _internalStateHandlers.at(left).priority < _internalStateHandlers.at(right).priority;
    });
    const InternalStateEvent event{this, _continuousStepNumber, _currentTime,
        _originTime + _currentTime, internalStateView()};
    std::string firstException;
    for (InternalStateEventHandlerId id : ids) {
        auto it = _internalStateHandlers.find(id);
        if (it == _internalStateHandlers.end()) continue;
        if (it->second.lifetime.expired()) {
            _internalStateHandlers.erase(it);
            continue;
        }
        try {
            InternalStateEventHandler handler = it->second.handler;
            if (handler) handler(event);
        } catch (const std::exception& e) {
            if (firstException.empty()) firstException = e.what();
        } catch (...) {
            if (firstException.empty()) firstException = "unknown exception";
        }
    }
    onInternalStateObserversCompleted();
    if (!firstException.empty()) {
        errorMessage = "Continuous state observer failed: " + firstException;
        _lastErrorMessage = errorMessage;
        return false;
    }
    return true;
}

void ContinuousModelDataDefinition::_completeRun() {
    _confirmedCurrentTime = _currentTime;
    confirmContinuousState();
    _lastStatus = "Completed";
    onContinuousRunCompleted();
    if (_lastStatus == "Failed") {
        _setExecutionState(ExecutionState::Failed, _lastErrorMessage);
    } else {
        _setExecutionState(ExecutionState::Completed);
    }
    _notifyCompletion({_executionState, _lastErrorMessage});
}

void ContinuousModelDataDefinition::_failRun(const std::string& errorMessage) {
    _removePendingSynchronizationEvents();
    _pendingCalendarWake = false;
    const std::string message = errorMessage.empty() ? "Continuous model execution failed." : errorMessage;
    _setExecutionState(ExecutionState::Failed, message);
    _notifyCompletion({_executionState, message});
}

void ContinuousModelDataDefinition::_removePendingSynchronizationEvents() {
    if (_parentModel == nullptr || _parentModel->getFutureEvents() == nullptr) return;
    auto* events = _parentModel->getFutureEvents()->list();
    for (auto event = events->begin(); event != events->end();) {
        auto* internalEvent = dynamic_cast<InternalEvent*>(*event);
        if (internalEvent != nullptr && internalEvent->object() == this) {
            event = events->erase(event);
            delete internalEvent;
        } else ++event;
    }
}

bool ContinuousModelDataDefinition::_scheduleSynchronization(std::string& errorMessage) {
    const double stopTime = _activeStopTime;
    const double stepSize = _activeStepSize;
    const unsigned int syncSteps = _activeStepsToSynchronize;
    const unsigned int remainingSteps = _currentTime < stopTime &&
        syncSteps > _stepsSinceSynchronization
        ? syncSteps - _stepsSinceSynchronization
        : 0u;
    unsigned int stepsUntilSynchronization = remainingSteps;
    _pendingSynchronizationTime = _currentTime;
    if (stepsUntilSynchronization > 0u) {
        const double stepsUntilStop = std::ceil((stopTime - _currentTime) / stepSize);
        if (stepsUntilStop <= 0.0) {
            stepsUntilSynchronization = 0u;
            _pendingSynchronizationTime = stopTime;
        } else {
            if (std::isfinite(stepsUntilStop)) {
                stepsUntilSynchronization = std::min(
                    stepsUntilSynchronization, static_cast<unsigned int>(std::min(
                        stepsUntilStop, static_cast<double>(std::numeric_limits<unsigned int>::max()))));
            }
            _pendingSynchronizationTime = std::min(
                stopTime, std::fma(stepSize, static_cast<double>(stepsUntilSynchronization), _currentTime));
        }
    }
    _pendingCalendarWake = false;
    _calendarInterruptedBlock = false;
    double eventTime = _originTime + _pendingSynchronizationTime;
    const double globalTime = _parentModel->getSimulation()->getSimulatedTime();
    const double nextCalendarTime = _nextExternalCalendarEventTime();
    // Do not calculate a step whose endpoint would lie after a pending discrete
    // event. If the event falls between grid points, wake at the next grid point;
    // the event itself is processed first by the chronological kernel calendar.
    if (std::isfinite(nextCalendarTime) && nextCalendarTime > globalTime &&
        nextCalendarTime < eventTime) {
        _calendarInterruptedBlock = true;
        const double eventLocalTime = nextCalendarTime - _originTime;
        const double available = eventLocalTime - _currentTime;
        const double tolerance = 8.0 * std::numeric_limits<double>::epsilon() *
            std::max({1.0, std::abs(eventLocalTime), std::abs(_currentTime), std::abs(stepSize)});
        const double wholeSteps = std::floor(std::max(0.0, available + tolerance) / stepSize);
        if (wholeSteps < 1.0) {
            _pendingCalendarWake = true;
            _pendingSynchronizationTime = std::min(stopTime, _currentTime + stepSize);
        } else {
            const auto safeSteps = static_cast<unsigned int>(std::min(
                wholeSteps, static_cast<double>(std::numeric_limits<unsigned int>::max())));
            _pendingSynchronizationTime = std::min(
                _pendingSynchronizationTime,
                std::min(stopTime, std::fma(stepSize, static_cast<double>(safeSteps), _currentTime)));
        }
        eventTime = _originTime + _pendingSynchronizationTime;
    }
    if (!std::isfinite(eventTime) || eventTime < _parentModel->getSimulation()->getSimulatedTime()) {
        errorMessage = "Continuous synchronization time is invalid or in the past.";
        _lastErrorMessage = errorMessage;
        _lastStatus = "Failed";
        _setExecutionState(ExecutionState::Failed, errorMessage);
        return false;
    }
    auto* event = new InternalEvent(eventTime, "ContinuousModelSynchronization");
    event->setEventHandler(this, &ContinuousModelDataDefinition::_onSynchronization, nullptr);
    _parentModel->getFutureEvents()->insert(event);
    _setExecutionState(ExecutionState::Running);
    return true;
}

double ContinuousModelDataDefinition::_nextExternalCalendarEventTime() const {
    if (_parentModel == nullptr || _parentModel->getFutureEvents() == nullptr ||
        _parentModel->getFutureEvents()->empty()) {
        return std::numeric_limits<double>::infinity();
    }
    const auto* events = _parentModel->getFutureEvents()->list();
    const double now = _parentModel->getSimulation()->getSimulatedTime();
    for (const Event* event : *events) {
        if (event == nullptr) continue;
        const auto* ownSync = dynamic_cast<const InternalEvent*>(event);
        if (ownSync != nullptr && ownSync->object() == this) continue;
        if (event->getTime() > now) return event->getTime();
    }
    return std::numeric_limits<double>::infinity();
}

bool ContinuousModelDataDefinition::_hasExternalCalendarEventBefore(double absoluteTime) const {
    if (_parentModel == nullptr || _parentModel->getFutureEvents() == nullptr) return false;
    const double tolerance = 8.0 * std::numeric_limits<double>::epsilon() *
        std::max(1.0, std::abs(absoluteTime));
    for (const Event* event : *_parentModel->getFutureEvents()->list()) {
        if (event == nullptr) continue;
        const auto* ownSync = dynamic_cast<const InternalEvent*>(event);
        if (ownSync != nullptr && ownSync->object() == this) continue;
        if (event->getTime() < absoluteTime - tolerance) return true;
    }
    return false;
}

bool ContinuousModelDataDefinition::_continueEvolution(std::string& errorMessage) {
    if (isAwaitingSynchronization()) {
        errorMessage = "Continuous evolution is awaiting a scheduled synchronization.";
        return false;
    }
    if (_executionState == ExecutionState::Running && _currentTime < _activeStopTime && _activeStepsToSynchronize > 0u) {
        if (!_scheduleSynchronization(errorMessage)) return false;
        while (!_pendingCalendarWake && _executionState == ExecutionState::Running &&
               _currentTime < _pendingSynchronizationTime) {
            if (!_advanceStep(_pendingSynchronizationTime, errorMessage)) return false;
            const double eventTime = _nextExternalCalendarEventTime();
            const double currentAbsoluteTime = _originTime + _currentTime;
            const double tolerance = 8.0 * std::numeric_limits<double>::epsilon() *
                std::max({1.0, std::abs(eventTime), std::abs(currentAbsoluteTime)});
            if (_hasExternalCalendarEventBefore(currentAbsoluteTime)) {
                errorMessage = "A calendar event was scheduled before the already-evolved continuous state.";
                return false;
            }
            if (std::isfinite(eventTime) && eventTime < _originTime + _pendingSynchronizationTime &&
                eventTime <= currentAbsoluteTime + tolerance) {
                _removePendingSynchronizationEvents();
                _pendingCalendarWake = false;
                _calendarInterruptedBlock = true;
                _pendingSynchronizationTime = _currentTime;
                const double syncTime = _originTime + _currentTime;
                if (!std::isfinite(syncTime) || syncTime <
                    _parentModel->getSimulation()->getSimulatedTime()) {
                    errorMessage = "Observer-generated synchronization time is invalid or in the past.";
                    return false;
                }
                auto* syncEvent = new InternalEvent(syncTime, "ContinuousModelSynchronization");
                syncEvent->setEventHandler(this, &ContinuousModelDataDefinition::_onSynchronization, nullptr);
                _parentModel->getFutureEvents()->insert(syncEvent);
                break;
            } else if (std::isfinite(eventTime) &&
                       eventTime < _originTime + _pendingSynchronizationTime) {
                // A handler may have inserted an event after this step but
                // before the previously planned synchronization. Recompute the
                // safe horizon now, before attempting the next mathematical step.
                _removePendingSynchronizationEvents();
                if (!_scheduleSynchronization(errorMessage)) return false;
                if (_pendingCalendarWake) break;
            }
        }
        if (_executionState == ExecutionState::Running) _setExecutionState(ExecutionState::WaitingForSynchronization);
        return true;
    }
    while (_executionState == ExecutionState::Running && _currentTime < _activeStopTime) {
        if (!_advanceStep(_activeStopTime, errorMessage)) return false;
    }
    if (_executionState == ExecutionState::Running) _completeRun();
    return true;
}

void ContinuousModelDataDefinition::_onSynchronization(void*) {
    if (!isAwaitingSynchronization()) return;
    const bool interruptedByCalendar = _calendarInterruptedBlock;
    _calendarInterruptedBlock = false;
    _confirmedCurrentTime = _currentTime;
    confirmContinuousState();
    _setExecutionState(ExecutionState::Running);
    _stepsSinceSynchronization = 0u;
    _pendingCalendarWake = false;
    if (_currentTime >= _activeStopTime) {
        _completeRun();
        return;
    }
    if (interruptedByCalendar) {
        const double continuationTime = _originTime + std::min(_activeStopTime,
                                                               _currentTime + _activeStepSize);
        std::vector<ContinuationId> ids;
        ids.reserve(_continuationHandlers.size());
        for (const auto& [id, handler] : _continuationHandlers) {
            (void)handler;
            ids.push_back(id);
        }
        if (ids.empty()) {
            auto* continuation = new InternalEvent(continuationTime, "ContinuousModelContinuation");
            continuation->setEventHandler(this, &ContinuousModelDataDefinition::_onContinuation, nullptr);
            _parentModel->getFutureEvents()->insert(continuation);
        } else {
            _setExecutionState(ExecutionState::WaitingForContinuation);
            try {
                for (ContinuationId id : ids) {
                    auto it = _continuationHandlers.find(id);
                    if (it == _continuationHandlers.end() || !it->second) continue;
                    ContinuationHandler handler = it->second;
                    handler(continuationTime);
                    // One entity event resumes the shared mathematical run.
                    // Other components joined to this definition remain retained
                    // and receive the same eventual completion notification.
                    break;
                }
            } catch (const std::exception& e) {
                _failRun(std::string("Continuous continuation handler failed: ") + e.what());
            } catch (...) {
                _failRun("Continuous continuation handler failed with an unknown exception.");
            }
            return;
        }
        _setExecutionState(ExecutionState::WaitingForContinuation);
        return;
    }
    std::string errorMessage;
    if (!_continueEvolution(errorMessage)) {
        traceError("Continuous model synchronization failed: " + errorMessage);
        _failRun(errorMessage);
    }
}

void ContinuousModelDataDefinition::_onContinuation(void*) {
    (void)resumeExecution();
}

void ContinuousModelDataDefinition::_initBetweenReplications() {
    ModelDataDefinition::_initBetweenReplications();
    cancelExecution("Continuous execution cancelled at replication boundary.");
    onContinuousReplicationStart();
    _stepsSinceSynchronization = 0;
    _currentTime = 0.0;
    _confirmedCurrentTime = 0.0;
    _continuousStepNumber = 0;
    _originTime = _parentModel->getSimulation()->getSimulatedTime();
    _originInitialized = true;
    _lastStatus = "Idle";
    _executionState = ExecutionState::Idle;
    _lastErrorMessage.clear();
    if (_stepsToSynchronize > 0) {
        _activeStopTime = _stopTime;
        _activeStepSize = _stepSize;
        _activeStepsToSynchronize = _stepsToSynchronize;
        _definitionStopTimeAtActiveStart = _stopTime;
        _definitionStepSizeAtActiveStart = _stepSize;
        _activeUsesDefinitionTimeWindow = true;
        _activeRequestedStartTime = 0.0;
        // Defer autonomous evolution until the kernel dispatches this zero-time
        // event. Replication-start handlers have then populated the calendar, so
        // the first lookahead can respect their discrete events.
        auto* startEvent = new InternalEvent(_originTime, "ContinuousModelAutomaticStart");
        startEvent->setEventHandler(this, &ContinuousModelDataDefinition::_startAutomaticExecution, nullptr);
        _parentModel->getFutureEvents()->insert(startEvent);
    } else {
        initializeContinuousState();
    }
}

void ContinuousModelDataDefinition::_startAutomaticExecution(void*) {
    if (_isExecutionActive() || _executionState != ExecutionState::Idle) return;
    std::string errorMessage;
    if (!_check(errorMessage)) {
        _failRun(errorMessage);
        traceError(errorMessage);
        return;
    }
    _beginAtCurrentKernelTime();
    if (!_continueEvolution(errorMessage)) {
        _failRun(errorMessage);
        traceError(errorMessage);
    }
}

bool ContinuousModelDataDefinition::simulate(std::string& errorMessage) {
    if (_isExecutionActive()) {
        errorMessage = "Continuous model evolution is already active.";
        return false;
    }
    if (!_check(errorMessage)) return false;
    _activeStopTime = _stopTime;
    _activeStepSize = _stepSize;
    _definitionStopTimeAtActiveStart = _stopTime;
    _definitionStepSizeAtActiveStart = _stepSize;
    _activeUsesDefinitionTimeWindow = true;
    _activeRequestedStartTime = 0.0;
    _activeStepsToSynchronize = _stepsToSynchronize;
    _beginAtCurrentKernelTime();
    if (!_continueEvolution(errorMessage)) {
        _failRun(errorMessage);
        return false;
    }
    return true;
}

bool ContinuousModelDataDefinition::simulate(double stopTime, double stepSize,
                                             std::string& errorMessage) {
    if (_isExecutionActive()) {
        errorMessage = "Continuous model evolution is already active.";
        return false;
    }
    setStopTime(stopTime);
    setStepSize(stepSize);
    return simulate(errorMessage);
}

bool ContinuousModelDataDefinition::simulate(double startTime, double stopTime, double stepSize,
                                             std::string& errorMessage) {
    if (startTime != 0.0) {
        errorMessage = "Continuous model StartTime must be zero.";
        return false;
    }
    return simulate(stopTime, stepSize, errorMessage);
}

bool ContinuousModelDataDefinition::advanceOneStep(std::string& errorMessage) {
    if (isAwaitingSynchronization()) {
        errorMessage = "Continuous model is awaiting synchronization.";
        return false;
    }
    if (_executionState != ExecutionState::Running) {
        if (!_check(errorMessage)) return false;
        _beginAtCurrentKernelTime();
    }
    if (_currentTime >= _activeStopTime) {
        _completeRun();
        return true;
    }
    if (!_advanceStep(_activeStopTime, errorMessage)) {
        _failRun(errorMessage);
        return false;
    }
    if (_activeStepsToSynchronize > 0 &&
        (_stepsSinceSynchronization >= _activeStepsToSynchronize || _currentTime >= _activeStopTime)) {
        return _continueEvolution(errorMessage);
    }
    if (_currentTime >= _activeStopTime) _completeRun();
    return true;
}

bool ContinuousModelDataDefinition::advanceTo(double targetTime, std::string& errorMessage) {
    if (!std::isfinite(targetTime)) {
        errorMessage = "Invalid continuous integration target.";
        return false;
    }
    if (!_originInitialized) {
        _originTime = _parentModel->getSimulation()->getSimulatedTime();
        _originInitialized = true;
    }
    const double localTargetTime = targetTime - _originTime;
    if (!std::isfinite(localTargetTime) || localTargetTime < 0.0) {
        errorMessage = "Continuous integration target precedes its global time origin.";
        return false;
    }
    if (isAwaitingSynchronization()) {
        if (localTargetTime > _pendingSynchronizationTime) {
            errorMessage = "Continuous integration target exceeds the pending synchronization time.";
            return false;
        }
        // A block has already been mathematically evaluated ahead of the kernel.
        // Reactive callers at an intermediate calendar time observe the last
        // confirmed state; they must not re-integrate or expose the work state.
        if (localTargetTime <= _currentTime && localTargetTime >= _confirmedCurrentTime) return true;
    }
    if (!_check(errorMessage) || localTargetTime < _currentTime) {
        if (errorMessage.empty()) errorMessage = "Invalid continuous integration target.";
        return false;
    }
    // Used by legacy reactive component invocations; no calendar event is posted.
    if (_executionState != ExecutionState::Running) {
        _activeStopTime = _stopTime;
        _activeStepSize = _stepSize;
        _activeStepsToSynchronize = _stepsToSynchronize;
        _lastStatus = "Running";
        _setExecutionState(ExecutionState::Running);
    }
    const double boundedLocalTargetTime = std::min(localTargetTime, _activeStopTime);
    while (_currentTime < boundedLocalTargetTime) {
        if (!_advanceStep(boundedLocalTargetTime, errorMessage)) {
            _failRun(errorMessage);
            return false;
        }
    }
    if (!isAwaitingSynchronization()) {
        _confirmedCurrentTime = _currentTime;
        confirmContinuousState();
        if (_currentTime >= _activeStopTime) _completeRun();
        else _setExecutionState(ExecutionState::Idle);
    }
    return true;
}
