#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include <functional>

class QWidget;
class Simulator;
class ModelSimulation;

/**
 * @brief Thin controller responsible for simulation-command preconditions in GUI.
 *
 * This class centralizes guard logic previously duplicated across simulation actions
 * (`start`, `step`, `pause`, `resume`, `stop`) in MainWindow.
 *
 * It deliberately avoids owning simulation model state; ownership remains in Simulator.
 *
 * @todo Evolve this controller to encapsulate command execution itself
 *       (not only readiness validation), reducing MainWindow responsibilities further.
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
