#include "controllers/SimulationCommandController.h"

#include "controllers/SimulationController.h"
#include "animations/AnimationTransition.h"
#include "../../../../kernel/simulator/model/ModelSimulation.h"
#include <QDebug>

SimulationCommandController::SimulationCommandController(
    SimulationController* simulationController,
    const std::function<void(const std::string&)>& insertCommandInConsole,
    const std::function<void()>& actualizeActions,
    const std::function<bool()>& checkModel,
    const std::function<bool()>& setSimulationModelBasedOnText,
    const std::function<void()>& prepareNewSimulationOutputs,
    const std::function<bool()>& wasModelTabSelected,
    const std::function<void()>& showTracesForNewSimulation,
    const std::function<bool()>& animationsEnabled)
    : _simulationController(simulationController),
      _insertCommandInConsole(insertCommandInConsole),
      _actualizeActions(actualizeActions),
      _checkModel(checkModel),
      _setSimulationModelBasedOnText(setSimulationModelBasedOnText),
      _prepareNewSimulationOutputs(prepareNewSimulationOutputs),
      _wasModelTabSelected(wasModelTabSelected),
      _showTracesForNewSimulation(showTracesForNewSimulation),
      _animationsEnabled(animationsEnabled) {
}

// Move simulation start orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationStartTriggered(bool modelChecked) const {
    const bool selectTraces = _wasModelTabSelected && _wasModelTabSelected();
    if (!_simulationController || !_simulationController->ensureReady(
            true,
            modelChecked,
            _checkModel,
            _setSimulationModelBasedOnText)) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    // Log start command context before dereferencing the simulation pointer.
    qInfo() << "GUI SimulationCommand start simulationNull=" << (simulation == nullptr);
    if (simulation == nullptr) {
        return;
    }
    if (selectTraces && _showTracesForNewSimulation) {
        _showTracesForNewSimulation();
    }
    if (!simulation->isRunning() && !simulation->isPaused() && _prepareNewSimulationOutputs) {
        _prepareNewSimulationOutputs();
    }

    const bool animationsEnabled = !_animationsEnabled || _animationsEnabled();
    qInfo() << "GUI SimulationCommand start setRunning=" << animationsEnabled << "setPause=false";
    AnimationTransition::setRunning(animationsEnabled);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("start");
    simulation->start();
    // A completed synchronous run may select Results through the existing
    // simulation-end handler. Preserve the requested trace view for commands
    // that originated on Model after that handler has completed.
    if (selectTraces && _showTracesForNewSimulation) {
        _showTracesForNewSimulation();
    }
}

// Move simulation step orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationStepTriggered(bool modelChecked) const {
    const bool selectTraces = _wasModelTabSelected && _wasModelTabSelected();
    if (!_simulationController || !_simulationController->ensureReady(
            true,
            modelChecked,
            _checkModel,
            _setSimulationModelBasedOnText)) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    // Log step command context before dereferencing the simulation pointer.
    qInfo() << "GUI SimulationCommand step simulationNull=" << (simulation == nullptr);
    if (simulation == nullptr) {
        return;
    }
    if (selectTraces && _showTracesForNewSimulation) {
        _showTracesForNewSimulation();
    }
    if (!simulation->isRunning() && !simulation->isPaused() && _prepareNewSimulationOutputs) {
        _prepareNewSimulationOutputs();
    }

    const bool animationsEnabled = !_animationsEnabled || _animationsEnabled();
    qInfo() << "GUI SimulationCommand step setRunning=" << animationsEnabled << "setPause=false";
    AnimationTransition::setRunning(animationsEnabled);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("step");
    simulation->step();
    if (selectTraces && _showTracesForNewSimulation) {
        _showTracesForNewSimulation();
    }
}

// Move simulation pause orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationPauseTriggered() const {
    if (!_simulationController || !_simulationController->hasCurrentModelSimulation()) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    // Log pause command context before dereferencing the simulation pointer.
    qInfo() << "GUI SimulationCommand pause simulationNull=" << (simulation == nullptr);
    if (simulation == nullptr) {
        return;
    }

    // Log static animation runtime flags applied by the pause action.
    qInfo() << "GUI SimulationCommand pause setRunning=true setPause=true";
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(true);

    _insertCommandInConsole("pause");
    simulation->pause();
}

// Move simulation resume orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationResumeTriggered(bool modelChecked) const {
    if (!_simulationController || !_simulationController->ensureReady(
            false,
            modelChecked,
            _checkModel,
            _setSimulationModelBasedOnText)) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    // Log resume command context before dereferencing the simulation pointer.
    qInfo() << "GUI SimulationCommand resume simulationNull=" << (simulation == nullptr);
    if (simulation == nullptr) {
        return;
    }

    // Resume must respect a runtime animation disable performed while paused.
    const bool animationsEnabled = !_animationsEnabled || _animationsEnabled();
    qInfo() << "GUI SimulationCommand resume setRunning=" << animationsEnabled << "setPause=false";
    AnimationTransition::setRunning(animationsEnabled);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("resume");
    simulation->start();
}

// Move simulation stop orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationStopTriggered() const {
    if (!_simulationController || !_simulationController->hasCurrentModelSimulation()) {
        return;
    }

    ModelSimulation* simulation = _simulationController->currentSimulation();
    // Log stop command context before dereferencing the simulation pointer.
    qInfo() << "GUI SimulationCommand stop simulationNull=" << (simulation == nullptr);
    if (simulation == nullptr) {
        return;
    }

    // Log static animation runtime flags applied by the stop action.
    qInfo() << "GUI SimulationCommand stop setRunning=false setPause=false";
    AnimationTransition::setRunning(false);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("stop");

    simulation->stop();

    _actualizeActions();
}
