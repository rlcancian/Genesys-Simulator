#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QGraphicsItem>
#include <QUndoView>
#include <QMetaObject>
#include <memory>
#include <map>

#include "propertyeditor/DataComponentProperty.h"
#include "propertyeditor/DataComponentEditor.h"
#include "propertyeditor/ComboBoxEnum.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PropertyGenesys.h"
#include "kernel/simulator/TraceManager.h"
#include "graphicals/ModelGraphicsScene.h"

QT_BEGIN_NAMESPACE
		namespace Ui {
	class MainWindow;
}

QT_END_NAMESPACE

class ModelLanguageSynchronizer;
class GraphvizModelExporter;
class CppModelExporter;
class GraphicalModelSerializer;
class GraphicalModelBuilder;
class ModelInspectorController;
class TraceConsoleController;
class SimulationEventController;
class PluginCatalogController;
class PropertyEditorController;
class ModelLifecycleController;
class SimulationCommandController;
class EditCommandController;
class SceneToolController;
class GraphicalContextMenuController;
class DialogUtilityController;
class QAction;
class QTabWidget;
class ModelGraphicsView;

// Document MainWindow as composition root and incremental compatibility façade.
/**
 * @brief Main Qt window that composes controllers/services and preserves legacy GUI entry points.
 *
 * MainWindow remains the composition root of GenesysQtGUI: it owns the generated UI, wires
 * extracted controllers/services, and exposes the original Qt slot surface expected by actions,
 * widgets, and kernel callback registration. In the refactored architecture it primarily acts as
 * a compatibility façade that delegates operational responsibilities to focused collaborators.
 *
 * Responsibilities:
 * - bootstrap UI construction and dependency wiring for controllers/services;
 * - host compatibility wrappers used by menus/toolbars/scene callbacks;
 * - coordinate cross-cutting state that still spans multiple controllers.
 *
 * Boundaries:
 * - detailed domain workflows (simulation commands, lifecycle, persistence, scene tools,
 *   property editing, trace/event rendering) are delegated to dedicated controller/service
 *   classes whenever extraction already exists;
 * - MainWindow does not replace kernel ownership semantics.
 */
class MainWindow : public QMainWindow {
	Q_OBJECT

    // Allow the Phase 4 event controller to register private compatibility handlers.
    friend class SimulationEventController;

public:
    /** @brief Builds the composition root, wiring controllers/services and legacy Qt surface. */
	MainWindow(QWidget *parent = nullptr);
    /** @brief Releases UI resources while preserving existing Qt ownership semantics. */
	~MainWindow();
    /** @brief Returns the current model graphics scene used by delegated controllers/services. */
    ModelGraphicsScene* myScene() const;

public: // to notify changes
    /** @brief Indicates whether graphical representation has unsaved modifications. */
	bool graphicalModelHasChanged() const;
    /** @brief Updates graphical-change flag used by persistence/action synchronization. */
    void setGraphicalModelHasChanged(bool graphicalModelHasChanged);
    /** @brief Compatibility wrapper that clears currently selected draw tool actions. */
    void unselectDrawIcons();
    /** @brief Compatibility wrapper that reports whether any draw tool action is selected. */
    bool checkSelectedDrawIcons();

private slots:
    // actions
	void on_actionEditUndo_triggered();
	void on_actionEditRedo_triggered();
	void on_actionEditFind_triggered();
	// void on_actionReplace_triggered(); // old name (without menu namespace)
	void on_actionEditReplace_triggered();
	void on_actionEditCut_triggered();
	void on_actionEditCopy_triggered();
	void on_actionEditPaste_triggered();
	void on_actionEditDelete_triggered();
	void on_actionEditGroup_triggered();
	void on_actionEditUngroup_triggered();
    void on_actionViewGroup_triggered();
    void on_actionViewUngroup_triggered();

    void on_actionShowGrid_triggered();
	void on_actionShowRule_triggered();
	void on_actionShowGuides_triggered();
	void on_actionViewConfigure_triggered();

    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_actionZoom_All_triggered();

	void on_actionDrawLine_triggered();
	void on_actionDrawRectangle_triggered();
	void on_actionDrawEllipse_triggered();
	void on_actionDrawText_triggered();
	void on_actionDrawPoligon_triggered();

