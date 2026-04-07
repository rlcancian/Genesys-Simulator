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

// Move remaining dialog and utility orchestration into a dedicated Phase 11 controller.
class DialogUtilityController {
public:
    // Inject only narrow dependencies required by Phase 11 dialog and utility workflows.
    DialogUtilityController(MainWindow* ownerWidget,
                            Simulator* simulator,
                            Ui::MainWindow* ui,
                            ModelGraphicsView* graphicsView,
                            std::function<void()> showMessageNotImplemented,
                            std::function<void(bool)> actualizeDebugBreakpoints,
                            std::function<bool()> createModelImage,
                            std::function<void()> actualizeActions,
                            std::function<void()> actualizeTabPanes,
                            std::function<ModelGraphicsScene*()> currentScene,
                            double& optimizerPrecision,
                            unsigned int& optimizerMaxSteps,
                            bool& parallelizationEnabled,
                            int& parallelizationThreads,
                            int& parallelizationBatchSize,
                            QString& lastDataAnalyzerPath);

    void onActionAboutAboutTriggered();
    void onActionAboutLicenceTriggered();
    void onActionAboutGetInvolvedTriggered();
    void onActionEditFindTriggered();
    void onActionEditReplaceTriggered();
    void onActionToolsParserGrammarCheckerTriggered();
    void onActionToolsOptimizatorTriggered();
    void onActionToolsDataAnalyzerTriggered();
    void onActionViewConfigureTriggered();
    void onActionSimulatorPreferencesTriggered();
    void onActionSimulatorsPluginManagerTriggered();
    void onPushButtonBreakpointInsertClicked();
    void onPushButtonBreakpointRemoveClicked();
    void onPushButtonExportClicked();
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
    std::function<ModelGraphicsScene*()> _currentScene;
    double& _optimizerPrecision;
    unsigned int& _optimizerMaxSteps;
    bool& _parallelizationEnabled;
    int& _parallelizationThreads;
    int& _parallelizationBatchSize;
    QString& _lastDataAnalyzerPath;
};

#endif // DIALOGUTILITYCONTROLLER_H
