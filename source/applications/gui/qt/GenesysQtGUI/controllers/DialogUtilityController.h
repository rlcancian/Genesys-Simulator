#ifndef DIALOGUTILITYCONTROLLER_H
#define DIALOGUTILITYCONTROLLER_H

#include <functional>

class MainWindow;
class Simulator;
class ModelGraphicsView;
class ModelGraphicsScene;
class QString;

namespace Ui {
class MainWindow;
}

// Document the dialog and utility controller extracted from remaining MainWindow actions.
/**
 * @brief Controller that orchestrates auxiliary dialogs and utility actions.
 *
 * This Phase-11 controller gathers non-core menu/toolbar actions that were still in
 * MainWindow, such as About dialogs, breakpoint utilities, optimization/preferences dialogs,
 * data analyzer launch flow, and parallelization toggles.
 *
 * Responsibilities:
 * - execute dialog-driven utility actions delegated from MainWindow wrappers;
 * - access shared UI/simulator state through narrow injected callbacks/references;
 * - keep compatibility behavior for debug breakpoints and auxiliary tooling actions.
 *
 * Boundaries:
 * - it does not own simulator/model lifecycle;
 * - it does not manage scene command pipeline or property editor internals;
 * - it acts as a controller-level compatibility wrapper, not as a domain service.
 */
class DialogUtilityController {
public:
    /** @brief Builds the utility controller that backs legacy MainWindow dialog wrappers. */
    DialogUtilityController(MainWindow* ownerWidget,
                            Simulator* simulator,
                            Ui::MainWindow* ui,
                            ModelGraphicsView* graphicsView,
                            std::function<void()> showMessageNotImplemented,
                            std::function<void(bool)> actualizeDebugBreakpoints,
                            std::function<bool()> createModelImage,
                            std::function<void()> actualizeActions,
                            std::function<void()> actualizeTabPanes,
                            std::function<void()> reloadPluginCatalog,
                            std::function<ModelGraphicsScene*()> currentScene,
                            double& optimizerPrecision,
                            unsigned int& optimizerMaxSteps,
                            bool& parallelizationEnabled,
                            int& parallelizationThreads,
                            int& parallelizationBatchSize,
                            QString& lastDataAnalyzerPath);

    /** @brief Opens the About dialog while preserving legacy action routing. */
    void onActionAboutAboutTriggered();
    /** @brief Opens the license dialog from the compatibility façade action slot. */
    void onActionAboutLicenceTriggered();
    /** @brief Opens the get-involved/help contribution dialog flow. */
    void onActionAboutGetInvolvedTriggered();
    /** @brief Runs find workflow for the currently focused text editor pane. */
    void onActionEditFindTriggered();
    /** @brief Runs replace workflow for the currently focused text editor pane. */
    void onActionEditReplaceTriggered();
    /** @brief Opens parser grammar checker utility without altering model state. */
    void onActionToolsParserGrammarCheckerTriggered();
    /** @brief Opens optimization settings dialog and stores lightweight preferences. */
    void onActionToolsOptimizatorTriggered();
    /** @brief Launches the Data Analyzer workstation using persisted last-path compatibility state. */
    void onActionToolsDataAnalyzerTriggered();
    /** @brief Opens view configuration dialog and applies delegated scene/UI refreshes. */
    void onActionViewConfigureTriggered();
    /** @brief Opens simulator preferences utility dialog. */
    void onActionSimulatorPreferencesTriggered();
    /** @brief Opens plugin manager dialog from the compatibility action surface. */
    void onActionSimulatorsPluginManagerTriggered(bool showProblemPlugins = false);
    /** @brief Inserts a breakpoint entry based on current scene selection. */
    void onPushButtonBreakpointInsertClicked();
    /** @brief Removes selected breakpoint entries and refreshes debug breakpoint pane. */
    void onPushButtonBreakpointRemoveClicked();
    /** @brief Exports current debug/trace context using legacy push-button route. */
    void onPushButtonExportClicked();
    /** @brief Opens parallelization configuration and persists run-preference flags. */
    void onActionParallelizationTriggered();

private:
    MainWindow* _ownerWidget;
    Simulator* _simulator;
    Ui::MainWindow* _ui;
    ModelGraphicsView* _graphicsView;
    std::function<void()> _showMessageNotImplemented;
    std::function<void(bool)> _actualizeDebugBreakpoints;
    std::function<bool()> _createModelImage;
    std::function<void()> _actualizeActions;
    std::function<void()> _actualizeTabPanes;
    std::function<void()> _reloadPluginCatalog;
    std::function<ModelGraphicsScene*()> _currentScene;
    double& _optimizerPrecision;
    unsigned int& _optimizerMaxSteps;
    bool& _parallelizationEnabled;
    int& _parallelizationThreads;
    int& _parallelizationBatchSize;
    QString& _lastDataAnalyzerPath;
};

#endif // DIALOGUTILITYCONTROLLER_H
