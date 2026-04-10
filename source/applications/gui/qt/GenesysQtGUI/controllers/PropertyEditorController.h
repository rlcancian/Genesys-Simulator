#ifndef PROPERTYEDITORCONTROLLER_H
#define PROPERTYEDITORCONTROLLER_H

#include <functional>
#include <map>

class ObjectPropertyBrowser;
class ModelGraphicsView;
class PropertyEditorGenesys;
class SimulationControl;
class DataComponentProperty;
class DataComponentEditor;
class ComboBoxEnum;

// Document the property-editor controller and post-commit synchronization pipeline.
/**
 * @brief Controller that orchestrates property editor state and scene-selection coupling.
 *
 * This Phase-6 extraction isolates property editor wiring from MainWindow while preserving
 * the existing compatibility façade methods. It coordinates selection-driven property binding
 * and the deferred refresh cascade required after property commits.
 *
 * Responsibilities:
 * - synchronize current scene selection into PropertyEditorGenesys/ObjectPropertyBrowser;
 * - execute post-edit refresh callbacks for model language, trees, C++ export, and tabs;
 * - coalesce heavy refresh requests to avoid re-entrant UI update storms.
 *
 * Boundaries:
 * - it does not own property objects/editors lifetime (maps are injected);
 * - it does not alter undo/redo infrastructure or scene command orchestration;
 * - it does not persist models or execute simulation commands.
 */
class PropertyEditorController {
public:
    // Inject only the property-editor dependencies needed by the Phase 6 flow.
    PropertyEditorController(ObjectPropertyBrowser* propertyBrowser,
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
                             std::function<void()> actualizeActions);

    // Synchronize current scene selection into the property editor safely.
    void sceneSelectionChanged() const;
    // Execute the post-edit model/UI update cascade safely.
    void onPropertyEditorModelChanged() const;
    // Clear property editor selection and bindings defensively.
    void clearPropertyEditorSelection() const;
    // Report whether post-commit stabilization/refresh is still running or queued.
    bool isPostCommitPipelineActive() const;

private:
    // Keep direct access to the property browser used by this controller.
    ObjectPropertyBrowser* _propertyBrowser;
    // Keep direct access to the graphical view used for selection inspection.
    ModelGraphicsView* _graphicsView;
    // Keep direct access to property-editor dependencies required for activation.
    PropertyEditorGenesys* _propertyGenesys;
    std::map<SimulationControl*, DataComponentProperty*>* _propertyList;
    std::map<SimulationControl*, DataComponentEditor*>* _propertyEditorUI;
    std::map<SimulationControl*, ComboBoxEnum*>* _propertyBox;

    // Keep narrow callbacks for MainWindow-owned update methods.
    std::function<void()> _actualizeModelSimLanguage;
    std::function<void(bool)> _actualizeModelComponents;
    std::function<void(bool)> _actualizeModelDataDefinitions;
    std::function<void()> _actualizeModelCppCode;
    std::function<bool()> _createModelImage;
    std::function<void()> _actualizeTabPanes;
    std::function<void()> _actualizeActions;

    // Coalesce and serialize heavy refresh requests after property commits.
    mutable bool _isGlobalRefreshQueued = false;
    mutable bool _isGlobalRefreshRunning = false;
    mutable bool _pendingGlobalRefresh = false;
    // Coalesce deferred selection synchronization while commit/refresh is active.
    mutable bool _isDeferredSelectionSyncScheduled = false;

    void _runGlobalRefresh() const;
};

#endif // PROPERTYEDITORCONTROLLER_H
