#include "PropertyEditorController.h"

#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalModelComponent.h"
#include "propertyeditor/ObjectPropertyBrowser.h"

#include <QGraphicsItem>
#include <QDebug>

// Build the Phase 6 controller with narrow dependencies for property-editor orchestration.
PropertyEditorController::PropertyEditorController(
    ObjectPropertyBrowser* propertyBrowser,
    ModelGraphicsView* graphicsView,
    PropertyEditorGenesys* propertyGenesys,
    std::map<SimulationControl*, DataComponentProperty*>* propertyList,
    std::map<SimulationControl*, DataComponentEditor*>* propertyEditorUI,
    std::map<SimulationControl*, ComboBoxEnum*>* propertyBox,
    std::function<void()> actualizeModelSimLanguage,
    std::function<void(bool)> actualizeModelComponents,
    std::function<void(bool)> actualizeModelDataDefinitions,
    std::function<void()> actualizeModelCppCode,
    std::function<bool()> createModelImage,
    std::function<void()> actualizeTabPanes,
    std::function<void()> actualizeActions)
    : _propertyBrowser(propertyBrowser),
      _graphicsView(graphicsView),
      _propertyGenesys(propertyGenesys),
      _propertyList(propertyList),
      _propertyEditorUI(propertyEditorUI),
      _propertyBox(propertyBox),
      _actualizeModelSimLanguage(std::move(actualizeModelSimLanguage)),
      _actualizeModelComponents(std::move(actualizeModelComponents)),
      _actualizeModelDataDefinitions(std::move(actualizeModelDataDefinitions)),
      _actualizeModelCppCode(std::move(actualizeModelCppCode)),
      _createModelImage(std::move(createModelImage)),
      _actualizeTabPanes(std::move(actualizeTabPanes)),
      _actualizeActions(std::move(actualizeActions)) {
}

// Keep property-browser cleanup centralized so stale selection bindings are removed safely.
void PropertyEditorController::clearPropertyEditorSelection() const {
    if (_propertyBrowser == nullptr) {
        return;
    }

    qInfo() << "[PropertyEditorController] Clearing property editor selection and context";
    _propertyBrowser->clearCurrentlyConnectedObject();
}

// Preserve legacy single-selection behavior while moving orchestration out of MainWindow.
void PropertyEditorController::sceneSelectionChanged() const {
    if (_graphicsView == nullptr || _propertyBrowser == nullptr) {
        return;
    }

    const QList<QGraphicsItem*> selectedItems = _graphicsView->selectedItems();
    qInfo() << "[PropertyEditorController] sceneSelectionChanged selectedItems=" << selectedItems.size();
    if (selectedItems.size() == 1) {
        QGraphicsItem* item = selectedItems.at(0);
        GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item);
        if (gmc != nullptr) {
            qInfo() << "[PropertyEditorController] sceneSelectionChanged binding single GraphicalModelComponent";
            _propertyBrowser->setActiveObject(
                gmc,
                gmc->getComponent(),
                _propertyGenesys,
                _propertyList,
                _propertyEditorUI,
                _propertyBox);
            return;
        }
    }

    // Clear bindings when none or multiple scene items are selected.
    qInfo() << "[PropertyEditorController] sceneSelectionChanged clearing context for non-single or non-component selection";
    clearPropertyEditorSelection();
    if (_actualizeActions) {
        _actualizeActions();
    }
}

// Preserve the existing post-edit cascade while preventing stale scene pointer reuse.
void PropertyEditorController::onPropertyEditorModelChanged() const {
    qInfo() << "[PropertyEditorController] onPropertyEditorModelChanged enter";
    if (_actualizeModelSimLanguage) {
        _actualizeModelSimLanguage();
    }
    if (_actualizeModelComponents) {
        _actualizeModelComponents(true);
    }
    if (_actualizeModelDataDefinitions) {
        _actualizeModelDataDefinitions(true);
    }
    if (_actualizeModelCppCode) {
        _actualizeModelCppCode();
    }
    if (_createModelImage) {
        _createModelImage();
    }
    if (_actualizeTabPanes) {
        _actualizeTabPanes();
    }

    ModelGraphicsScene* scene = (_graphicsView != nullptr) ? _graphicsView->getScene() : nullptr;
    if (scene == nullptr) {
        qWarning() << "Skipping property-editor scene refresh because scene is null";
        return;
    }

    scene->actualizeDiagramArrows();
    scene->update();

    if (scene->existDiagram()) {
        const bool wasVisible = scene->visibleDiagram();
        scene->destroyDiagram();
        scene->createDiagrams();
        if (wasVisible) {
            scene->showDiagrams();
        } else {
            scene->hideDiagrams();
        }
    }
    qInfo() << "[PropertyEditorController] onPropertyEditorModelChanged exit";
}
