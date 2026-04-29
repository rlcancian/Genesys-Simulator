#include "PropertyEditorController.h"
#include "../GuiScopeTrace.h"

#include "graphicals/ModelGraphicsView.h"
#include "graphicals/ModelGraphicsScene.h"
#include "graphicals/GraphicalComponentPort.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalDiagramConnection.h"
#include "graphicals/GraphicalModelComponent.h"
#include "graphicals/GraphicalModelDataDefinition.h"
#include "propertyeditor/ObjectPropertyBrowser.h"

#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/GenesysPropertyIntrospection.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ModelManager.h"
#include "kernel/simulator/Simulator.h"

#include <QGraphicsItem>
#include <QDebug>
#include <QSet>
#include <QTimer>

#include <cctype>
#include <set>

namespace {
bool isSceneInfrastructureItem(QGraphicsItem* item) {
    return dynamic_cast<GraphicalComponentPort*>(item) != nullptr
        || dynamic_cast<GraphicalDiagramConnection*>(item) != nullptr
        || dynamic_cast<GraphicalConnection*>(item) != nullptr;
}

QSet<QString> graphicallyRepresentedDataDefinitionNames(ModelGraphicsScene* scene) {
    QSet<QString> names;
    if (scene == nullptr || scene->getAllDataDefinitions() == nullptr) {
        return names;
    }

    for (GraphicalModelDataDefinition* graphicalDataDefinition : *scene->getAllDataDefinitions()) {
        if (graphicalDataDefinition == nullptr || graphicalDataDefinition->getDataDefinition() == nullptr) {
            continue;
        }
        names.insert(QString::fromStdString(graphicalDataDefinition->getDataDefinition()->getName()));
    }
    return names;
}

void collectEditableReferencedDataDefinitions(
    List<SimulationControl*>* controls,
    QSet<QString>& names,
    std::set<const SimulationControl*>& recursionPath,
    int depth = 0
    ) {
    if (controls == nullptr || depth > 10) {
        return;
    }

    for (SimulationControl* control : *controls->list()) {
        if (control == nullptr || recursionPath.find(control) != recursionPath.end()) {
            continue;
        }

        const GenesysPropertyDescriptor desc = GenesysPropertyIntrospection::describe(control);
        if (desc.isModelDataDefinitionReference && !desc.readOnly) {
            ModelDataDefinition* referenced = control->getReferencedModelDataDefinition();
            if (referenced != nullptr) {
                names.insert(QString::fromStdString(referenced->getName()));
            }
        }

        recursionPath.insert(control);
        if (desc.supportsListEditor && desc.isClass) {
            const int itemCount = static_cast<int>(desc.choices.size());
            for (int index = 0; index < itemCount; ++index) {
                collectEditableReferencedDataDefinitions(
                    control->getChildSimulationControls(index),
                    names,
                    recursionPath,
                    depth + 1
                    );
            }
        } else if (desc.supportsInlineExpansion && control->hasObjectInstance()) {
            collectEditableReferencedDataDefinitions(
                control->getChildSimulationControls(),
                names,
                recursionPath,
                depth + 1
                );
        }
        recursionPath.erase(control);
    }
}

bool isPositiveIndexSuffix(const std::string& suffix) {
    if (suffix.empty()) {
        return false;
    }

    for (const char character : suffix) {
        if (!std::isdigit(static_cast<unsigned char>(character))) {
            return false;
        }
    }
    return true;
}

bool attachmentNameMatchesEditableControl(const std::string& attachmentName,
                                          const GenesysPropertyDescriptor& descriptor) {
    if (attachmentName.empty() || descriptor.readOnly || !descriptor.isClass) {
        return false;
    }

    if (attachmentName == descriptor.displayName || attachmentName == descriptor.technicalTypeName) {
        return true;
    }

    if (descriptor.isList
        && !descriptor.technicalTypeName.empty()
        && attachmentName.rfind(descriptor.technicalTypeName, 0) == 0) {
        return isPositiveIndexSuffix(attachmentName.substr(descriptor.technicalTypeName.size()));
    }

    return false;
}

void collectEditableAttachedDataDefinitionNames(ModelComponent* component, QSet<QString>& names) {
    if (component == nullptr
        || component->getSimulationControls() == nullptr
        || component->getAttachedData() == nullptr) {
        return;
    }

    for (SimulationControl* control : *component->getSimulationControls()->list()) {
        const GenesysPropertyDescriptor descriptor = GenesysPropertyIntrospection::describe(control);
        for (const auto& attachedData : *component->getAttachedData()) {
            ModelDataDefinition* dataDefinition = attachedData.second;
            if (dataDefinition == nullptr) {
                continue;
            }
            if (attachmentNameMatchesEditableControl(attachedData.first, descriptor)) {
                names.insert(QString::fromStdString(dataDefinition->getName()));
            }
        }
    }
}

QSet<QString> editableDataDefinitionNames(ModelGraphicsScene* scene) {
    QSet<QString> names;
    if (scene == nullptr || scene->getSimulator() == nullptr ||
        scene->getSimulator()->getModelManager() == nullptr) {
        return names;
    }

    Model* model = scene->getSimulator()->getModelManager()->current();
    if (model == nullptr) {
        return names;
    }

    std::set<const SimulationControl*> recursionPath;
    if (model->getComponentManager() != nullptr) {
        for (ModelComponent* component : *model->getComponentManager()->getAllComponents()) {
            if (component != nullptr) {
                collectEditableReferencedDataDefinitions(
                    component->getSimulationControls(),
                    names,
                    recursionPath
                    );
                collectEditableAttachedDataDefinitionNames(component, names);
            }
        }
    }

    if (model->getDataManager() != nullptr) {
        for (const std::string& dataTypename : model->getDataManager()->getDataDefinitionClassnames()) {
            List<ModelDataDefinition*>* dataDefinitions = model->getDataManager()->getDataDefinitionList(dataTypename);
            if (dataDefinitions == nullptr) {
                continue;
            }
            for (ModelDataDefinition* dataDefinition : *dataDefinitions->list()) {
                if (dataDefinition != nullptr) {
                    collectEditableReferencedDataDefinitions(
                        dataDefinition->getSimulationControls(),
                        names,
                        recursionPath
                        );
                }
            }
        }
    }

    return names;
}

void refreshGraphicalDataDefinitionEditability(
    ModelGraphicsScene* scene,
    const QSet<QString>& editableDataDefinitions
    ) {
    if (scene == nullptr || scene->getAllDataDefinitions() == nullptr) {
        return;
    }

    for (GraphicalModelDataDefinition* graphicalDataDefinition : *scene->getAllDataDefinitions()) {
        if (graphicalDataDefinition == nullptr || graphicalDataDefinition->getDataDefinition() == nullptr) {
            continue;
        }
        const QString name = QString::fromStdString(graphicalDataDefinition->getDataDefinition()->getName());
        graphicalDataDefinition->setEditableInPropertyEditor(editableDataDefinitions.contains(name));
    }
}
} // namespace

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