	void on_actionAnimateSimulatedTime_triggered();
	void on_actionAnimateVariable_triggered();
	void on_actionAnimateExpression_triggered();
	void on_actionAnimateResource_triggered();
	void on_actionAnimateQueue_triggered();
	void on_actionAnimateStation_triggered();
	void on_actionAnimateCounter_triggered();
	void on_actionAnimateEntity_triggered();
	void on_actionAnimateEvent_triggered();
	void on_actionAnimateAttribute_triggered();
	void on_actionAnimateStatistics_triggered();
	void on_actionAnimatePlot_triggered();

	void on_actionSimulationStop_triggered();
    /** @brief Starts full simulation run after precondition validation. */
	void on_actionSimulationStart_triggered();
    /** @brief Runs a single simulation step after precondition validation. */
	void on_actionSimulationStep_triggered();
    /** @brief Pauses current simulation run when available. */
	void on_actionSimulationPause_triggered();
    /** @brief Resumes paused simulation when available. */
	void on_actionSimulationResume_triggered();
    /** @brief Compatibility slot that delegates simulation configuration flow to lifecycle controller. */
	void on_actionSimulationConfigure_triggered();

	void on_actionAboutAbout_triggered();
	void on_actionAboutLicence_triggered();
	void on_actionAboutGetInvolved_triggered();

	void on_actionAlignMiddle_triggered();
	void on_actionAlignTop_triggered();
	void on_actionAlignRight_triggered();
	void on_actionAlignCenter_triggered();
	void on_actionAlignLeft_triggered();

	void on_actionToolsParserGrammarChecker_triggered();
	// Legacy slot: there is no matching QAction in mainwindow.ui at this moment.
	void on_actionToolsExperimentation_triggered();
	void on_actionToolsOptimizator_triggered();
	void on_actionToolsDataAnalyzer_triggered();

    void on_actionSimulatorsPluginManager_triggered();
	void on_actionSimulatorExit_triggered();
	void on_actionSimulatorPreferences_triggered();

    /** @brief Delegates new-model lifecycle orchestration while preserving legacy Qt action slot. */
	void on_actionModelNew_triggered();
    /** @brief Delegates open-model lifecycle orchestration while preserving legacy Qt action slot. */
	void on_actionModelOpen_triggered();
    /** @brief Delegates save-model lifecycle orchestration while preserving legacy Qt action slot. */
	void on_actionModelSave_triggered();
    /** @brief Delegates close-model lifecycle orchestration while preserving legacy Qt action slot. */
	void on_actionModelClose_triggered();
    /** @brief Delegates model-information dialog flow to lifecycle/dialog compatibility layer. */
	void on_actionModelInformation_triggered();
    /** @brief Delegates model-check workflow used as precondition for simulation/export flows. */
	void on_actionModelCheck_triggered();

	void on_actionGModelComponentBreakpoint_triggered();
	void on_actionShowInternalElements_triggered();
	void on_actionShowEditableElements_triggered();
	void on_actionShowAttachedElements_triggered();
	void on_actionShowRecursiveElements_triggered();


    // widget events
	//void on_textCodeEdit_Model_textChanged();
	void on_tabWidget_Model_tabBarClicked(int index);
	void on_tabWidget_Debug_currentChanged(int index);
	void on_tabWidgetCentral_currentChanged(int index);
	void on_tabWidgetCentral_tabBarClicked(int index);
	void on_tabWidgetModel_currentChanged(int index);
	void on_tabWidgetSimulation_currentChanged(int index);
	void on_tabWidgetReports_currentChanged(int index);
	void on_tabWidgetModelLanguages_currentChanged(int index);

	void on_treeWidget_Plugins_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_treeWidgetComponents_itemSelectionChanged();
    void on_treeWidgetDataDefnitions_itemSelectionChanged();
	void on_treeWidget_Plugins_itemClicked(QTreeWidgetItem *item, int column);

	void on_horizontalSlider_Zoom_valueChanged(int value);
	void on_horizontalSlider_ZoomGraphical_valueChanged(int value);

	void on_checkBox_ShowElements_stateChanged(int arg1);
	void on_checkBox_ShowInternals_stateChanged(int arg1);
	void on_checkBox_ShowEditableElements_stateChanged(int arg1);
	void on_checkBox_ShowRecursive_stateChanged(int arg1);
	void on_checkBox_ShowLevels_stateChanged(int arg1);

