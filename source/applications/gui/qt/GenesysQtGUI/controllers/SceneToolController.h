#ifndef SCENETOOLCONTROLLER_H
#define SCENETOOLCONTROLLER_H

#include <functional>

class ModelGraphicsView;
class ModelGraphicsScene;

namespace Ui {
class MainWindow;
}

// Document the scene tooling controller that receives delegated UI commands.
/**
 * @brief Controller for scene visualization, drawing tools, alignment, and zoom orchestration.
 *
 * MainWindow delegates many menu/toolbar wrappers to this Phase-10 controller to keep the
 * compatibility façade thin while concentrating scene-tool behavior in a dedicated component.
 *
 * Responsibilities:
 * - handle grid/ruler/guides/snap toggles and zoom commands;
 * - orchestrate drawing/connection modes and alignment actions;
 * - coordinate animation toggles and scene-display filters through injected callbacks.
 *
 * Boundaries:
 * - it does not own MainWindow or scene/view lifetime;
 * - it does not perform model persistence/export/simulation lifecycle work;
 * - it relies on injected references for shared UI state (zoom, first-click connection state).
 */
class SceneToolController {
public:
    // Inject only narrow dependencies required by scene and tool command handlers.
    SceneToolController(ModelGraphicsView* graphicsView,
                        Ui::MainWindow* ui,
                        std::function<ModelGraphicsScene*()> currentScene,
                        std::function<bool()> createModelImage,
                        std::function<void()> unselectDrawIcons,
                        std::function<bool()> checkSelectedDrawIcons,
                        std::function<void(double)> gentleZoom,
                        std::function<void()> actualizeActions,
                        std::function<void()> actualizeTabPanes,
                        int& zoomValue,
                        bool& firstClickShowConnection);

    void onActionShowGridTriggered();
    void onActionShowRuleTriggered();
    void onActionShowGuidesTriggered();
    void onActionShowSnapTriggered();
    void onActionZoomInTriggered();
    void onActionZoomOutTriggered();
    void onActionZoomAllTriggered();
    void onHorizontalSliderZoomGraphicalValueChanged(int value);
    void onActionDrawLineTriggered();
    void onActionDrawRectangleTriggered();
    void onActionDrawEllipseTriggered();
    void onActionDrawTextTriggered();
    void onActionDrawPoligonTriggered();
    void onActionAnimateSimulatedTimeTriggered();
    void onActionAnimateVariableTriggered();
    void onActionAnimateCounterTriggered();
    void onActionConnectTriggered();
    void onActionArranjeLeftTriggered();
    void onActionArranjeRightTriggered();
    void onActionArranjeTopTriggered();
    void onActionArranjeBototmTriggered();
    void onActionArranjeCenterTriggered();
    void onActionArranjeMiddleTriggered();
    void onActionActivateGraphicalSimulationTriggered();
    void onHorizontalSliderAnimationSpeedValueChanged(int value);
    void onActionDiagramsTriggered();
    void onActionSelectAllTriggered();
    void onActionShowInternalElementsTriggered();
    void onActionShowAttachedElementsTriggered();
    void onCheckBoxShowElementsStateChanged(int arg1);
    void onCheckBoxShowInternalsStateChanged(int arg1);
    void onCheckBoxShowRecursiveStateChanged(int arg1);
    void onCheckBoxShowLevelsStateChanged(int arg1);
    void onActionGModelShowConnectTriggered();

private:
    ModelGraphicsView* _graphicsView;
    Ui::MainWindow* _ui;
    std::function<ModelGraphicsScene*()> _currentScene;
    std::function<bool()> _createModelImage;
    std::function<void()> _unselectDrawIcons;
    std::function<bool()> _checkSelectedDrawIcons;
    std::function<void(double)> _gentleZoom;
    std::function<void()> _actualizeActions;
    std::function<void()> _actualizeTabPanes;
    int& _zoomValue;
    bool& _firstClickShowConnection;
};

#endif // SCENETOOLCONTROLLER_H