void PropertyEditorController::bindDataDefinitionFromInspector(
    ModelDataDefinition* dataDefinition,
    QObject* graphicalObject
    ) const {
    if (_propertyBrowser == nullptr) {
        return;
    }

    if (dataDefinition == nullptr) {
        clearPropertyEditorSelection();
        return;
    }

    ModelGraphicsScene* scene = (_graphicsView != nullptr) ? _graphicsView->getScene() : nullptr;
    QSet<QString> graphicalDataDefinitions = graphicallyRepresentedDataDefinitionNames(scene);
    QSet<QString> editableDataDefinitions = editableDataDefinitionNames(scene);

    refreshGraphicalDataDefinitionEditability(scene, editableDataDefinitions);

    _propertyBrowser->setActiveObject(
        graphicalObject,
        dataDefinition,
        graphicalDataDefinitions,
        editableDataDefinitions,
        _propertyGenesys,
        _propertyList,
        _propertyEditorUI,
        _propertyBox);
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
        ModelGraphicsScene* scene = _graphicsView->getScene();
        const QSet<QString> graphicalDataDefinitions = graphicallyRepresentedDataDefinitionNames(scene);
        const QSet<QString> editableDataDefinitions = editableDataDefinitionNames(scene);
        refreshGraphicalDataDefinitionEditability(scene, editableDataDefinitions);

        GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*>(item);
        if (gmc != nullptr) {
            qInfo() << "[PropertyEditorController] sceneSelectionChanged binding single GraphicalModelComponent";
            _propertyBrowser->setActiveObject(
                gmc,
                gmc->getComponent(),
                graphicalDataDefinitions,
                editableDataDefinitions,
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
                graphicalDataDefinitions,
                editableDataDefinitions,
                _propertyGenesys,
                _propertyList,
                _propertyEditorUI,
                _propertyBox);
            return;
        }

        if (item != nullptr
            && item->flags().testFlag(QGraphicsItem::ItemIsSelectable)
            && !isSceneInfrastructureItem(item)) {
            qInfo() << "[PropertyEditorController] sceneSelectionChanged binding single pure graphics item";
            _propertyBrowser->setActiveGraphicsItem(item);
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

        if (_actualizeModelComponents) {
            _actualizeModelComponents(true);
        }
        if (_actualizeModelDataDefinitions) {
            _actualizeModelDataDefinitions(true);
        }
        // Refresh the serialized model text only after component and data-definition panes have
        // consumed the committed kernel state. This keeps Siman aligned with the same model
        // snapshot already visible in the inspector widgets.
        if (_actualizeModelSimLanguage) {
            _actualizeModelSimLanguage();
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
            // Route through scene-owned scheduling to avoid sync during unstable mutation stacks.
            scene->requestGraphicalDataDefinitionsSync();
            scene->update();
        }
        qInfo() << "[PropertyEditorController] _runGlobalRefresh exit";
    } while (_pendingGlobalRefresh);
    _isGlobalRefreshRunning = false;
}