	void on_pushButton_Breakpoint_Insert_clicked();
	void on_pushButton_Breakpoint_Remove_clicked();
	void on_pushButton_Export_clicked();

    /** @brief Synchronizes selection/property editor state after rubber-band scene selection updates. */
	void on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);
    /** @brief Marks model-language representation dirty and updates synchronization-related UI state. */
	void on_TextCodeEditor_textChanged();

    void on_treeWidgetDataDefnitions_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_treeWidgetDataDefnitions_itemChanged(QTreeWidgetItem *item, int column);
    void on_actionArranjeLeft_triggered();
    void on_actionArranjeRight_triggered();
    void on_actionArranjeTop_triggered();
    void on_actionArranjeBototm_triggered();
    void on_actionArranjeCenter_triggered();
    void on_actionArranjeMiddle_triggered();
    void on_actionShowSnap_triggered();
    void on_actionGModelShowConnect_triggered(bool checked = false);

    void on_actionActivateGraphicalSimulation_triggered();
    void on_horizontalSliderAnimationSpeed_valueChanged(int value);
    void on_actionSelectAll_triggered();

    void on_actionParallelization_triggered();

    void on_horizontalSlider_ZoomGraphical_actionTriggered(int action);
    /** @brief Compatibility slot bridging property-editor commit notifications to controller pipeline. */
    void _onPropertyEditorModelChanged();

protected:
    void closeEvent(QCloseEvent *event) override;

private: // VIEW

private: // trace handlers
    /** @brief Compatibility façade wrapper that delegates generic trace rendering to controller. */
	void _simulatorTraceHandler(TraceEvent e);
    /** @brief Compatibility façade wrapper that delegates error trace rendering to controller. */
	void _simulatorTraceErrorHandler(TraceErrorEvent e);
    /** @brief Compatibility façade wrapper that delegates simulation trace rendering. */
	void _simulatorTraceSimulationHandler(TraceSimulationEvent e);
    /** @brief Compatibility façade wrapper that delegates reports trace rendering. */
	void _simulatorTraceReportsHandler(TraceEvent e);
private: // simulator event handlers
    /** @brief Compatibility wrapper for model-check success event handling delegation. */
	void _onModelCheckSuccessHandler(ModelEvent* re);
    /** @brief Compatibility wrapper for replication-start event handling delegation. */
	void _onReplicationStartHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for replication-end event handling delegation. */
	void _onReplicationEndHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for simulation-start event handling delegation. */
	void _onSimulationStartHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for simulation-pause event handling delegation. */
	void _onSimulationPausedHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for simulation-resume event handling delegation. */
	void _onSimulationResumeHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for simulation-end event handling delegation. */
	void _onSimulationEndHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for process-event handling delegation. */
	void _onProcessEventHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for entity-create handling delegation. */
	void _onEntityCreateHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for entity-remove handling delegation. */
	void _onEntityRemoveHandler(SimulationEvent* re);
    /** @brief Compatibility wrapper for move-entity animation event delegation. */
    void _onMoveEntityEvent(SimulationEvent * re);
    /** @brief Compatibility wrapper for after-process animation event delegation. */
    void _onAfterProcessEvent(SimulationEvent * re);
private: // model Graphics View handlers
    /** @brief Receives scene mouse callbacks and delegates interaction flow to scene-tool pipeline. */
    void _onSceneMouseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    /** @brief Handles scene wheel-in callbacks to keep zoom wrappers synchronized. */
    void _onSceneWheelInEvent();
    /** @brief Handles scene wheel-out callbacks to keep zoom wrappers synchronized. */
    void _onSceneWheelOutEvent();
    /** @brief Bridges scene graphical-model events to compatibility refresh/update routines. */
    void _onSceneGraphicalModelEvent(const GraphicalModelEvent& event);
private: // QGraphicsScene Slots
    /** @brief Reacts to scene dirty-region changes and updates compatibility state flags. */
	void sceneChanged(const QList<QRectF> &region);
    /** @brief Reacts to focus-item transitions and synchronizes property/editor wrappers. */
	void sceneFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason);
	//void sceneRectChanged(const QRectF &rect);
    /** @brief Reacts to scene selection changes and delegates to property/inspector synchronization. */
	void sceneSelectionChanged();
private: // Similar to QGraphicsScene Slots
    /** @brief Compatibility hook fired when scene model graph changes outside Qt dirty-region notifications. */
	void sceneGraphicalModelChanged();
