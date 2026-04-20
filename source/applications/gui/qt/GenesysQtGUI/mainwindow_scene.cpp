// Document this compilation unit as the scene-synchronization partition of MainWindow.
/**
 * @file mainwindow_scene.cpp
 * @brief Scene callback and scene synchronization partition of MainWindow implementation.
 *
 * This file concentrates scene-related callbacks, selection/focus/zoom bridging, and wrappers
 * that synchronize scene state with UI elements such as actions and the property editor
 * controller. It does not define a new class; it is a thematic partition of MainWindow.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
// Include the Phase 6 controller type so member calls compile with a complete class definition.
#include "controllers/PropertyEditorController.h"
#include "GuiScopeTrace.h"
#include <QDebug>

//-----------------------------------------

// Keep this scene callback local to MainWindow because it updates direct UI status widgets.
/**
 * @brief Updates the coordinate label with the current scene mouse position.
 * @param mouseEvent Mouse event received from the graphical scene.
 */
void MainWindow::_onSceneMouseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
    QPointF pos = mouseEvent->scenePos();
    ui->labelMousePos->setText(QString::fromStdString("<" + std::to_string((int) pos.x()) + "," + std::to_string((int) pos.y()) + ">"));
}

// Keep this scene callback local to MainWindow because it bridges wheel events to existing slider state.
/**
 * @brief Increases graphical view zoom through the main slider.
 */
void MainWindow::_onSceneWheelInEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value + TraitsGUI<GMainWindow>::zoomButtonChange);
}

// Keep this scene callback local to MainWindow because it bridges wheel events to existing slider state.
/**
 * @brief Decreases graphical view zoom through the main slider.
 */
void MainWindow::_onSceneWheelOutEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value - TraitsGUI<GMainWindow>::zoomButtonChange);
}

// Keep this scene callback as a compatibility façade entry-point for tab updates.
/**
 * @brief Refreshes dependent panes when the graphical model changes.
 * @param event Graphical change event (currently unused by this compatibility wrapper).
 */
void MainWindow::_onSceneGraphicalModelEvent(const GraphicalModelEvent& /*event*/) {
    // Any structural graphical event changes the persisted .gui representation.
    setGraphicalModelHasChanged(true);
}

//-----------------------------------------

// Keep this slot in MainWindow because it centralizes action-state synchronization.
/**
 * @brief Scene-change notification slot (undo/redo and edit action synchronization).
 * @param region Invalidated scene regions.
 */
void MainWindow::sceneChanged(const QList<QRectF> &region) {
    if (_shuttingDown || ui == nullptr || ui->graphicsView == nullptr || ui->graphicsView->getScene() == nullptr) {
        return;
    }
    Q_UNUSED(region);
    // Synchronize undo/redo state and textual model dirty flag.
    bool canUndo = ui->graphicsView->getScene()->getUndoStack()->canUndo();
    bool canRedo = ui->graphicsView->getScene()->getUndoStack()->canRedo();

    ui->actionEditUndo->setEnabled(canUndo);
    ui->actionEditRedo->setEnabled(canRedo);

    _actualizeModelTextHasChanged(canUndo);

    ui->graphicsView->scene()->update();

    // Refresh edit-action enablement based on current selection and copy buffers.
    _actualizeActions();

    // Finalize visual state refresh for connection tool and scene update.
    if (ui->graphicsView->getScene()->connectingStep() == 0)
        ui->actionGModelShowConnect->setChecked(false);

    ui->graphicsView->scene()->update();
}

// Keep this helper local to MainWindow because it contributes to centralized action-state logic.
/**
 * @brief Checks whether the scene has relevant items to enable edit operations.
 * @return true when components/drawings/animations exist in the graphical model.
 */
bool MainWindow::_checkItemsScene() {
    bool res = false;

    QList<QGraphicsItem *> *components = myScene()->getGraphicalModelComponents();
    QList<QGraphicsItem *> *geometries = myScene()->getGraphicalGeometries();
    QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();
    QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();
    QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();
    QList<AnimationPlaceholder *> *placeholders = myScene()->getAnimationsPlaceholder();

    if (!components->empty() || !geometries->empty() || !counters->empty() || !variables->empty() || !timers->empty() || !placeholders->empty()) {
        res = true;
    }

    return res;
}
void MainWindow::sceneFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason) {
    Q_UNUSED(newFocusItem);
    Q_UNUSED(oldFocusItem);
    Q_UNUSED(reason);
    // int a = 0;
}
//void sceneRectChanged(const QRectF &rect){}

// Keep this wrapper as part of the final compatibility façade for property editor synchronization.
/**
 * @brief Slot called when scene selection changes.
 *
 * Updates property editor state according to current selection.
 */
void MainWindow::sceneSelectionChanged() {
    // Adds scoped tracing for scene-selection diagnostics in GUI crash investigations.
    const GuiScopeTrace scopeTrace("MainWindow::sceneSelectionChanged", this);
    qInfo() << "[MainWindow] sceneSelectionChanged enter";
    // Keep this wrapper for compatibility during the incremental Phase 6 refactor.
    if (_shuttingDown) {
        qInfo() << "[MainWindow] sceneSelectionChanged exit early due to shutdown";
        return;
    }
    // Skip property editor synchronization while simulation is running or paused.
    const bool simulationInteractionLocked =
        simulator != nullptr &&
        simulator->getModelManager()->current() != nullptr &&
        simulator->getModelManager()->current()->getSimulation() != nullptr &&
        (simulator->getModelManager()->current()->getSimulation()->isRunning() ||
         simulator->getModelManager()->current()->getSimulation()->isPaused());

    if (simulationInteractionLocked) {
        qInfo() << "[MainWindow] sceneSelectionChanged skipped while simulation keeps property editor disabled";
        return;
    }
    if (_propertyEditorController == nullptr) {
        if (ui != nullptr && ui->treeViewPropertyEditor != nullptr) {
            qWarning() << "[MainWindow] sceneSelectionChanged without controller. Clearing property editor directly";
            ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();
        }
        qInfo() << "[MainWindow] sceneSelectionChanged exit early";
        return;
    }
    if (_propertyEditorController->isPostCommitPipelineActive()) {
        qInfo() << "[MainWindow] sceneSelectionChanged observed active post-commit pipeline; selection sync will be deferred";
    }
    _propertyEditorController->sceneSelectionChanged();
    _actualizeActions();
    qInfo() << "[MainWindow] sceneSelectionChanged exit";
}

//-----------------------------------------

void MainWindow::sceneGraphicalModelChanged() {
    _actualizeTabPanes();
}
