#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include <functional>

class QWidget;
class Simulator;
class ModelSimulation;

// Document the simulation precondition controller role in the refactored GUI architecture.
/**
 * @brief Controller that validates simulation command readiness for MainWindow wrappers.
 *
 * This controller is part of the incremental extraction that keeps MainWindow as a
 * compatibility façade while moving simulation-command orchestration behind smaller
 * collaborators. It centralizes precondition checks shared by start/step/pause/resume/stop
 * flows, including model availability and model-text synchronization guards.
 *
 * Responsibilities:
 * - verify that the current model and its ModelSimulation are available;
 * - execute callback-based readiness checks delegated by MainWindow;
 * - return a single command-ready decision used by higher-level command controllers.
 *
 * Boundaries:
 * - it does not own simulation state or lifecycle (owned by Simulator);
 * - it does not execute GUI commands directly (handled by SimulationCommandController);
 * - it does not update widgets, tabs, or trace outputs.
 *
 * Dependencies are injected narrowly (owner widget + simulator + callbacks) to preserve
 * existing ownership and enable incremental compatibility without changing slot signatures.
 */
class SimulationController {
public:
    /**
     * @brief Creates a controller bound to GUI owner and simulator.
     * @param ownerWidget Widget used to show warning dialogs.
     * @param simulator Simulator facade used by GUI.
     */
    SimulationController(QWidget* ownerWidget, Simulator* simulator);

    /**
     * @brief Checks whether there is a valid current model with simulation object.
     * @return true when simulation commands can access `current()->getSimulation()` safely.
     */
    bool hasCurrentModelSimulation() const;

    /**
     * @brief Returns current simulation object when available.
     * @return Pointer to current ModelSimulation or nullptr when unavailable.
     */
    ModelSimulation* currentSimulation() const;

    /**
     * @brief Validates whether GUI can execute a simulation command now.
     *
     * Validation pipeline:
     * 1) Ensure current model/simulation exists;
     * 2) Optionally execute model check callback when model is not checked yet;
     * 3) Execute model synchronization callback (text -> model) and ensure simulation still exists.
     *
     * @param checkModel If true, model-check callback may be executed.
     * @param modelChecked Indicates current model check state from MainWindow.
     * @param runModelCheck Callback that executes model check when needed.
     * @param syncModelFromText Callback that synchronizes textual model representation.
     * @return true if command can proceed safely.
     *
     * @todo Replace callback-based contract by dedicated interfaces to improve testability.
     */
    bool ensureReady(bool checkModel,
                     bool modelChecked,
                     const std::function<bool()>& runModelCheck,
                     const std::function<bool()>& syncModelFromText) const;

private:
    QWidget* _ownerWidget;
    Simulator* _simulator;
};

#endif // SIMULATIONCONTROLLER_H