private: // simulator related
    /** @brief Registers simulator event callbacks while preserving private wrapper surface. */
	void _setOnEventHandlers();
    /** @brief Guards command execution with model-check and text/model synchronization preconditions. */
    bool _ensureSimulationReady(bool checkModel = true);
    /** @brief Returns whether current model exposes a valid ModelSimulation instance. */
    bool _hasCurrentModelSimulation() const;
    /** @brief Compatibility wrapper delegating plugin-tree insertion to extracted controller. */
	void _insertPluginUI(Plugin* plugin);
    /** @brief Compatibility wrapper delegating fake-plugin insertion to extracted controller. */
	void _insertFakePlugins();
    /** @brief Compatibility wrapper delegating text-to-model synchronization to service. */
	bool _setSimulationModelBasedOnText();
    /** @brief Compatibility wrapper delegating Graphviz image generation to service. */
	bool _createModelImage();
    /** @brief Compatibility wrapper delegating DOT name normalization to service. */
	std::string _adjustDotName(std::string name);
    /** @brief Compatibility wrapper delegating DOT fragment insertion to service. */
	void _insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode = false);
    /** @brief Compatibility wrapper delegating recursive DOT generation to service. */
	void _recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap);
    /** @brief Compatibility wrapper delegating C++ representation refresh to service. */
	void _actualizeModelCppCode();
    /** @brief Compatibility wrapper delegating C++ line formatting to service. */
	std::string _addCppCodeLine(std::string line, unsigned int indent = 0);
private: // view
    /** @brief Initializes scene/view widgets and core scene callback wiring. */
	void _initModelGraphicsView();
    /** @brief Connects scene signals using stored QMetaObject connections for lifecycle control. */
    void _connectSceneSignals();
    /** @brief Disconnects scene signals during lifecycle transitions and shutdown. */
    void _disconnectSceneSignals(const char* context);
    /** @brief Creates the graphical-model tab host and moves the initial view into the first page. */
    void _initializeModelTabs();
    /** @brief Applies simulator/property-editor dependencies to a graphics view and its scene. */
    void _configureModelGraphicsView(ModelGraphicsView* graphicsView);
    /** @brief Creates a configured graphics view for one opened kernel model tab. */
    ModelGraphicsView* _createModelGraphicsView(QWidget* parent);
    /** @brief Makes a graphics view the active compatibility target used by legacy wrappers. */
    void _activateModelGraphicsView(ModelGraphicsView* graphicsView, bool rebuildControllers = true);
    /** @brief Ensures that an opened kernel model has a corresponding graphical tab. */
    ModelGraphicsView* _ensureModelTab(Model* model);
    /** @brief Removes the tab and graphical view associated with a model no longer opened. */
    void _removeModelTab(Model* model);
    /** @brief Updates graphical model tab labels and navigation actions from ModelManager state. */
    void _updateModelTabs();
    /** @brief Changes the current model when the user selects a graphical model tab. */
    void _activateModelTab(int index);
    /** @brief Moves to the previous opened model and selects its tab. */
    void _activatePreviousModel();
    /** @brief Moves to the next opened model and selects its tab. */
    void _activateNextModel();
    /** @brief Rebuilds controllers/services that keep direct pointers to the active graphics view. */
    void _rebuildViewDependentControllers();
    /** @brief Reinitializes UI state after new/load model lifecycle operations. */
	void _initUiForNewModel(Model* m);
    /** @brief Recomputes action enabled/checked states from delegated controller/service state. */
	void _actualizeActions();
    /** @brief Refreshes undo view/actions after scene command changes. */
    void _actualizeUndo();
    /** @brief Synchronizes tab panes that depend on simulation/model representation updates. */
	void _actualizeTabPanes();
    /** @brief Compatibility wrapper delegating model-language text refresh to service. */
	void _actualizeModelSimLanguage();
    /** @brief Updates text-change flags and related UI hints. */
	void _actualizeModelTextHasChanged(bool hasChanged);
    /** @brief Refreshes simulation-events pane based on latest event payload. */
	void _actualizeSimulationEvents(SimulationEvent * re);
    /** @brief Refreshes debug variables pane while preserving force-update semantics. */
	void _actualizeDebugVariables(bool force);
    /** @brief Refreshes debug entities pane while preserving force-update semantics. */
	void _actualizeDebugEntities(bool force);
    /** @brief Refreshes debug breakpoints pane while preserving force-update semantics. */
	void _actualizeDebugBreakpoints(bool force);
    /** @brief Configures and clears the simulation results table widget headers and behavior. */
    void _prepareReportsResultsTable();
    /** @brief Clears all rows from the simulation results table widget. */
    void _clearReportsResultsTable();
    /** @brief Fills the simulation results table with final aggregated simulation statistics. */
    void _actualizeReportsResultsTable();
    /** @brief Configures the simulation results plot pane. */
    void _prepareReportsPlots();
    /** @brief Clears all widgets from the simulation results plot pane. */
    void _clearReportsPlots();
    /** @brief Fills the simulation results plot pane with grouped aggregate charts. */
    void _actualizeReportsPlots();
    /** @brief Emits a replication text report from the GUI when the model suppresses it. */
    void _showReplicationReportIfSuppressed();
    /** @brief Emits a simulation text report from the GUI when the model suppresses it. */
    void _showSimulationReportIfSuppressed();
    /** @brief Compatibility wrapper delegating model-components tree synchronization. */
	void _actualizeModelComponents(bool force);
    /** @brief Compatibility wrapper delegating data-definitions tree synchronization. */
	void _actualizeModelDataDefinitions(bool force);
    /** @brief Refreshes graphical animation state based on simulation event progression. */
	void _actualizeGraphicalModel(SimulationEvent * re);
    /** @brief Appends command text to console, preserving legacy command-log formatting. */
	void _insertCommandInConsole(std::string text);
    /** @brief Clears model editors during lifecycle reset and persistence transitions. */
	void _clearModelEditors();
    /** @brief Applies smooth zoom transformation used by slider/action wrappers. */
	void _gentle_zoom(double factor);
    /** @brief Shows compatibility placeholder message for not-yet-implemented utilities. */
	void _showMessageNotImplemented();
    /** @brief Compatibility wrapper delegating recursive graphical rebuild to service. */
    void _recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInline);
    /** @brief Compatibility wrapper delegating full graphical model rebuild to service. */
	void _generateGraphicalModelFromModel();
    /** @brief Compatibility wrapper delegating clipboard selection filtering to controller. */
    void saveItemForCopy(QList<GraphicalModelComponent*> * gmcList, QList<GraphicalConnection*> * connList);
	//bool _checkStartSimulation();
