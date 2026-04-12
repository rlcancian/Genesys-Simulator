#include "SceneToolController.h"

#include "ui_mainwindow.h"
#include "../TraitsGUI.h"
#include "../graphicals/ModelGraphicsView.h"
#include "../graphicals/ModelGraphicsScene.h"
#include "../graphicals/GraphicalModelComponent.h"
#include "../animations/AnimationTransition.h"

#include <QGraphicsItem>
#include <QSignalBlocker>
#include <QDebug>
#include <Qt>

// Store only narrow collaborators needed for Phase 10 scene-tool orchestration.
SceneToolController::SceneToolController(ModelGraphicsView* graphicsView,
                                         Ui::MainWindow* ui,
                                         std::function<ModelGraphicsScene*()> currentScene,
                                         std::function<bool()> createModelImage,
                                         std::function<void()> unselectDrawIcons,
                                         std::function<bool()> checkSelectedDrawIcons,
                                         std::function<void(double)> gentleZoom,
                                         std::function<void()> actualizeActions,
                                         std::function<void()> actualizeTabPanes,
                                         int& zoomValue,
                                         bool& firstClickShowConnection)
    : _graphicsView(graphicsView),
      _ui(ui),
      _currentScene(std::move(currentScene)),
      _createModelImage(std::move(createModelImage)),
      _unselectDrawIcons(std::move(unselectDrawIcons)),
      _checkSelectedDrawIcons(std::move(checkSelectedDrawIcons)),
      _gentleZoom(std::move(gentleZoom)),
      _actualizeActions(std::move(actualizeActions)),
      _actualizeTabPanes(std::move(actualizeTabPanes)),
      _zoomValue(zoomValue),
      _firstClickShowConnection(firstClickShowConnection) {
}

// Preserve deterministic action-to-scene grid synchronization.
void SceneToolController::onActionShowGridTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->setGridVisible(_ui->actionShowGrid->isChecked());
    }
}

// Preserve ruler state synchronization between QAction and graphics view backend.
void SceneToolController::onActionShowRuleTriggered() {
    const bool requestedVisible = _ui->actionShowRule->isChecked();
    _graphicsView->setRuleVisible(requestedVisible);
    _ui->actionShowRule->setChecked(_graphicsView->isRuleVisible());
}

// Preserve guides state synchronization between QAction and graphics view backend.
void SceneToolController::onActionShowGuidesTriggered() {
    const bool requestedVisible = _ui->actionShowGuides->isChecked();
    _graphicsView->setGuidesVisible(requestedVisible);
    _ui->actionShowGuides->setChecked(_graphicsView->isGuidesVisible());
}

// Preserve deterministic snap-to-grid synchronization from QAction to scene.
void SceneToolController::onActionShowSnapTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->setSnapToGrid(_ui->actionShowSnap->isChecked());
    }
}

// Preserve zoom-in command behavior through the graphical zoom slider.
void SceneToolController::onActionZoomInTriggered() {
    const int value = _ui->horizontalSlider_ZoomGraphical->value();
    _ui->horizontalSlider_ZoomGraphical->setValue(value + TraitsGUI<GMainWindow>::zoomButtonChange);
}

// Preserve zoom-out command behavior through the graphical zoom slider.
void SceneToolController::onActionZoomOutTriggered() {
    const int value = _ui->horizontalSlider_ZoomGraphical->value();
    _ui->horizontalSlider_ZoomGraphical->setValue(value - TraitsGUI<GMainWindow>::zoomButtonChange);
}

// Preserve zoom-all fit behavior and reset the slider baseline coherently.
void SceneToolController::onActionZoomAllTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr || scene->items().isEmpty()) {
        return;
    }

    const QRectF bounds = scene->itemsBoundingRect();
    if (!bounds.isValid() || bounds.isEmpty()) {
        return;
    }

    _graphicsView->resetTransform();
    _graphicsView->fitInView(bounds.adjusted(-20.0, -20.0, 20.0, 20.0), Qt::KeepAspectRatio);
    {
        QSignalBlocker blocker(_ui->horizontalSlider_ZoomGraphical);
        _zoomValue = _ui->horizontalSlider_ZoomGraphical->maximum() / 2;
        _ui->horizontalSlider_ZoomGraphical->setValue(_zoomValue);
    }
    _graphicsView->centerOn(bounds.center());
}

