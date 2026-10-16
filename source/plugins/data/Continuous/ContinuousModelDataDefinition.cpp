#include "plugins/data/Continuous/ContinuousModelDataDefinition.h"

#include "kernel/simulator/Event.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelSimulation.h"

#include <algorithm>
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

void ContinuousModelDataDefinition::setStopTime(double value) { _stopTime = value; }
double ContinuousModelDataDefinition::getStopTime() const { return _stopTime; }
void ContinuousModelDataDefinition::setStepSize(double value) { _stepSize = value; }
double ContinuousModelDataDefinition::getStepSize() const { return _stepSize; }
void ContinuousModelDataDefinition::setStep(double value) { setStepSize(value); }
double ContinuousModelDataDefinition::getStep() const { return getStepSize(); }
double ContinuousModelDataDefinition::getCurrentTime() const { return _currentTime; }
void ContinuousModelDataDefinition::setCurrentTime(double value) { _currentTime = value; }
void ContinuousModelDataDefinition::setStepsToSynchronize(unsigned int value) { _stepsToSynchronize = value; }
unsigned int ContinuousModelDataDefinition::getStepsToSynchronize() const { return _stepsToSynchronize; }
void ContinuousModelDataDefinition::setLastStatus(std::string value) { _lastStatus = std::move(value); }
std::string ContinuousModelDataDefinition::getLastStatus() const { return _lastStatus; }
void ContinuousModelDataDefinition::setLastErrorMessage(std::string value) { _lastErrorMessage = std::move(value); }
std::string ContinuousModelDataDefinition::getLastErrorMessage() const { return _lastErrorMessage; }
void ContinuousModelDataDefinition::setStartTime(double value) {
    if (value != 0.0) {
        throw std::invalid_argument("Continuous model StartTime must be zero");
    }
}
double ContinuousModelDataDefinition::getStartTime() const { return 0.0; }
bool ContinuousModelDataDefinition::isAwaitingSynchronization() const { return _awaitingSynchronization; }

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
    _stepsSinceSynchronization = 0;
    _awaitingSynchronization = false;
    _running = true;
    _lastStatus = "Running";
    _lastErrorMessage.clear();
    initializeContinuousState();
}

bool ContinuousModelDataDefinition::_advanceStep(double targetTime, std::string& errorMessage) {
    const double remainingTime = targetTime - _currentTime;
    const double scale = std::max({1.0, std::abs(targetTime), std::abs(_currentTime), std::abs(_stepSize)});
    const bool finalStep = remainingTime <= _stepSize +
        4.0 * std::numeric_limits<double>::epsilon() * scale;
    const double dt = finalStep ? remainingTime : _stepSize;
    const double next = finalStep ? targetTime : _currentTime + dt;
    if (!(dt > 0.0) || !std::isfinite(next) || next <= _currentTime) {
        errorMessage = "Continuous time cannot advance with this StepSize.";
        _lastErrorMessage = errorMessage;
        _lastStatus = "Failed";
        _running = false;
        return false;
    }
    if (!evolveState(_currentTime, dt, errorMessage)) {
        _lastErrorMessage = errorMessage.empty() ? "Continuous model step failed." : errorMessage;
        errorMessage = _lastErrorMessage;
        _lastStatus = "Failed";
        _running = false;
        return false;
    }
    _currentTime = next;
    ++_stepsSinceSynchronization;
    onContinuousStepCompleted();
    return true;
}

void ContinuousModelDataDefinition::_completeRun() {
    _running = false;
    _awaitingSynchronization = false;
    _lastStatus = "Completed";
    onContinuousRunCompleted();
}

bool ContinuousModelDataDefinition::_scheduleSynchronization(std::string& errorMessage) {
    const unsigned int remainingSteps = _currentTime < _stopTime &&
        _stepsToSynchronize > _stepsSinceSynchronization
        ? _stepsToSynchronize - _stepsSinceSynchronization
        : 0u;
    unsigned int stepsUntilSynchronization = remainingSteps;
    _pendingSynchronizationTime = _currentTime;
    if (stepsUntilSynchronization > 0u) {
        const double stepsUntilStop = std::ceil((_stopTime - _currentTime) / _stepSize);
        if (stepsUntilStop <= 0.0) {
            stepsUntilSynchronization = 0u;
            _pendingSynchronizationTime = _stopTime;
        } else {
            if (std::isfinite(stepsUntilStop)) {
                stepsUntilSynchronization = std::min(
                    stepsUntilSynchronization, static_cast<unsigned int>(std::min(
                        stepsUntilStop, static_cast<double>(std::numeric_limits<unsigned int>::max()))));
            }
            _pendingSynchronizationTime = std::min(
                _stopTime, std::fma(_stepSize, static_cast<double>(stepsUntilSynchronization), _currentTime));
        }
    }
    const double eventTime = _originTime + _pendingSynchronizationTime;
    if (!std::isfinite(eventTime) || eventTime < _parentModel->getSimulation()->getSimulatedTime()) {
        errorMessage = "Continuous synchronization time is invalid or in the past.";
        _lastErrorMessage = errorMessage;
        _lastStatus = "Failed";
        _running = false;
        return false;
    }
    auto* event = new InternalEvent(eventTime, "ContinuousModelSynchronization");
    event->setEventHandler(this, &ContinuousModelDataDefinition::_onSynchronization, nullptr);
    _parentModel->getFutureEvents()->insert(event);
    _awaitingSynchronization = true;
    return true;
}