private: // opened-model document state
    /** @brief Stores the active editor/dirty state before switching away from the current model. */
    void _syncCurrentModelDocumentState();
    /** @brief Restores filename, editor text and dirty flags for the given model after activation. */
    void _restoreModelDocumentState(Model* model);
    /** @brief Returns a user-facing name for prompts and tab titles without changing model state. */
    QString _modelDisplayName(Model* model) const;
    /** @brief Normalizes .gen/.gui filenames to the shared save basename used by both serializers. */
    QString _modelSaveBaseFilename(QString filename) const;
    /** @brief Returns whether a specific opened model has pending text, graphical or kernel changes. */
    bool _modelHasPendingChanges(Model* model) const;
    /** @brief Saves the current model text and graphical representation using a per-model filename. */
    bool _saveCurrentModel(bool promptForFilename = true);
    /** @brief Saves a model after activating its tab, used by close/exit workflows. */
    bool _saveModelWithActivation(Model* model);
    /** @brief Confirms and optionally saves one model before closing/removing it. */
    bool _confirmCloseModel(Model* model);
    /** @brief Closes one opened model tab and removes its kernel model when confirmed. */
    bool _closeModel(Model* model);
    /** @brief Clears graphical scene state for a view that is about to be closed. */
    void _clearModelGraphicsViewForClose(ModelGraphicsView* graphicsView);
private: // graphical model persistence
    /** @brief Compatibility wrapper delegating graphical model persistence to service. */
    bool _saveGraphicalModel(QString filename);
    /** @brief Compatibility wrapper delegating text model persistence to service. */
    bool _saveTextModel(QFile *saveFile, QString data);
    /** @brief Compatibility wrapper delegating model loading and restoration to service. */
	Model* _loadGraphicalModel(std::string filename);
private: //???
    /** @brief Compatibility wrapper delegating deep-copy helper behavior to edit controller. */
	void _helpCopy();
