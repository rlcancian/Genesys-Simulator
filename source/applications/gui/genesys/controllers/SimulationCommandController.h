#ifndef SIMULATIONCOMMANDCONTROLLER_H
#define SIMULATIONCOMMANDCONTROLLER_H

#include <functional>
#include <string>

class SimulationController;

// Document simulation command orchestration delegated by MainWindow slots.
/**
 * @brief Controller that orchestrates UI simulation commands over SimulationController guards.
 *
 * This class sits between MainWindow compatibility slots and the low-level simulation
 * readiness controller. It coordinates command intent (start/step/pause/resume/stop), command
 * logging, and action refresh callbacks while preserving the existing UI trigger surface.
 *
 * Responsibilities:
 * - execute simulation command flows delegated from MainWindow;
 * - use SimulationController precondition checks before unsafe commands;
 * - invoke injected callbacks for console command logging and action-state refresh.
 *
 * Boundaries:
 * - it does not own simulation state or kernel event wiring;
 * - it does not render trace/event widgets;
 * - it is a controller/orchestration layer, not a persistence or synchronization service.
 */
class SimulationCommandController {
public:
    /**
     * @brief Creates a command controller with narrow callbacks from MainWindow.
     */
    SimulationCommandController(
        SimulationController* simulationController,
        const std::function<void(const std::string&)>& insertCommandInConsole,
        const std::function<void()>& actualizeActions,
        const std::function<bool()>& checkModel,
        const std::function<bool()>& setSimulationModelBasedOnText,
        const std::function<void()>& prepareNewSimulationOutputs);

    /** @brief Delegates the start simulation command flow. */
    void onActionSimulationStartTriggered(bool modelChecked) const;
    /** @brief Delegates the step simulation command flow. */
    void onActionSimulationStepTriggered(bool modelChecked) const;
    /** @brief Delegates the pause simulation command flow. */
    void onActionSimulationPauseTriggered() const;
    /** @brief Delegates the resume simulation command flow. */
    void onActionSimulationResumeTriggered(bool modelChecked) const;
    /** @brief Delegates the stop simulation command flow. */
    void onActionSimulationStopTriggered() const;

private:
    SimulationController* _simulationController;
    std::function<void(const std::string&)> _insertCommandInConsole;
    std::function<void()> _actualizeActions;
    std::function<bool()> _checkModel;
    std::function<bool()> _setSimulationModelBasedOnText;
    std::function<void()> _prepareNewSimulationOutputs;
};

#endif // SIMULATIONCOMMANDCONTROLLER_H
