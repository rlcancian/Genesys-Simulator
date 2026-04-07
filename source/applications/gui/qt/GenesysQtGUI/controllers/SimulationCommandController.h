#ifndef SIMULATIONCOMMANDCONTROLLER_H
#define SIMULATIONCOMMANDCONTROLLER_H

#include <functional>
#include <string>

class SimulationController;

/**
 * @brief Phase 8 controller for UI-level simulation command orchestration.
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
        const std::function<bool()>& setSimulationModelBasedOnText);

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
};

#endif // SIMULATIONCOMMANDCONTROLLER_H
