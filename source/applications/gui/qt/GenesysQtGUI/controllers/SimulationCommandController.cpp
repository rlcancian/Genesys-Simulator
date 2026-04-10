#include "controllers/SimulationCommandController.h"

#include "controllers/SimulationController.h"
#include "animations/AnimationTransition.h"
#include "../../../../../kernel/simulator/ModelSimulation.h"
#include <QDebug>

SimulationCommandController::SimulationCommandController(
    SimulationController* simulationController,
    const std::function<void(const std::string&)>& insertCommandInConsole,
    const std::function<void()>& actualizeActions,
    const std::function<bool()>& checkModel,
    const std::function<bool()>& setSimulationModelBasedOnText)
    : _simulationController(simulationController),
      _insertCommandInConsole(insertCommandInConsole),
      _actualizeActions(actualizeActions),
      _checkModel(checkModel),
      _setSimulationModelBasedOnText(setSimulationModelBasedOnText) {
}

// Move simulation start orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationStartTriggered(bool modelChecked) const {
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

    // Log static animation runtime flags applied by the start action.
    qInfo() << "GUI SimulationCommand start setRunning=true setPause=false";
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("start");
    simulation->start();
}

// Move simulation step orchestration into the dedicated Phase 8 controller.
void SimulationCommandController::onActionSimulationStepTriggered(bool modelChecked) const {
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

    // Log static animation runtime flags applied by the step action.
    qInfo() << "GUI SimulationCommand step setRunning=true setPause=false";
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);
    _insertCommandInConsole("step");
    simulation->step();
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

    // Log static animation runtime flags applied by the resume action.
    qInfo() << "GUI SimulationCommand resume setRunning=true setPause=false";
    AnimationTransition::setRunning(true);
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