// Preserve graphical slider zoom delta behavior and gentle zoom scaling.
void SceneToolController::onHorizontalSliderZoomGraphicalValueChanged(int value) {
    const double factor = (value - _zoomValue) * 0.002;
    _zoomValue = value;
    _gentleZoom(1.0 + factor);
}

// Preserve line-drawing tool activation and cursor semantics.
void SceneToolController::onActionDrawLineTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionDrawLine->isChecked()) {
        _graphicsView->setCursor(Qt::SizeHorCursor);
        _ui->actionDrawLine->setChecked(true);
        scene->setAction(_ui->actionDrawLine);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::LINE);
    } else {
        _unselectDrawIcons();
    }
}

// Preserve rectangle-drawing tool activation and cursor semantics.
void SceneToolController::onActionDrawRectangleTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionDrawRectangle->isChecked()) {
        _graphicsView->setCursor(Qt::CrossCursor);
        _ui->actionDrawRectangle->setChecked(true);
        scene->setAction(_ui->actionDrawRectangle);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::RECTANGLE);
    } else {
        _unselectDrawIcons();
    }
}

// Preserve ellipse-drawing tool activation and cursor semantics.
void SceneToolController::onActionDrawEllipseTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionDrawEllipse->isChecked()) {
        _graphicsView->setCursor(Qt::CrossCursor);
        _ui->actionDrawEllipse->setChecked(true);
        scene->setAction(_ui->actionDrawEllipse);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::ELLIPSE);
    } else {
        _unselectDrawIcons();
    }
}

// Preserve text-drawing tool activation semantics.
void SceneToolController::onActionDrawTextTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionDrawText->isChecked()) {
        _ui->actionDrawText->setChecked(true);
        scene->setAction(_ui->actionDrawText);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::TEXT);
    } else {
        _unselectDrawIcons();
    }
}

// Preserve polygon-drawing tool activation and cursor semantics.
void SceneToolController::onActionDrawPoligonTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionDrawPoligon->isChecked()) {
        _graphicsView->setCursor(Qt::ArrowCursor);
        _ui->actionDrawPoligon->setChecked(true);
        scene->setAction(_ui->actionDrawPoligon);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::POLYGON);
    } else {
        _unselectDrawIcons();
    }
}

// Preserve simulated-time animation tool activation behavior.
void SceneToolController::onActionAnimateSimulatedTimeTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionAnimateSimulatedTime->isChecked()) {
        _graphicsView->setCursor(Qt::CrossCursor);
        scene->setAction(_ui->actionAnimateSimulatedTime);
        scene->drawingTimer();
    } else {
        _unselectDrawIcons();
    }
}

// Preserve variable animation tool activation behavior.
void SceneToolController::onActionAnimateVariableTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionAnimateVariable->isChecked()) {
        _graphicsView->setCursor(Qt::CrossCursor);
        scene->setAction(_ui->actionAnimateVariable);
        scene->drawingVariable();
    } else {
        _unselectDrawIcons();
    }
}

// Preserve counter animation tool activation behavior.
void SceneToolController::onActionAnimateCounterTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_checkSelectedDrawIcons() && _ui->actionAnimateCounter->isChecked()) {
        _graphicsView->setCursor(Qt::CrossCursor);
        scene->setAction(_ui->actionAnimateCounter);
        scene->drawingCounter();
    } else {
        _unselectDrawIcons();
    }
}

// Preserve connect-tool activation through the existing graphics-view flow.
void SceneToolController::onActionConnectTriggered() {
    _graphicsView->beginConnection();
}

// Preserve left arrange command semantics.
void SceneToolController::onActionArranjeLeftTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(0);
    }
}

// Preserve right arrange command semantics.
void SceneToolController::onActionArranjeRightTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(1);
    }
}

// Preserve top arrange command semantics.
void SceneToolController::onActionArranjeTopTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(2);
    }
}

// Preserve bottom arrange command semantics.
void SceneToolController::onActionArranjeBototmTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(3);
    }
}

// Preserve center arrange command semantics.
void SceneToolController::onActionArranjeCenterTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(4);
    }
}

// Preserve middle arrange command semantics.
void SceneToolController::onActionArranjeMiddleTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene != nullptr) {
        scene->arranjeModels(5);
    }
}

