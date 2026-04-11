#include "PropertyEditorController.h"
#include "../GuiScopeTrace.h"

#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "propertyeditor/ObjectPropertyBrowser.h"

#include <QGraphicsItem>
#include <QDebug>
#include <QTimer>

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

bool PropertyEditorController::isPostCommitPipelineActive() const {
    const bool propertyEditorBusy = (_propertyBrowser != nullptr) && _propertyBrowser->isCommitPipelineBusy();
    return propertyEditorBusy || _isGlobalRefreshQueued || _isGlobalRefreshRunning || _pendingGlobalRefresh;
}

// Preserve legacy single-selection behavior while moving orchestration out of MainWindow.
void PropertyEditorController::sceneSelectionChanged() const {
    // Adds scoped tracing for property-controller selection synchronization diagnostics.
    const GuiScopeTrace scopeTrace("PropertyEditorController::sceneSelectionChanged", this);
    if (_graphicsView == nullptr || _propertyBrowser == nullptr) {
        return;
    }

    if (isPostCommitPipelineActive()) {
        qInfo() << "[PropertyEditorController] sceneSelectionChanged deferred because post-commit pipeline is active";
        if (!_isDeferredSelectionSyncScheduled) {
            _isDeferredSelectionSyncScheduled = true;
            QTimer::singleShot(0, [this]() {
                _isDeferredSelectionSyncScheduled = false;
                this->sceneSelectionChanged();
            });
        }
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

        // Bind data-definition selection to the same property editor API used for components.
        GraphicalModelDataDefinition* gmdd = dynamic_cast<GraphicalModelDataDefinition*>(item);
        if (gmdd != nullptr) {
            qInfo() << "[PropertyEditorController] sceneSelectionChanged binding single GraphicalModelDataDefinition";
            _propertyBrowser->setActiveObject(
                gmdd,
                gmdd->getDataDefinition(),
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
    qInfo() << "[PropertyEditorController] onPropertyEditorModelChanged request. queued=" << _isGlobalRefreshQueued
            << " running=" << _isGlobalRefreshRunning << " pending=" << _pendingGlobalRefresh;
    _pendingGlobalRefresh = true;
    if (_isGlobalRefreshRunning || _isGlobalRefreshQueued) {
        return;
    }

    _isGlobalRefreshQueued = true;
    QTimer::singleShot(0, [this]() {
        _isGlobalRefreshQueued = false;
        _runGlobalRefresh();
    });
}

void PropertyEditorController::_runGlobalRefresh() const {
    if (_isGlobalRefreshRunning) {
        _pendingGlobalRefresh = true;
        qInfo() << "[PropertyEditorController] _runGlobalRefresh skipped because refresh is already running";
        return;
    }

    if (_propertyBrowser != nullptr && _propertyBrowser->isCommitPipelineBusy()) {
        qInfo() << "[PropertyEditorController] _runGlobalRefresh deferred because property editor is still stabilizing";
        if (!_isGlobalRefreshQueued) {
            _isGlobalRefreshQueued = true;
            QTimer::singleShot(0, [this]() {
                _isGlobalRefreshQueued = false;
                _runGlobalRefresh();
            });
        }
        return;
    }

    _isGlobalRefreshRunning = true;
    do {
        _pendingGlobalRefresh = false;
        qInfo() << "[PropertyEditorController] _runGlobalRefresh enter";

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
            qWarning() << "[PropertyEditorController] Skipping scene refresh because scene is null";
        } else {
            // Refresh diagram arrows in-place without forcing legacy destroy/create diagram regeneration.
            scene->actualizeDiagramArrows();
            scene->update();
        }
        qInfo() << "[PropertyEditorController] _runGlobalRefresh exit";
    } while (_pendingGlobalRefresh);
    _isGlobalRefreshRunning = false;
}