bool ContinuousModelDataDefinition::_continueEvolution(std::string& errorMessage) {
    if (_awaitingSynchronization) {
        errorMessage = "Continuous evolution is awaiting a scheduled synchronization.";
        return false;
    }
    if (_running && _currentTime < _stopTime && _stepsToSynchronize > 0u) {
        return _scheduleSynchronization(errorMessage);
    }
    while (_running && _currentTime < _stopTime) {
        if (!_advanceStep(_stopTime, errorMessage)) return false;
    }
    if (_running) _completeRun();
    return true;
}

void ContinuousModelDataDefinition::_onSynchronization(void*) {
    if (!_awaitingSynchronization || !_running) return;
    _awaitingSynchronization = false;
    while (_currentTime < _pendingSynchronizationTime) {
        std::string errorMessage;
        if (!_advanceStep(_pendingSynchronizationTime, errorMessage)) {
            traceError("Continuous model synchronization failed: " + errorMessage);
            return;
        }
    }
    _stepsSinceSynchronization = 0u;
    if (_currentTime >= _stopTime) {
        _completeRun();
        return;
    }
    std::string errorMessage;
    if (!_continueEvolution(errorMessage)) {
        traceError("Continuous model synchronization failed: " + errorMessage);
    }
}

void ContinuousModelDataDefinition::_initBetweenReplications() {
    ModelDataDefinition::_initBetweenReplications();
    _running = false;
    _awaitingSynchronization = false;
    _stepsSinceSynchronization = 0;
    _currentTime = 0.0;
    _originTime = _parentModel->getSimulation()->getSimulatedTime();
    _originInitialized = true;
    _lastStatus = "Idle";
    _lastErrorMessage.clear();
    if (_stepsToSynchronize > 0) {
        _beginAtCurrentKernelTime();
        std::string errorMessage;
        if (!_check(errorMessage) || !_continueEvolution(errorMessage)) {
            _running = false;
            _lastStatus = "Failed";
            _lastErrorMessage = errorMessage;
            traceError(errorMessage);
        }
    } else {
        initializeContinuousState();
    }
}

bool ContinuousModelDataDefinition::simulate(std::string& errorMessage) {
    if (_running || _awaitingSynchronization) {
        errorMessage = "Continuous model evolution is already active.";
        return false;
    }
    if (!_check(errorMessage)) return false;
    _beginAtCurrentKernelTime();
    return _continueEvolution(errorMessage);
}

bool ContinuousModelDataDefinition::simulate(double stopTime, double stepSize,
                                             std::string& errorMessage) {
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
    if (_awaitingSynchronization) {
        errorMessage = "Continuous model is awaiting synchronization.";
        return false;
    }
    if (!_running) {
        if (!_check(errorMessage)) return false;
        _beginAtCurrentKernelTime();
    }
    if (_currentTime >= _stopTime) {
        _completeRun();
        return true;
    }
    if (!_advanceStep(_stopTime, errorMessage)) return false;
    if (_stepsToSynchronize > 0 &&
        (_stepsSinceSynchronization >= _stepsToSynchronize || _currentTime >= _stopTime)) {
        _pendingSynchronizationTime = _currentTime;
        return _scheduleSynchronization(errorMessage);
    }
    if (_currentTime >= _stopTime) _completeRun();
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
    if (_awaitingSynchronization && localTargetTime > _pendingSynchronizationTime) {
        errorMessage = "Continuous integration target exceeds the pending synchronization time.";
        return false;
    }
    if (!_check(errorMessage) || localTargetTime < _currentTime) {
        if (errorMessage.empty()) errorMessage = "Invalid continuous integration target.";
        return false;
    }
    // Used by legacy reactive component invocations; no calendar event is posted.
    if (!_running) {
        _running = true;
        _lastStatus = "Running";
    }
    const double boundedLocalTargetTime = std::min(localTargetTime, _stopTime);
    while (_currentTime < boundedLocalTargetTime) {
        if (!_advanceStep(boundedLocalTargetTime, errorMessage)) return false;
    }
    if (!_awaitingSynchronization) {
        if (_currentTime >= _stopTime) _completeRun();
        else _running = false;
    }
    return true;
}
