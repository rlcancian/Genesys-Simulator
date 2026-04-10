#ifndef SIMULATIONEVENTCONTROLLER_H
#define SIMULATIONEVENTCONTROLLER_H

#include <functional>

#include "../../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../../kernel/simulator/OnEventManager.h"
#include "../../../../../kernel/simulator/Simulator.h"

class MainWindow;
class ModelGraphicsScene;
class ModelGraphicsView;
class QLabel;
class QProgressBar;
class QTableWidget;
class QTextEdit;
class QTabWidget;
class QAction;

// Document the controller that bridges kernel simulation events to GUI updates.
/**
 * @brief Controller for simulation event callbacks, UI updates, and handler wiring.
 *
 * This controller concentrates event-reaction logic previously embedded in MainWindow and
 * keeps compatibility by exposing methods invoked from MainWindow wrappers. It also provides
 * the event-registration entry point used to wire kernel OnEventManager callbacks to those
 * wrappers without changing the existing signal/slot surface.
 *
 * Responsibilities:
 * - react to model-check, replication, simulation, process, and entity events;
 * - update progress/status widgets and debug tables through injected UI dependencies;
 * - trigger delegated refresh callbacks for actions, debug panes, and scene synchronization;
 * - register handlers through MainWindow compatibility façade ownership.
 *
 * Boundaries:
 * - it does not own MainWindow or widget lifetime;
 * - it does not execute simulation commands (handled by command controllers);
 * - it does not perform persistence/export/model-language synchronization.
 */
class SimulationEventController {
public:
    // Group callback dependencies to keep constructor explicit and narrow.
    struct Callbacks {
        std::function<void()> actualizeActions;
        std::function<void(SimulationEvent*)> actualizeSimulationEvents;
        std::function<void(bool)> actualizeDebugEntities;
        std::function<void(bool)> actualizeDebugVariables;
        std::function<void(SimulationEvent*)> actualizeGraphicalModel;
    };

    // Inject simulator/view/widget dependencies required by legacy event reactions.
    SimulationEventController(Simulator* simulator,
                              ModelGraphicsScene* scene,
                              ModelGraphicsView* graphicsView,
                              QLabel* replicationLabel,
                              QProgressBar* simulationProgressBar,
                              QTableWidget* simulationEventsTable,
                              QTableWidget* entitiesTable,
                              QTableWidget* variablesTable,
                              QTextEdit* simulationText,
                              QTextEdit* reportsText,
                              QTabWidget* centralTabWidget,
                              QAction* activateGraphicalSimulation,
                              bool* modelChecked,
                              int tabCentralReportsIndex,
                              Callbacks callbacks);

    // Keep model-check success handling behavior equivalent to legacy MainWindow logic.
    void onModelCheckSuccessHandler(ModelEvent* re) const;
    // Keep replication-start UI updates equivalent to legacy MainWindow logic.
    void onReplicationStartHandler(SimulationEvent* re) const;
    // Keep simulation-start UI reset/setup equivalent to legacy MainWindow logic.
    void onSimulationStartHandler(SimulationEvent* re) const;
    // Keep simulation-paused reaction equivalent to legacy MainWindow logic.
    void onSimulationPausedHandler(SimulationEvent* re) const;
    // Keep simulation-resume animation continuation behavior equivalent to legacy MainWindow logic.
    void onSimulationResumeHandler(SimulationEvent* re) const;
    // Keep simulation-end cleanup behavior equivalent to legacy MainWindow logic.
    void onSimulationEndHandler(SimulationEvent* re) const;
    // Keep process-event updates equivalent to legacy MainWindow logic.
    void onProcessEventHandler(SimulationEvent* re) const;
    // Keep entity-create hook compatibility.
    void onEntityCreateHandler(SimulationEvent* re) const;
    // Keep entity-remove hook compatibility.
    void onEntityRemoveHandler(SimulationEvent* re) const;
    // Keep entity-move animation behavior equivalent to legacy MainWindow logic.
    void onMoveEntityEvent(SimulationEvent* re) const;
    // Keep after-process animation updates equivalent to legacy MainWindow logic.
    void onAfterProcessEvent(SimulationEvent* re) const;
    // Register simulation event handlers on OnEventManager while keeping MainWindow wrappers.
    void setOnEventHandlers(MainWindow* owner) const;

private:
    // Keep simulator access scoped to simulation-event responsibilities.
    Simulator* _simulator;
    // Keep scene dependency for animation-related handlers.
    ModelGraphicsScene* _scene;
    // Keep graphics-view dependency for legacy compatibility and potential scene access.
    ModelGraphicsView* _graphicsView;
    // Keep replication label dependency for replication-start updates.
    QLabel* _replicationLabel;
    // Keep simulation progress bar dependency for process-event updates.
    QProgressBar* _simulationProgressBar;
    // Keep simulation-events table dependency for replication-start updates.
    QTableWidget* _simulationEventsTable;
    // Keep entities table dependency for simulation-start resets.
    QTableWidget* _entitiesTable;
    // Keep variables table dependency for simulation-start resets.
    QTableWidget* _variablesTable;
    // Keep simulation text dependency for simulation-start resets.
    QTextEdit* _simulationText;
    // Keep reports text dependency for simulation-start resets.
    QTextEdit* _reportsText;
    // Keep central-tab dependency for simulation-end tab switch.
    QTabWidget* _centralTabWidget;
    // Keep action dependency used by graphical simulation toggle.
    QAction* _activateGraphicalSimulation;
    // Keep mutable model-check flag dependency used at simulation end.
    bool* _modelChecked;
    // Keep tab index dependency used for report-tab activation.
    int _tabCentralReportsIndex;
    // Keep callback dependencies for delegated MainWindow operations.
    Callbacks _callbacks;
};

#endif // SIMULATIONEVENTCONTROLLER_H