// Preserve graphical simulation visibility toggle and animation running state.
void SceneToolController::onActionActivateGraphicalSimulationTriggered() {
    bool visivible = true;

    if (!_ui->actionActivateGraphicalSimulation->isChecked()) {
        AnimationTransition::setRunning(false);
        visivible = false;
    } else {
        AnimationTransition::setRunning(true);
    }

    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    QList<QGraphicsItem*>* componentes = scene->getGraphicalModelComponents();
    for (QGraphicsItem* item : *componentes) {
        if (GraphicalModelComponent* component = dynamic_cast<GraphicalModelComponent*>(item)) {
            component->visivibleImageQueue(visivible);
        }
    }
}

// Preserve animation speed slider conversion to execution time value.
void SceneToolController::onHorizontalSliderAnimationSpeedValueChanged(int value) {
    const double newValue = static_cast<double>(value) / 2.0;
    AnimationTransition::setTimeExecution(newValue);
}

// Preserve diagram visibility logic and effective QAction re-synchronization.
void SceneToolController::onActionDiagramsTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (_ui->actionDiagrams->isChecked()) {
        if (!scene->existDiagram()) {
            scene->createDiagrams();
        }
        scene->showDiagrams();
    } else {
        if (scene->existDiagram()) {
            scene->hideDiagrams();
        }
    }

    const bool diagramsVisible = scene->existDiagram() && scene->visibleDiagram();
    if (_ui->actionDiagrams->isChecked() != diagramsVisible) {
        _ui->actionDiagrams->setChecked(diagramsVisible);
    }
}

// Preserve select-all semantics while ignoring non-operable internal infrastructure items.
void SceneToolController::onActionSelectAllTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    const QList<QGraphicsItem*> itemsToScene = scene->userOperableItems(scene->items());
    for (QGraphicsItem* item : itemsToScene) {
        item->setSelected(true);
    }
}

// Preserve action-checkbox synchronization for internals image visibility.
void SceneToolController::onActionShowInternalElementsTriggered() {
    const bool checked = _ui->actionShowInternalElements->isChecked();
    if (_ui->checkBox_ShowInternals->isChecked() != checked) {
        _ui->checkBox_ShowInternals->setChecked(checked);
    } else {
        _createModelImage();
    }
}

// Preserve action-checkbox synchronization for attached-elements image visibility.
void SceneToolController::onActionShowAttachedElementsTriggered() {
    const bool checked = _ui->actionShowAttachedElements->isChecked();
    if (_ui->checkBox_ShowElements->isChecked() != checked) {
        _ui->checkBox_ShowElements->setChecked(checked);
    } else {
        _createModelImage();
    }
}

// Preserve checkbox-to-action synchronization for attached-elements visibility.
void SceneToolController::onCheckBoxShowElementsStateChanged(int arg1) {
    _ui->actionShowAttachedElements->setChecked(arg1 == Qt::Checked);
    _createModelImage();
}

// Preserve checkbox-to-action synchronization for internals visibility.
void SceneToolController::onCheckBoxShowInternalsStateChanged(int arg1) {
    _ui->actionShowInternalElements->setChecked(arg1 == Qt::Checked);
    _createModelImage();
}

// Preserve recursive image-toggle behavior by regenerating model image.
void SceneToolController::onCheckBoxShowRecursiveStateChanged(int arg1) {
    Q_UNUSED(arg1)
    _createModelImage();
}

// Preserve levels image-toggle behavior by regenerating model image.
void SceneToolController::onCheckBoxShowLevelsStateChanged(int arg1) {
    Q_UNUSED(arg1)
    _createModelImage();
}

// Preserve legacy connect-step toggle semantics and first-click synchronization.
void SceneToolController::onActionGModelShowConnectTriggered() {
    ModelGraphicsScene* scene = _currentScene();
    if (scene == nullptr) {
        return;
    }

    if (!_ui->actionGModelShowConnect->isChecked() && !_firstClickShowConnection) {
        qInfo() << "Connection tool deactivated";
        _ui->actionGModelShowConnect->setChecked(false);
        scene->setConnectingStep(0);
        _graphicsView->setCursor(Qt::ArrowCursor);
    } else {
        qInfo() << "Connection tool activated";
        _ui->actionGModelShowConnect->setChecked(true);
        scene->beginConnection();
        _firstClickShowConnection = false;
    }
}