private: // interface and model main elements to join
	Ui::MainWindow *ui;
	Simulator* simulator;
    // Hosts one graphical scene/view per opened kernel model.
    QTabWidget* _modelGraphicsTabs = nullptr;
    // Maps opened kernel models to their graphical view so tab switching preserves each scene.
    std::map<Model*, ModelGraphicsView*> _modelGraphicsViews;
    // Reverse lookup used by tab activation without relying on QVariant pointer casts.
    std::map<ModelGraphicsView*, Model*> _modelsByGraphicsView;
    // Keeps each opened model's last graphical/text filename independent while switching tabs.
    std::map<Model*, QString> _modelFilenames;
    // Caches the editable Simulang text per opened model so tab switches do not discard unsaved text.
    std::map<Model*, QString> _modelTextContents;
    // Tracks text-editor dirty state per opened model while legacy flags mirror only the active model.
    std::map<Model*, bool> _modelTextHasChanged;
    // Tracks graphical dirty state per opened model while legacy flags mirror only the active model.
    std::map<Model*, bool> _modelGraphicalHasChanged;
    // Tracks tabs being changed programmatically to avoid re-entrant model switching.
    bool _changingModelTabProgrammatically = false;
    // Runtime menu actions for navigating the opened-model list.
    QAction* _actionModelPrevious = nullptr;
    QAction* _actionModelNext = nullptr;
    // Keep core simulation command gateway owned by MainWindow composition root.
    std::unique_ptr<class SimulationController> _simulationController;
    // Keep phase-ordered services to make composition dependencies explicit in Phase 12.
    // Synchronize textual model language with the kernel model manager.
    std::unique_ptr<ModelLanguageSynchronizer> _modelLanguageSynchronizer;
    // Generate Graphviz DOT and rendered model images for the GUI pane.
    std::unique_ptr<GraphvizModelExporter> _graphvizModelExporter;
    // Generate C++ model representation text for the code viewer pane.
    std::unique_ptr<CppModelExporter> _cppModelExporter;
    // Persist and restore graphical .gui state through the Phase 2 serializer service.
    std::unique_ptr<GraphicalModelSerializer> _graphicalModelSerializer;
    // Rebuild graphical components and links through the Phase 2 builder service.
    std::unique_ptr<GraphicalModelBuilder> _graphicalModelBuilder;
    // Keep phase-ordered controllers to preserve the final compatibility façade surface.
    // Add the Phase 3 model-inspector controller owned by MainWindow.
    std::unique_ptr<ModelInspectorController> _modelInspectorController;
    // Add the Phase 4 trace controller owned by MainWindow.
    std::unique_ptr<TraceConsoleController> _traceConsoleController;
    // Add the Phase 4 simulation-event controller owned by MainWindow.
    std::unique_ptr<SimulationEventController> _simulationEventController;
    // Add the Phase 5 plugin-catalog controller owned by MainWindow.
    std::unique_ptr<PluginCatalogController> _pluginCatalogController;
    // Add the Phase 6 property-editor controller owned by MainWindow.
    std::unique_ptr<PropertyEditorController> _propertyEditorController;
    // Add the Phase 7 model-lifecycle controller owned by MainWindow.
    std::unique_ptr<ModelLifecycleController> _modelLifecycleController;
    // Add the Phase 8 simulation-command controller owned by MainWindow.
    std::unique_ptr<SimulationCommandController> _simulationCommandController;
    // Add the Phase 9 edit-command controller owned by MainWindow.
    std::unique_ptr<EditCommandController> _editCommandController;
    // Add the Phase 10 scene-tool controller owned by MainWindow.
    std::unique_ptr<SceneToolController> _sceneToolController;
    // Add the graphical context-menu controller owned by MainWindow.
    std::unique_ptr<GraphicalContextMenuController> _graphicalContextMenuController;
    // Add the Phase 11 dialog-utility controller owned by MainWindow.
    std::unique_ptr<DialogUtilityController> _dialogUtilityController;
	PropertyEditorGenesys* propertyGenesys;
    std::map<SimulationControl*, DataComponentProperty*>* propertyList;
    std::map<SimulationControl*, DataComponentEditor*>* propertyEditorUI;
    std::map<SimulationControl*, ComboBoxEnum*>* propertyBox;
private: // attributes to be saved and loaded withing the graphical model
	int _zoomValue; // todo should be set for each open graphical model, such as view rect, etc
