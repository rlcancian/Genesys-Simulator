#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QGraphicsItem>
#include <QUndoView>

#include "propertyeditor/DataComponentProperty.h"
#include "propertyeditor/DataComponentEditor.h"
#include "propertyeditor/ComboBoxEnum.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/PropertyGenesys.h"
#include "../../../../kernel/simulator/TraceManager.h"
#include "graphicals/ModelGraphicsScene.h"

QT_BEGIN_NAMESPACE
		namespace Ui {
	class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
    ModelGraphicsScene* myScene() const;

public: // to notify changes
	bool graphicalModelHasChanged() const;
    void setGraphicalModelHasChanged(bool graphicalModelHasChanged);
    void unselectDrawIcons();
    bool checkSelectedDrawIcons();

private slots:
    // actions
	void on_actionEditUndo_triggered();
	void on_actionEditRedo_triggered();
	void on_actionEditFind_triggered();
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
	void on_actionSimulationStart_triggered();
	void on_actionSimulationStep_triggered();
	void on_actionSimulationPause_triggered();
	void on_actionSimulationResume_triggered();
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
	void on_actionToolsExperimentation_triggered();
	void on_actionToolsOptimizator_triggered();
	void on_actionToolsDataAnalyzer_triggered();

    void on_actionSimulatorsPluginManager_triggered();
	void on_actionSimulatorExit_triggered();
	void on_actionSimulatorPreferences_triggered();

	void on_actionModelNew_triggered();
	void on_actionModelOpen_triggered();
	void on_actionModelSave_triggered();
	void on_actionModelClose_triggered();
	void on_actionModelInformation_triggered();
	void on_actionModelCheck_triggered();

	void on_actionConnect_triggered();
	void on_actionComponent_Breakpoint_triggered();


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
	void on_treeWidget_Plugins_itemClicked(QTreeWidgetItem *item, int column);

	void on_horizontalSlider_Zoom_valueChanged(int value);
	void on_horizontalSlider_ZoomGraphical_valueChanged(int value);

	void on_checkBox_ShowElements_stateChanged(int arg1);
	void on_checkBox_ShowInternals_stateChanged(int arg1);
	void on_checkBox_ShowRecursive_stateChanged(int arg1);
	void on_checkBox_ShowLevels_stateChanged(int arg1);

	void on_pushButton_Breakpoint_Insert_clicked();
	void on_pushButton_Breakpoint_Remove_clicked();
	void on_pushButton_Export_clicked();

	void on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);
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
    void on_actionGModelShowConnect_triggered();

    void on_actionActivateGraphicalSimulation_triggered();
    void on_horizontalSliderAnimationSpeed_valueChanged(int value);
    void on_actionSelectAll_triggered();

    void on_actionDiagrams_triggered();

    void on_actionParallelization_triggered();

    void on_horizontalSlider_ZoomGraphical_actionTriggered(int action);

protected:
    void closeEvent(QCloseEvent *event) override;

private: // VIEW

private: // trace handlers
	void _simulatorTraceHandler(TraceEvent e);
	void _simulatorTraceErrorHandler(TraceErrorEvent e);
	void _simulatorTraceSimulationHandler(TraceSimulationEvent e);
	void _simulatorTraceReportsHandler(TraceEvent e);
private: // simulator event handlers
	void _onModelCheckSuccessHandler(ModelEvent* re);
	void _onReplicationStartHandler(SimulationEvent* re);
	void _onSimulationStartHandler(SimulationEvent* re);
	void _onSimulationPausedHandler(SimulationEvent* re);
	void _onSimulationResumeHandler(SimulationEvent* re);
	void _onSimulationEndHandler(SimulationEvent* re);
	void _onProcessEventHandler(SimulationEvent* re);
	void _onEntityCreateHandler(SimulationEvent* re);
	void _onEntityRemoveHandler(SimulationEvent* re);
    void _onMoveEntityEvent(SimulationEvent * re);
    void _onAfterProcessEvent(SimulationEvent * re);
private: // model Graphics View handlers
    void _onSceneMouseEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void _onSceneWheelInEvent();
    void _onSceneWheelOutEvent();
    void _onSceneGraphicalModelEvent(GraphicalModelEvent* event);
private: // QGraphicsScene Slots
	void sceneChanged(const QList<QRectF> &region);
	void sceneFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason);
	//void sceneRectChanged(const QRectF &rect);
	void sceneSelectionChanged();
private: // Similar to QGraphicsScene Slots
	void sceneGraphicalModelChanged();
private: // simulator related
	void _setOnEventHandlers();
	void _insertPluginUI(Plugin* plugin);
	void _insertFakePlugins();
	bool _setSimulationModelBasedOnText();
	bool _createModelImage();
	std::string _adjustDotName(std::string name);
	void _insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode = false);
	void _recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap);
	void _actualizeModelCppCode();
	std::string _addCppCodeLine(std::string line, unsigned int indent = 0);
private: // view
	void _initModelGraphicsView();
	void _initUiForNewModel(Model* m);
	void _actualizeActions();
    void _actualizeUndo();
	void _actualizeTabPanes();
	void _actualizeModelSimLanguage();
	void _actualizeModelTextHasChanged(bool hasChanged);
	void _actualizeSimulationEvents(SimulationEvent * re);
	void _actualizeDebugVariables(bool force);
	void _actualizeDebugEntities(bool force);
	void _actualizeDebugBreakpoints(bool force);
	void _actualizeModelComponents(bool force);
	void _actualizeModelDataDefinitions(bool force);
	void _actualizeGraphicalModel(SimulationEvent * re);
	void _insertCommandInConsole(std::string text);
	void _clearModelEditors();
	void _gentle_zoom(double factor);
	void _showMessageNotImplemented();
    void _recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInline);
	void _generateGraphicalModelFromModel();
    void saveItemForCopy(QList<GraphicalModelComponent*> * gmcList, QList<GraphicalConnection*> * connList);
	//bool _checkStartSimulation();
private: // graphical model persistence
    bool _saveGraphicalModel(QString filename);
    bool _saveTextModel(QFile *saveFile, QString data);
	Model* _loadGraphicalModel(std::string filename);
private: //???
	void _helpCopy();
private: // interface and model main elements to join
	Ui::MainWindow *ui;
	Simulator* simulator;
	PropertyEditorGenesys* propertyGenesys;
    std::map<SimulationControl*, DataComponentProperty*>* propertyList;
    std::map<SimulationControl*, DataComponentEditor*>* propertyEditorUI;
    std::map<SimulationControl*, ComboBoxEnum*>* propertyBox;
private: // attributes to be saved and loaded withing the graphical model
	int _zoomValue; // todo should be set for each open graphical model, such as view rect, etc
private: // misc useful
    bool _check(bool success = true);
	bool _textModelHasChanged = false;
	bool _graphicalModelHasChanged = false;
	bool _modelWasOpened = false;
	QString _autoLoadPluginsFilename = "autoloadplugins.txt";
    bool _checkItemsScene();
	QString _modelfilename;
	std::map<std::string /*category*/,QColor>* _pluginCategoryColor = new std::map<std::string,QColor>();
	QColor myrgba(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
    static std::string dotColor(uint64_t color); // TODO: Should NOT be here, but in UtilGUI.h, but then it generates multiple definitions error
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
	//CodeEditor* textCodeEdit_Model;
};
#endif // MAINWINDOW_H
