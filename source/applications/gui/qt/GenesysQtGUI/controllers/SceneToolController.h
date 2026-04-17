#ifndef SCENETOOLCONTROLLER_H
#define SCENETOOLCONTROLLER_H

#include <functional>

class ModelGraphicsView;
class ModelGraphicsScene;
class QAction;

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
    /**
     * @brief Builds the scene-tool delegated controller used by MainWindow compatibility slots.
     */
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

    /** @brief Toggles grid visibility and requests scene redraw updates. */
    void onActionShowGridTriggered();
    /** @brief Toggles ruler visibility through the delegated scene callback path. */
    void onActionShowRuleTriggered();
    /** @brief Toggles guide visibility and keeps tab/action synchronization. */
    void onActionShowGuidesTriggered();
    /** @brief Toggles snap behavior on the current scene without changing tool wiring. */
    void onActionShowSnapTriggered();
    /** @brief Applies incremental zoom-in and updates shared zoom state. */
    void onActionZoomInTriggered();
    /** @brief Applies incremental zoom-out and updates shared zoom state. */
    void onActionZoomOutTriggered();
    /** @brief Fits the current graphical model in view and re-synchronizes zoom widgets. */
    void onActionZoomAllTriggered();
    /** @brief Synchronizes slider-originated zoom changes with scene/view scale. */
    void onHorizontalSliderZoomGraphicalValueChanged(int value);
    /** @brief Activates line drawing mode as a compatibility façade wrapper. */
    void onActionDrawLineTriggered();
    /** @brief Activates rectangle drawing mode as a compatibility façade wrapper. */
    void onActionDrawRectangleTriggered();
    /** @brief Activates ellipse drawing mode as a compatibility façade wrapper. */
    void onActionDrawEllipseTriggered();
    /** @brief Activates text drawing mode as a compatibility façade wrapper. */
    void onActionDrawTextTriggered();
    /** @brief Activates polygon drawing mode as a compatibility façade wrapper. */
    void onActionDrawPoligonTriggered();
    /** @brief Toggles simulated-time animation overlays in the current scene. */
    void onActionAnimateSimulatedTimeTriggered();
    /** @brief Toggles variable animation overlays in the current scene. */
    void onActionAnimateVariableTriggered();
    /** @brief Toggles counter animation overlays in the current scene. */
    void onActionAnimateCounterTriggered();
    /** @brief Activates attribute animation placeholder drawing. */
    void onActionAnimateAttributeTriggered();
    /** @brief Activates entity animation placeholder drawing. */
    void onActionAnimateEntityTriggered();
    /** @brief Activates event animation placeholder drawing. */
    void onActionAnimateEventTriggered();
    /** @brief Activates expression animation placeholder drawing. */
    void onActionAnimateExpressionTriggered();
    /** @brief Activates plot animation placeholder drawing. */
    void onActionAnimatePlotTriggered();
    /** @brief Activates queue animation placeholder drawing. */
    void onActionAnimateQueueTriggered();
    /** @brief Activates resource animation placeholder drawing. */
    void onActionAnimateResourceTriggered();
    /** @brief Activates station animation placeholder drawing. */
    void onActionAnimateStationTriggered();
    /** @brief Activates statistics animation placeholder drawing. */
    void onActionAnimateStatisticsTriggered();
    /** @brief Activates connection-creation mode and resets first-click compatibility state. */
    void onActionConnectTriggered();
    /** @brief Aligns selected graphical items to the left boundary. */
    void onActionArranjeLeftTriggered();
    /** @brief Aligns selected graphical items to the right boundary. */
    void onActionArranjeRightTriggered();
    /** @brief Aligns selected graphical items to the top boundary. */
    void onActionArranjeTopTriggered();
    /** @brief Aligns selected graphical items to the bottom boundary. */
    void onActionArranjeBototmTriggered();
    /** @brief Aligns selected graphical items to horizontal center. */
    void onActionArranjeCenterTriggered();
    /** @brief Aligns selected graphical items to vertical middle. */
    void onActionArranjeMiddleTriggered();
    /** @brief Toggles graphical simulation visualization mode and action state. */
    void onActionActivateGraphicalSimulationTriggered();
    /** @brief Applies animation-speed slider values to scene animation pacing. */
    void onHorizontalSliderAnimationSpeedValueChanged(int value);
    /** @brief Selects all graphical items in the active scene context. */
    void onActionSelectAllTriggered();
    /** @brief Toggles rendering of statistics data definitions and triggers representation refresh. */
    void onActionShowInternalElementsTriggered();
    /** @brief Toggles rendering of editable data definitions and triggers representation refresh. */
    void onActionShowEditableElementsTriggered();
    /** @brief Toggles rendering of shared data definitions and triggers representation refresh. */
    void onActionShowAttachedElementsTriggered();
    /** @brief Bridges checkbox state changes to the show-elements action wrapper. */
    void onCheckBoxShowElementsStateChanged(int arg1);
    /** @brief Bridges checkbox state changes to the show-internals action wrapper. */
    void onCheckBoxShowInternalsStateChanged(int arg1);
    /** @brief Bridges checkbox state changes to the show-editable-elements action wrapper. */
    void onCheckBoxShowEditableElementsStateChanged(int arg1);
    /** @brief Toggles recursive data-definition expansion and triggers representation refresh. */
    void onActionShowRecursiveElementsTriggered();
    /** @brief Bridges recursive-render checkbox changes to model-image refresh workflow. */
    void onCheckBoxShowRecursiveStateChanged(int arg1);
    /** @brief Bridges level-annotation checkbox changes to model-image refresh workflow. */
    void onCheckBoxShowLevelsStateChanged(int arg1);
    /** @brief Toggles interactive connection-hint visibility in scene callback wiring. */
    void onActionGModelShowConnectTriggered();

private:
    void activateAnimationDrawingTool(QAction* action, void (ModelGraphicsScene::*drawingFunction)());

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