private: // misc useful
    /** @brief Helper for legacy bool-check chaining used in compatibility flows. */
    bool _check(bool success = true);
    /** @brief Delegates pending-change calculation to lifecycle controller compatibility flow. */
	bool _hasPendingModelChanges() const;
    /** @brief Delegates application-exit confirmation to lifecycle controller flow. */
	bool _confirmApplicationExit();
	bool _textModelHasChanged = false;
	bool _graphicalModelHasChanged = false;
	bool _modelWasOpened = false;
	// Avoids duplicated prompts when exit is approved and Qt emits close events during shutdown.
	bool _closingApproved = false;
	QString _autoLoadPluginsFilename = "autoloadplugins.txt";
    /** @brief Validates scene item invariants before executing scene-dependent commands. */
    bool _checkItemsScene();
	QString _modelfilename;
	std::map<std::string /*category*/,QColor>* _pluginCategoryColor = new std::map<std::string,QColor>();
    // TODO 1: Faz parte do mecanismo de restaurar dataDefinitions deletados do modelo e que são restaurados com um Control Z
    // Caso: Ao adicionar um Create no modelo e dar um check() o EntityType será criado,
    // mas ao deletar o Create, dar outro check() e em sequida dar um Control Z (voltando o Create no modelo) e checar novamente, o EntityType não é restaurado
    // na lista de dataDefinitions do modelo, apesar de ainda existir no Create.
    // Neste caso, ocorre um erro acusando que o EntityType do Create não está no modelo.
    // Isso se dá ao fato de que no Kernel ele verifica se _entityType = nullptr para criar um novo e reinserir no modelo, mas não trata o caso dele não ser nullptr
    // e não estar nos dataDefinitions do modelo, que é o que ocorre na situação que foi descrita.
    // Esta função foi feita para tratar esse caso, assim como é feito com insertRestoredDataDefinitions e saveDataDefinitions em ModelGraphicScene
    void setStatisticsCollector();

    bool _cut;
    QList<GraphicalModelComponent*> * _gmc_copies  = new QList<GraphicalModelComponent*>();
    QList<GraphicalConnection*> * _ports_copies = new QList<GraphicalConnection*>();
    QList<QGraphicsItem *>  *_draw_copy = new QList<QGraphicsItem *>();
    QList<QGraphicsItemGroup *>  *_group_copy = new QList<QGraphicsItemGroup *>();

    struct COPY {
        GraphicalModelComponent * old;
        GraphicalModelComponent * copy;
    };

    bool _firstClickShowConnection = true;

private:

	const struct TABINDEXES_STRUC {
		const int TabCentralModelIndex = 0;
		const int TabCentralSimulationIndex = 1;
		const int TabCentralReportsIndex = 2;
		//
		const int TabModelSimLangIndex = 0;
		const int TabModelCppCodeIndex = 1;
		const int TabModelDiagramIndex = 2;
		const int TabModelComponentsIndex = 3;
		//
		const int TabModelDataDefinitionsIndex = 4;
		const int TabSimulationBreakpointsIndex = 0;
		const int TabSimulationVariablesIndex = 1;
		const int TabSimulationEntitiesIndex = 2;
		const int TabSimulationTracesIndex = 3;
		const int TabSimulationEventsIndex = 4;
		//
		const int TabReportReportIndex = 0;
		const int TabReportResultIndex = 1;
		const int TabReportPlotIndex = 2;
	} CONST;

    QUndoView *undoView = nullptr;
    bool _graphicalSimulation = false;
    bool _modelCheked = false;
    bool _loaded = false;
    bool _shuttingDown = false;
    // Persists lightweight tool settings introduced in Phase 5 dialogs.
    double _optimizerPrecision = 1e-6;
    unsigned int _optimizerMaxSteps = 1000;
    // Persists minimal parallel execution preferences configured in GUI.
    bool _parallelizationEnabled = false;
    int _parallelizationThreads = 1;
    int _parallelizationBatchSize = 100;
    // Remembers the last dataset location used by the data analyzer workflow.
    QString _lastDataAnalyzerPath;
    QMetaObject::Connection _sceneChangedConnection;
    QMetaObject::Connection _sceneFocusItemChangedConnection;
    QMetaObject::Connection _sceneSelectionChangedConnection;
    bool _isDeferredPropertyEditorModelChangedScheduled = false;
	//CodeEditor* textCodeEdit_Model;
};
#endif // MAINWINDOW_H
