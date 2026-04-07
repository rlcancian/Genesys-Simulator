#include "mainwindow.h"
#include "ui_mainwindow.h"

// Dialogs
// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../TraitsApp.h"
// GUI
#include "graphicals/ModelGraphicsScene.h"
#include "TraitsGUI.h"
#include "graphicals/GraphicalConnection.h"
#include "controllers/SimulationController.h"
// Add dedicated controllers progressively during incremental GUI refactors.
#include "controllers/ModelInspectorController.h"
#include "controllers/TraceConsoleController.h"
#include "controllers/SimulationEventController.h"
// Add Phase 5 controller include for plugin-catalog responsibilities.
#include "controllers/PluginCatalogController.h"
// Add Phase 6 controller include for property-editor and scene-selection orchestration.
#include "controllers/PropertyEditorController.h"
#include "services/ModelLanguageSynchronizer.h"
#include "services/GraphvizModelExporter.h"
#include "services/CppModelExporter.h"
#include "services/GraphicalModelSerializer.h"
#include "services/GraphicalModelBuilder.h"
#include "UtilGUI.h"
// PropEditor
#include "propertyeditor/qtpropertybrowser/qttreepropertybrowser.h"
#include "animations/AnimationVariable.h"
#include "systempreferences.h"
//#include "actions/PasteUndoCommand.h"
//#include "actions/DeleteUndoCommand.h"
// @TODO: Should NOT be hardcoded!!! (Used to visualize variables)
#include "../../../../plugins/data/Variable.h"
// std
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
//#include <streambuf>
// QT
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
// #include <qt5/QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsitem.h>
#include <QGraphicsScene>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QAction>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // Keep plugins tree as drag source only (never a drop target).
    ui->treeWidget_Plugins->setDragDropMode(QAbstractItemView::DragOnly);
    ui->treeWidget_Plugins->setAcceptDrops(false);
    ui->treeWidget_Plugins->viewport()->setAcceptDrops(false);
    ui->treeWidget_Plugins->setDropIndicatorShown(false);
    //
    // Genesys Simulator
    simulator = new Simulator();
    _simulationController = std::make_unique<SimulationController>(this, simulator);
    // This block initializes phase-1 service objects used for progressive delegation from MainWindow.
    _modelLanguageSynchronizer = std::make_unique<ModelLanguageSynchronizer>(simulator, ui->TextCodeEditor, &_textModelHasChanged, this, [this]() {
        // Keep event-handler ownership in MainWindow while delegating model-language synchronization.
        _setOnEventHandlers();
    });
    // Keep Graphviz exporter dependencies explicit to avoid broad MainWindow coupling.
    _graphvizModelExporter = std::make_unique<GraphvizModelExporter>(simulator,
                                                                     ui->label_ModelGraphic,
                                                                     ui->checkBox_ShowInternals,
                                                                     ui->checkBox_ShowElements,
                                                                     ui->checkBox_ShowRecursive,
                                                                     ui->checkBox_ShowLevels,
                                                                     // Keep synchronization behavior unchanged via a narrow callback dependency.
                                                                     [this]() { return this->_setSimulationModelBasedOnText(); });
    _cppModelExporter = std::make_unique<CppModelExporter>(simulator, ui->plainTextEditCppCode);
    simulator->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    simulator->getTraceManager()->addTraceHandler<MainWindow>(this, &MainWindow::_simulatorTraceHandler);
    simulator->getTraceManager()->addTraceErrorHandler<MainWindow>(this, &MainWindow::_simulatorTraceErrorHandler);
    simulator->getTraceManager()->addTraceReportHandler<MainWindow>(this, &MainWindow::_simulatorTraceReportsHandler);
    simulator->getTraceManager()->addTraceSimulationHandler<MainWindow>(this, &MainWindow::_simulatorTraceSimulationHandler);

	propertyGenesys = new PropertyEditorGenesys();
    propertyList = new std::map<SimulationControl*, DataComponentProperty*>();
    propertyEditorUI = new std::map<SimulationControl*, DataComponentEditor*>();
    propertyBox = new std::map<SimulationControl*, ComboBoxEnum*>();

    //
    // Docks //@TODO how place them in a specified rank?
    //
    //ui->dockWidgetPlugins->doc
    //ui->dockWidgetContentsPlugin->setMinimumHeight(250);
    //ui->dockWidgetContentsPlugin->setMaximumWidth(230);
    //UNCOMMENT//  tabifyDockWidget(ui->dockWidgetConsole, ui->dockWidgetPropertyEditor);
    //
    // Docks //@TODO Trying again to set some of them to minimum height
    //
    ui->dockWidgetConsole->setMinimumHeight(100);
    QSizePolicy policy = ui->dockWidgetConsole->sizePolicy();
    policy.setVerticalPolicy(QSizePolicy::Minimum);
    ui->dockWidgetConsole->setSizePolicy(policy);
    //...
    // plugins
    ui->treeWidget_Plugins->sortByColumn(0, Qt::AscendingOrder);
    // Text Code Editor // @todo No need for programming
    //QVBoxLayout* layout = dynamic_cast<QVBoxLayout*> (ui->tabModelText->layout());
    //ui->TextCodeEditor = new CodeEditor(ui->tabModelText);
    //layout->addWidget(ui->TextCodeEditor);
    //connect(ui->TextCodeEditor, SIGNAL(textChanged()), this, SLOT(on_ui->TextCodeEditor_textChanged()));
    //
    // Tables
    QStringList headers;
    headers << tr("Time") << tr("Component") << tr("Entity");
    ui->tableWidget_Simulation_Event->setHorizontalHeaderLabels(headers);
    ui->tableWidget_Simulation_Event->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    headers.clear();
    headers << tr("Enabled") << tr("Based On") << tr("Break in Value");
    ui->tableWidget_Breakpoints->setHorizontalHeaderLabels(headers);
    ui->tableWidget_Breakpoints->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    headers.clear();
    headers << tr("Name") << tr("Dimentions") << tr("Values");
    ui->tableWidget_Variables->setHorizontalHeaderLabels(headers);
    ui->tableWidget_Variables->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    headers.clear();
    headers << tr("Number") << tr("Name") << tr("Type"); // << and each attribute as a column
    ui->tableWidget_Entities->setHorizontalHeaderLabels(headers);
    ui->tableWidget_Entities->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_Simulation_Event->setContentsMargins(1, 0, 1, 0);
    //
    // Trees
    QTreeWidgetItem *treeHeader = ui->treeWidgetComponents->headerItem();
    treeHeader->setText(0, "Id");
    treeHeader->setText(1, "Type");
    treeHeader->setText(2, "Name");
    treeHeader->setText(3, "Properties");
    treeHeader->setExpanded(true);
    treeHeader = ui->treeWidgetDataDefnitions->headerItem();
    treeHeader->setText(0, "Id");
    treeHeader->setText(1, "Type");
    treeHeader->setText(2, "Name");
    treeHeader->setText(3, "Properties");
    treeHeader->setExpanded(true);
    //
    // ModelGraphic
    ui->graphicsView->setParentWidget(ui->centralwidget);
    ui->graphicsView->setSimulator(simulator);
	ui->graphicsView->setPropertyEditor(propertyGenesys);
    ui->graphicsView->setPropertyList(propertyList);
    ui->graphicsView->setPropertyEditorUI(propertyEditorUI);
    ui->graphicsView->setComboBox(propertyBox);
    _zoomValue = ui->horizontalSlider_ZoomGraphical->maximum() / 2;
    //
    // set current tabs
    ui->tabWidgetCentral->setCurrentIndex(CONST.TabCentralModelIndex);
    ui->tabWidgetModel->setCurrentIndex(CONST.TabModelSimLangIndex);
    ui->tabWidgetSimulation->setCurrentIndex(CONST.TabSimulationBreakpointsIndex);
    ui->tabWidgetReports->setCurrentIndex(CONST.TabReportReportIndex);
    //
    // adjust toolbars position and ranking
    //
    addToolBar(Qt::LeftToolBarArea, ui->toolBarModel);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarEdit);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarView);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarArranje);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarDraw);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarSimulation);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarGraphicalModel);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarAnimate);
    addToolBar(Qt::LeftToolBarArea, ui->toolBarAbout);
    //
    addToolBar(Qt::TopToolBarArea, ui->toolBarModel);
    addToolBar(Qt::TopToolBarArea, ui->toolBarEdit);
    addToolBar(Qt::TopToolBarArea, ui->toolBarView);
    addToolBar(Qt::TopToolBarArea, ui->toolBarArranje);
    //addToolBar(Qt::LeftToolBarArea, ui->toolBarDraw);
    addToolBar(Qt::TopToolBarArea, ui->toolBarSimulation);
    addToolBar(Qt::TopToolBarArea, ui->toolBarGraphicalModel);
    //addToolBar(Qt::LeftToolBarArea, ui->toolBarAnimate);
    addToolBar(Qt::TopToolBarArea, ui->toolBarAbout);
    //
    // graphicsView
    _initModelGraphicsView();
    // Initialize the Phase 3 model-inspector controller after view and simulator dependencies are ready.
    _modelInspectorController = std::make_unique<ModelInspectorController>(simulator,
                                                                           ui->treeWidgetComponents,
                                                                           ui->treeWidgetDataDefnitions,
                                                                           ui->graphicsView);
    // Initialize the Phase 4 trace controller after trace output widgets are available.
    _traceConsoleController = std::make_unique<TraceConsoleController>(ui->textEdit_Console,
                                                                        ui->textEdit_Simulation,
                                                                        ui->textEdit_Reports);
    // Initialize the Phase 4 simulation-event controller after simulator and scene dependencies are available.
    _simulationEventController = std::make_unique<SimulationEventController>(
        simulator,
        ui->graphicsView->getScene(),
        ui->graphicsView,
        ui->label_ReplicationNum,
        ui->progressBarSimulation,
        ui->tableWidget_Simulation_Event,
        ui->tableWidget_Entities,
        ui->tableWidget_Variables,
        ui->textEdit_Simulation,
        ui->textEdit_Reports,
        ui->tabWidgetCentral,
        ui->actionActivateGraphicalSimulation,
        &_modelCheked,
        CONST.TabCentralReportsIndex,
        SimulationEventController::Callbacks{
            [this]() { _actualizeActions(); },
            [this](SimulationEvent* re) { _actualizeSimulationEvents(re); },
            [this](bool force) { _actualizeDebugEntities(force); },
            [this](bool force) { _actualizeDebugVariables(force); },
            [this](SimulationEvent* re) { _actualizeGraphicalModel(re); }});
    // Initialize the Phase 5 plugin-catalog controller after simulator and plugin-tree dependencies are ready.
    _pluginCatalogController = std::make_unique<PluginCatalogController>(simulator,
                                                                         ui->treeWidget_Plugins,
                                                                         ui->TextCodeEditor,
                                                                         _pluginCategoryColor);
    // Initialize Phase 2 services using narrow dependencies and compatibility callbacks.
    _graphicalModelBuilder = std::make_unique<GraphicalModelBuilder>(simulator,
                                                                      ui->graphicsView,
                                                                      ui->graphicsView->getScene(),
                                                                      _pluginCategoryColor,
                                                                      ui->textEdit_Console);
    // Keep MainWindow wrappers while delegating persistence and loading logic to Phase 2 service.
    _graphicalModelSerializer = std::make_unique<GraphicalModelSerializer>(simulator,
                                                                            this,
                                                                            ui->TextCodeEditor,
                                                                            ui->graphicsView,
                                                                            ui->horizontalSlider_ZoomGraphical,
                                                                            ui->actionShowGrid,
                                                                            ui->actionShowRule,
                                                                            ui->actionShowSnap,
                                                                            ui->actionShowGuides,
                                                                            ui->actionShowInternalElements,
                                                                            ui->actionShowAttachedElements,
                                                                            ui->actionDiagrams,
                                                                            ui->textEdit_Console,
                                                                            &_modelfilename,
                                                                            [this]() { _clearModelEditors(); },
                                                                            [this]() { _generateGraphicalModelFromModel(); },
                                                                            [this]() { on_actionShowInternalElements_triggered(); },
                                                                            [this]() { on_actionShowAttachedElements_triggered(); },
                                                                            [this]() { on_actionDiagrams_triggered(); });
    //
    // property editor
    ui->treeViewPropertyEditor->setAlternatingRowColors(true);
    // Initialize the Phase 6 property-editor controller after view/editor dependencies are available.
    _propertyEditorController = std::make_unique<PropertyEditorController>(
        ui->treeViewPropertyEditor,
        ui->graphicsView,
        propertyGenesys,
        propertyList,
        propertyEditorUI,
        propertyBox,
        [this]() { _actualizeModelSimLanguage(); },
        [this](bool force) { _actualizeModelComponents(force); },
        [this](bool force) { _actualizeModelDataDefinitions(force); },
        [this]() { _actualizeModelCppCode(); },
        [this]() { return _createModelImage(); },
        [this]() { _actualizeTabPanes(); },
        [this]() { _actualizeActions(); });
    // Keep callback wiring in MainWindow while delegating behavior to the Phase 6 controller.
    ui->treeViewPropertyEditor->setModelChangedCallback([this]() {
        this->_onPropertyEditorModelChanged();
    });

    // system preferences
    SystemPreferences::load();
    if (SystemPreferences::autoLoadPlugins()) {
        simulator->getPluginManager()->autoInsertPlugins(_autoLoadPluginsFilename.toStdString());
        // now complete the information
        for (unsigned int i = 0; i < simulator->getPluginManager()->size(); i++) {
            //@TODO: now it's the opportunity to adjust template
            _insertPluginUI(simulator->getPluginManager()->getAtRank(i));
        }
    }
    if (SystemPreferences::startMaximized()) {
        // another try to start maximized (it should not be that hard)
        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        this->resize(screenGeometry.width(), screenGeometry.height());
    }
    if (SystemPreferences::modelAtStart() == 1) { // NEW MODEL (should be enum
        this->on_actionModelNew_triggered();
    } else  if (SystemPreferences::modelAtStart() == 2) { // LOAD MODEL (should be enum
        this->_loadGraphicalModel(SystemPreferences::modelfilename());
    }

    for (QAction* action : this->findChildren<QAction*>()) {
        if (action == nullptr || !action->objectName().startsWith("action")) {
            continue;
        }
        connect(action, &QAction::triggered, this, [action](bool checked) {
            qInfo().noquote() << QString("GUI action triggered: %1 (%2) checked=%3")
                                     .arg(action->objectName(), action->text())
                                     .arg(checked);
        });
    }

    // finally
    _actualizeActions();
    //_actualizeTabPanes();
}

MainWindow::~MainWindow() {
    _shuttingDown = true;
    _disconnectSceneSignals("~MainWindow");
    disconnect();
    if (ui != nullptr && ui->graphicsView != nullptr) {
        ui->graphicsView->clearEventHandlers();
    }
    delete ui;
    delete simulator;
    delete propertyGenesys;
    delete propertyList;
    delete propertyEditorUI;
    delete propertyBox;
    delete _pluginCategoryColor;
    delete _gmc_copies;
    delete _ports_copies;
    delete _draw_copy;
    delete _group_copy;
    delete undoView;
}

void MainWindow::_disconnectSceneSignals(const char* context) {
    if (_sceneChangedConnection) {
        QObject::disconnect(_sceneChangedConnection);
        _sceneChangedConnection = QMetaObject::Connection();
    }
    if (_sceneFocusItemChangedConnection) {
        QObject::disconnect(_sceneFocusItemChangedConnection);
        _sceneFocusItemChangedConnection = QMetaObject::Connection();
    }
    if (_sceneSelectionChangedConnection) {
        QObject::disconnect(_sceneSelectionChangedConnection);
        _sceneSelectionChangedConnection = QMetaObject::Connection();
    }
    QObject* sceneObject = (ui != nullptr && ui->graphicsView != nullptr) ? ui->graphicsView->scene() : nullptr;
    qInfo() << "Scene/MainWindow signal connections disconnected. context=" << context
            << " scene=" << sceneObject << " mainWindow=" << this;
}

void MainWindow::_connectSceneSignals() {
    _disconnectSceneSignals("_connectSceneSignals");
    if (ui == nullptr || ui->graphicsView == nullptr || ui->graphicsView->scene() == nullptr) {
        qWarning() << "Skipping scene connection because ui/graphicsView/scene is null";
        return;
    }
    _sceneChangedConnection = connect(ui->graphicsView->scene(), &QGraphicsScene::changed, this, &MainWindow::sceneChanged);
    _sceneFocusItemChangedConnection = connect(ui->graphicsView->scene(), &QGraphicsScene::focusItemChanged, this, &MainWindow::sceneFocusItemChanged);
    _sceneSelectionChangedConnection = connect(ui->graphicsView->scene(), &QGraphicsScene::selectionChanged, this, &MainWindow::sceneSelectionChanged);
    qInfo() << "Scene/MainWindow signal connections attached. scene=" << ui->graphicsView->scene()
            << " mainWindow=" << this;
}

ModelGraphicsScene* MainWindow::myScene() const {
    return ui->graphicsView->getScene();
}

void MainWindow::_onPropertyEditorModelChanged() {
    // Keep this wrapper for compatibility during the incremental Phase 6 refactor.
    if (_propertyEditorController != nullptr) {
        _propertyEditorController->onPropertyEditorModelChanged();
    }
}


//-----------------------------------------------------------------


void::MainWindow::saveItemForCopy(QList<GraphicalModelComponent*> * gmcList, QList<GraphicalConnection*> * connList) {
    foreach (GraphicalConnection *conn, *connList) {
        ModelComponent * source = conn->getSource()->component;
        ModelComponent * dst = conn->getDestination()->component;

        GraphicalModelComponent * sourceSelected = nullptr;
        GraphicalModelComponent * dstSelected = nullptr;
        foreach (GraphicalModelComponent * comp, *gmcList) {

            if (source != nullptr) {

                if (comp->getComponent()->getId() == source->getId()) {
                    sourceSelected = comp;
                }
            }

            if (dst != nullptr) {

                if (comp->getComponent()->getId() == dst->getId()) {
                    dstSelected = comp;
                }
            }
        }

        if (sourceSelected == nullptr || dstSelected == nullptr) {
            connList->removeOne(conn);
        }
    }
}



void MainWindow::_actualizeActions() {
    bool opened = simulator->getModelManager()->current() != nullptr;
    bool running = false;
    bool paused = false;
    bool canCutCopyDelete = false;
    bool canPaste = false;
    bool canGroup = false;
    bool canUngroup = false;
    bool canConnect = false;
    unsigned int actualCommandundoRedo = 0; //@TODO
    unsigned int maxCommandundoRedo = 0; //@TODO
    if (opened) {
        running = simulator->getModelManager()->current()->getSimulation()->isRunning();
        paused = simulator->getModelManager()->current()->getSimulation()->isPaused();

        ModelGraphicsScene* scene = ui->graphicsView->getScene();
        if (scene != nullptr) {
            const QList<QGraphicsItem*> selectedItems = scene->selectedItems();
            canCutCopyDelete = !selectedItems.isEmpty();
            canPaste = !_draw_copy->empty() || !_gmc_copies->empty() || !_group_copy->empty() || !_ports_copies->empty();

            int selectedComponents = 0;
            bool selectedGroup = false;
            for (QGraphicsItem* item : selectedItems) {
                if (dynamic_cast<GraphicalModelComponent*>(item) != nullptr) {
                    selectedComponents++;
                } else if (dynamic_cast<QGraphicsItemGroup*>(item) != nullptr) {
                    selectedGroup = true;
                }
            }

            canGroup = selectedComponents >= 2 && !selectedGroup;
            canUngroup = selectedItems.size() == 1 && selectedGroup;
            canConnect = scene->connectingStep() == 0;
        }
    }

    //
    ui->graphicsView->setEnabled(opened);
    ui->tabWidgetCentral->setEnabled(opened);
    // model
    ui->menuModel->setEnabled(!running);
    ui->actionModelNew->setEnabled(!running);
    ui->actionModelSave->setEnabled(opened && !running);
    ui->actionModelOpen->setEnabled(!running);
    ui->actionModelClose->setEnabled(opened && !running);
    ui->actionModelInformation->setEnabled(opened);
    ui->actionModelCheck->setEnabled(opened && !running);
    //edit
    ui->toolBarEdit->setEnabled(opened && !running);
    ui->menuEdit->setEnabled(opened && !running);
    // view
    ui->menuView->setEnabled(opened && !running);
    ui->toolBarView->setEnabled(opened && !running);
    ui->toolBarAnimate->setEnabled(opened && !running);
    ui->toolBarGraphicalModel->setEnabled(opened && !running);
    ui->toolBarDraw->setEnabled(opened && !running);
    // simulation
    ui->menuSimulation->setEnabled(opened);
    ui->actionSimulationConfigure->setEnabled(opened && !running);
    ui->actionSimulationStart->setEnabled(opened && !running);
    ui->actionSimulationStep->setEnabled(opened && !running);
    ui->actionSimulationStop->setEnabled(opened && (running || paused));
    ui->actionSimulationPause->setEnabled(opened && running);
    ui->actionSimulationResume->setEnabled(opened && paused);
    ui->actionActivateGraphicalSimulation->setEnabled(opened);
    ui->actionSimulatorsPluginManager->setEnabled(!running);
    ui->actionSimulatorPreferences->setEnabled(!running);

    // debug
    ui->tableWidget_Breakpoints->setEnabled(opened && !running);
    ui->tableWidget_Entities->setEnabled(opened && !running);
    ui->tableWidget_Variables->setEnabled(opened && !running);

    // Property Editor
    ui->treeViewPropertyEditor->setEnabled(!running);

    // based on SELECTED GRAPHICAL OBJECTS or on COMMANDS DONE (UNDO/REDO)
    ui->toolBarArranje->setEnabled(opened && !running);
    ui->actionEditCopy->setEnabled(canCutCopyDelete && !running);
    ui->actionEditCut->setEnabled(canCutCopyDelete && !running);
    ui->actionEditDelete->setEnabled(canCutCopyDelete && !running);
    ui->actionEditPaste->setEnabled(canPaste && !running);
    ui->actionGModelShowConnect->setEnabled(opened && canConnect && !running);
    ui->actionViewGroup->setEnabled(opened && canGroup && !running);
    ui->actionEditGroup->setEnabled(opened && canGroup && !running);
    ui->actionViewUngroup->setEnabled(opened && canUngroup && !running);
    ui->actionEditUngroup->setEnabled(opened && canUngroup && !running);
    ui->actionEditReplace->setEnabled(opened && !running);

    // sliders
    ui->horizontalSlider_ZoomGraphical->setEnabled(opened && !running);
    if (_modelWasOpened && !opened) {
        _clearModelEditors();
    }

    //slider animation speed
    ui->horizontalSliderAnimationSpeed->setEnabled(running && !paused);

    ui->actionSelectAll->setEnabled(opened && !running);

    _modelWasOpened = opened;
}

void MainWindow::_actualizeTabPanes() {
    bool opened = simulator->getModelManager()->current() != nullptr;
    if (opened) {
        int index = ui->tabWidgetCentral->currentIndex();
        if (index == CONST.TabCentralModelIndex) {
            index = ui->tabWidgetModel->currentIndex();
            if (ui->tabWidgetModel->currentIndex() == CONST.TabModelDiagramIndex) {
                _createModelImage();
            } else if (index == CONST.TabModelSimLangIndex) {
                this->_actualizeModelSimLanguage();
            } else if (index == CONST.TabModelCppCodeIndex) {
                _actualizeModelCppCode();
            } else if (index == CONST.TabModelComponentsIndex) {
                _actualizeModelComponents(true);
            } else if (index == CONST.TabModelDataDefinitionsIndex) {
                _actualizeModelDataDefinitions(true);
            }
        } else if (index == CONST.TabCentralModelIndex) {
            index = ui->tabWidgetSimulation->currentIndex();
            if (index == CONST.TabSimulationBreakpointsIndex) {
                _actualizeDebugBreakpoints(true);
            } else if (index == CONST.TabSimulationEntitiesIndex) {
                _actualizeDebugEntities(true);
            } else if (index == CONST.TabSimulationVariablesIndex) {
                _actualizeDebugVariables(true);
            }
        } else if (index == CONST.TabCentralReportsIndex) {
            index = ui->tabWidgetReports->currentIndex(); //@TODO: Add results
        }
    } else {
        ui->actionAnimateCounter->setChecked(false);
        ui->actionAnimateVariable->setChecked(false);
        ui->actionAnimateSimulatedTime->setChecked(false);
    }
}


void MainWindow::_actualizeSimulationEvents(SimulationEvent * re) {
    int row = ui->tableWidget_Simulation_Event->rowCount();
    ui->tableWidget_Simulation_Event->setRowCount(row + 1);
    QTableWidgetItem * newItem;
    newItem = new QTableWidgetItem(QString::fromStdString(std::to_string(re->getCurrentEvent()->getTime())));
    ui->tableWidget_Simulation_Event->setItem(row, 0, newItem);
    newItem = new QTableWidgetItem(QString::fromStdString(re->getCurrentEvent()->getComponent()->getName()));
    ui->tableWidget_Simulation_Event->setItem(row, 1, newItem);
    newItem = new QTableWidgetItem(QString::fromStdString(re->getCurrentEvent()->getEntity()->show()));
    ui->tableWidget_Simulation_Event->setItem(row, 2, newItem);
    QCoreApplication::processEvents();
}

void MainWindow::_actualizeDebugVariables(bool force) {
    QCoreApplication::processEvents();
    if (force || ui->tabWidgetSimulation->currentIndex() == CONST.TabSimulationVariablesIndex) {
        ui->tableWidget_Variables->setRowCount(0);
        List<ModelDataDefinition*>* variables = simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<Variable>());
        int row = 0;
        ui->tableWidget_Variables->setRowCount(variables->size());
        Variable* variable;
        for (ModelDataDefinition* varData : *variables->list()) {
            variable = dynamic_cast<Variable*> (varData);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem(QString::fromStdString(variable->getName()));
            ui->tableWidget_Variables->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(Util::List2str(variable->getDimensionSizes())));
            ui->tableWidget_Variables->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(Util::Map2str(variable->getValues())));
            ui->tableWidget_Variables->setItem(row, 2, newItem);
        }
    }
}

void MainWindow::_actualizeDebugEntities(bool force) {
    QCoreApplication::processEvents();
    if (force || ui->tabWidgetSimulation->currentIndex() == CONST.TabSimulationEntitiesIndex) {
        List<ModelDataDefinition*>* entities = simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<Entity>());
        List<ModelDataDefinition*>* attributes = simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>());
        Entity* entity;
        int row = 0;
        int column = 3;
        QTableWidgetItem* newItem;
        if (ui->tableWidget_Entities->columnCount() < attributes->size() + 3) {
            ui->tableWidget_Entities->setColumnCount(3 + attributes->size());
            for (ModelDataDefinition* attribData : *attributes->list()) {
                newItem = new QTableWidgetItem(QString::fromStdString(attribData->getName()));
                ui->tableWidget_Entities->setHorizontalHeaderItem(column++, newItem);
            }
        }
        ui->tableWidget_Entities->setRowCount(0);
        ui->tableWidget_Entities->setRowCount(entities->size());
        for (ModelDataDefinition* entData : *entities->list()) {
            entity = dynamic_cast<Entity*> (entData);
            //			ui->tableWidget_Entities->setRowCount(row);
            //std::cout << row << " - " << entity->entityNumber() << " - " << entity->getName() << " - " << entity->getEntityTypeName() << std::endl;
            newItem = new QTableWidgetItem(QString::fromStdString(std::to_string(entity->entityNumber())));
            ui->tableWidget_Entities->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(entity->getName()));
            ui->tableWidget_Entities->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(entity->getEntityTypeName()));
            ui->tableWidget_Entities->setItem(row, 2, newItem);
            int column = 3;
            for (ModelDataDefinition* attribData : *attributes->list()) {
                newItem = new QTableWidgetItem(QString::fromStdString(std::to_string(entity->getAttributeValue(attribData->getName()))));
                ui->tableWidget_Entities->setItem(row, column++, newItem);
            }
            row++;
        }
        QCoreApplication::processEvents();
    }
}

void MainWindow::_actualizeDebugBreakpoints(bool force) {
    QCoreApplication::processEvents();
    if (force || ui->tabWidgetSimulation->currentIndex() == CONST.TabSimulationBreakpointsIndex) {
        ui->tableWidget_Breakpoints->setRowCount(0);
        ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
        int row = 0;
        for (ModelComponent* comp : *sim->getBreakpointsOnComponent()->list()) {
            ui->tableWidget_Breakpoints->setRowCount(row + 1);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem("True");
            ui->tableWidget_Breakpoints->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem("Component");
            ui->tableWidget_Breakpoints->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(comp->getName()));
            ui->tableWidget_Breakpoints->setItem(row, 2, newItem);
            row++;
        }
        for (Entity* entity : *sim->getBreakpointsOnEntity()->list()) {
            ui->tableWidget_Breakpoints->setRowCount(row + 1);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem("True");
            ui->tableWidget_Breakpoints->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem("Entity");
            ui->tableWidget_Breakpoints->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(entity->getName()));
            ui->tableWidget_Breakpoints->setItem(row, 2, newItem);
            row++;
        }
        for (double time : *sim->getBreakpointsOnTime()->list()) {
            ui->tableWidget_Breakpoints->setRowCount(row + 1);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem("True");
            ui->tableWidget_Breakpoints->setItem(row, 0, newItem);
            newItem = new QTableWidgetItem("Time");
            ui->tableWidget_Breakpoints->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(std::to_string(time)));
            ui->tableWidget_Breakpoints->setItem(row, 2, newItem);
            row++;
        }
    }
}


void MainWindow::_insertCommandInConsole(std::string text) {
    ui->textEdit_Console->setTextColor(UtilGUI::rgbaFromPacked(TraitsGUI<GMainWindow>::consoleTextColor));
    QFont font(ui->textEdit_Console->font());
    font.setBold(true);
    ui->textEdit_Console->setFont(font);
    ui->textEdit_Console->append("\n$genesys> " + QString::fromStdString(text));
    ui->textEdit_Console->moveCursor(QTextCursor::MoveOperation::Down, QTextCursor::MoveMode::MoveAnchor);
    font.setBold(false);
    ui->textEdit_Console->setFont(font);
}


//-------------

void MainWindow::_gentle_zoom(double factor) {
    QPointF target_scene_pos, target_viewport_pos;
    QPoint mouse_event_pos = QPoint(ui->graphicsView->width()/2, ui->graphicsView->height()/2); //QPoint(100, 100);
    target_viewport_pos = mouse_event_pos;
    target_scene_pos = ui->graphicsView->mapToScene(mouse_event_pos);
    ui->graphicsView->scale(factor, factor);
    ui->graphicsView->centerOn(target_scene_pos);
    QPointF delta_viewport_pos = target_viewport_pos - QPointF(ui->graphicsView->viewport()->width() / 2.0,
            ui->graphicsView->viewport()->height() / 2.0);
    QPointF viewport_center = ui->graphicsView->mapFromScene(target_scene_pos) - delta_viewport_pos;
    ui->graphicsView->centerOn(ui->graphicsView->mapToScene(viewport_center.toPoint()));
    //emit zoomed();
}

void MainWindow::_showMessageNotImplemented(){
    QMessageBox::warning(this, "Ops...", "Sorry. This functionalitty was not implemented yet. Genesys is a free open-source simulator (and tools) available at 'https://github.com/rlcancian/Genesys-Simulator'. Help us by submiting your pull requests containing code improvements.");
}


void MainWindow::_helpCopy() {
    // Pega a cena
    ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->getScene());

    QList<COPY*> * aux = new QList<COPY *>();
    QList<GraphicalModelComponent *> *gmc_aux  = new QList<GraphicalModelComponent*>();
    QList<GraphicalModelComponent *> *gmc_old_group_aux  = new QList<GraphicalModelComponent*>();
    QList<GraphicalModelComponent *> *gmc_new_group_aux  = new QList<GraphicalModelComponent*>();
    QList<GraphicalConnection *> *ports_aux = new QList<GraphicalConnection*>();
    QList<QGraphicsItem *> *drawing_aux = new QList<QGraphicsItem*>();
    QList<QGraphicsItemGroup *> *group_aux = new QList<QGraphicsItemGroup*>();

    // Adicionando todos os componentes antes
    foreach (GraphicalModelComponent * gmc , *_gmc_copies) {

        if (gmc->group())
            continue;

        // Componente
        ModelComponent * previousComponent = gmc->getComponent();

        // Adiciona o componente no modelo
        simulator->getModelManager()->current()->getComponentManager()->insert(previousComponent);

        // Nome do plugin para a copia do componente
        std::string pluginname = previousComponent->getClassname();

        // Plugin para a copia do novo component
        Plugin* plugin = simulator->getPluginManager()->find(pluginname);

        // Ajustando a posicao da copia
        //@TODO: Modificar para por onde o mouse clicou
        QPointF position = gmc->pos();

        // Copiando a cor
        QColor color = gmc->getColor();

        // Componente de Copia ou Recorte
        ModelComponent * component  = (ModelComponent*) plugin->newInstance(simulator->getModelManager()->current());


        GraphicalModelComponent* newgmc = new GraphicalModelComponent(plugin, component, position, color);
        // Adiciona o componente graficamente
        GraphicalModelComponent * oldgmc = scene->findGraphicalModelComponent(previousComponent->getId());

        COPY * temp = new COPY();
        temp->old = oldgmc;
        temp->copy = newgmc;
        aux->append(temp);
        gmc_aux->append(newgmc);
    }

    // Adicionando todos os componentes antes
    foreach (QGraphicsItemGroup *group , *_group_copy) {
        QList<GraphicalConnection*> * connGroup = new QList<GraphicalConnection*>();

        unsigned int size = group->childItems().size();

        for (unsigned int i = 0; i < (unsigned int) size; i++) {
            GraphicalModelComponent *gmc = dynamic_cast<GraphicalModelComponent *>(group->childItems().at(0));

            // remove do grupo para tratar o componente como um componente individual
            group->removeFromGroup(gmc);

            // Componente
            ModelComponent * previousComponent = gmc->getComponent();

            // Adiciona o componente no modelo
            simulator->getModelManager()->current()->getComponentManager()->insert(previousComponent);

            // Nome do plugin para a copia do componente
            std::string pluginname = previousComponent->getClassname();

            // Plugin para a copia do novo component
            Plugin* plugin = simulator->getPluginManager()->find(pluginname);

            // Ajustando a posicao da copia
            //@TODO: Modificar para por onde o mouse clicou
            QPointF position = gmc->pos();

            // Copiando a cor
            QColor color = gmc->getColor();

            // Componente de Copia ou Recorte
            ModelComponent * component  = (ModelComponent*) plugin->newInstance(simulator->getModelManager()->current());

            GraphicalModelComponent* newgmc = new GraphicalModelComponent(plugin, component, position, color);
            // Adiciona o componente graficamente
            GraphicalModelComponent * oldgmc = scene->findGraphicalModelComponent(previousComponent->getId());

            if (!oldgmc->getGraphicalInputPorts().empty() && !oldgmc->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                connGroup->removeOne(oldgmc->getGraphicalInputPorts().at(0)->getConnections()->at(0));
                connGroup->append(oldgmc->getGraphicalInputPorts().at(0)->getConnections()->at(0));
            }

            for (int j = 0; j < oldgmc->getGraphicalOutputPorts().size(); ++j) {
                GraphicalComponentPort *port = oldgmc->getGraphicalOutputPorts().at(j);

                if (!port->getConnections()->empty()) {
                    connGroup->removeOne(port->getConnections()->at(0));
                    connGroup->append(port->getConnections()->at(0));
                }
            }

            gmc_old_group_aux->append(oldgmc);
            gmc_new_group_aux->append(newgmc);
            COPY * temp = new COPY();
            temp->old = oldgmc;
            temp->copy = newgmc;
            aux->append(temp);
            gmc_aux->append(newgmc);
        }

        saveItemForCopy(gmc_old_group_aux, connGroup);

        // volta os itens no grupo
        for (unsigned int k = 0; k < size; k++) {
            group->addToGroup(gmc_old_group_aux->at(k));
        }

        for (unsigned int k = 0; k < (unsigned int) connGroup->size(); k++) {
            _ports_copies->removeOne(connGroup->at(k));
            _ports_copies->append(connGroup->at(k));
        }

        QGraphicsItemGroup *newGroup = new QGraphicsItemGroup();

        ui->graphicsView->getScene()->insertComponentGroup(newGroup, *gmc_new_group_aux);

        gmc_old_group_aux->clear();
        gmc_new_group_aux->clear();
        group_aux->append(newGroup);
    }

    // Adicionando as conexões (e seus respectivos componentes)
    foreach (GraphicalConnection * conn, *_ports_copies) {

        ModelComponent * source = conn->getSource()->component;
        ModelComponent * dst = conn->getDestination()->component;

        GraphicalComponentPort* sourcePort = nullptr;
        GraphicalComponentPort* destinationPort = nullptr;

        unsigned int portSourceConnection = 0;
        unsigned int portDestinationConnection = 0;

        // Ajustando a posicao da copia
        //@TODO: Modificar para por onde o mouse clicou
        GraphicalModelComponent * gmcSource = scene->findGraphicalModelComponent(source->getId());
        GraphicalModelComponent * gmcDestination = scene->findGraphicalModelComponent(dst->getId());

        foreach (GraphicalModelComponent * comp, *_gmc_copies) {

            if (comp->getComponent()->getId() == source->getId()) {
                sourcePort = conn->getSourceGraphicalPort();
                portSourceConnection = conn->getPortSourceConnection();

            }

            if (comp->getComponent()->getId() == dst->getId()) {
                destinationPort = conn->getDestinationGraphicalPort();
                portDestinationConnection = conn->getPortDestinationConnection();
            }
        }

        GraphicalModelComponent * gmcNewSource;
        GraphicalModelComponent * gmcNewDestination;

        foreach (COPY * c, *aux) {

            if (c->old == gmcSource) gmcNewSource = c->copy;
            if (c->old == gmcDestination) gmcNewDestination = c->copy;

        }

        // Cria GraphicalComponentPort para gmc source
        sourcePort = gmcNewSource->getGraphicalOutputPorts().at(portSourceConnection);

        // Cria GraphicalComponentPort para gmc destination
        destinationPort = gmcNewDestination->getGraphicalInputPorts().at(portDestinationConnection);

        // Conecta os componente graficamente e no modelo
        GraphicalConnection * newConn = scene->addGraphicalConnection(sourcePort, destinationPort, portSourceConnection, portDestinationConnection);

        ui->graphicsView->getScene()->clearPorts(newConn, gmcNewSource, gmcDestination);

        ports_aux->append(newConn);
    }

    //Adicionando os desenhos
    foreach(QGraphicsItem * draw, *_draw_copy) {
        AnimationCounter *animationCounter = dynamic_cast<AnimationCounter*>(draw);
        if (animationCounter) {
            AnimationCounter *copiedItem;
            copiedItem = new AnimationCounter();
            copiedItem->setRect(0, 0, animationCounter->boundingRect().width(), animationCounter->boundingRect().height());
            copiedItem->setPos(animationCounter->pos());
            copiedItem->setCounter(animationCounter->getCounter());
            copiedItem->setValue(animationCounter->getValue());
            drawing_aux->append(copiedItem);
            continue;
        }

        AnimationVariable *animationVariable = dynamic_cast<AnimationVariable*>(draw);
        if (animationVariable) {
            AnimationVariable *copiedItem;
            copiedItem = new AnimationVariable();
            copiedItem->setRect(0, 0, animationVariable->boundingRect().width(), animationVariable->boundingRect().height());
            copiedItem->setPos(animationVariable->pos());
            copiedItem->setVariable(animationVariable->getVariable());
            copiedItem->setValue(animationVariable->getValue());
            drawing_aux->append(copiedItem);
            continue;
        }

        QGraphicsRectItem* rectItem = dynamic_cast<QGraphicsRectItem*>(draw);
        if (rectItem) {
            QGraphicsRectItem *copiedItem;
            copiedItem = new QGraphicsRectItem(rectItem->rect());
            copiedItem->setPos(rectItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawing_aux->append(copiedItem);
            continue;
        }

        QGraphicsEllipseItem* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(draw);
        if (ellipseItem) {
            QGraphicsEllipseItem *copiedItem;
            copiedItem = new QGraphicsEllipseItem(ellipseItem->rect());
            copiedItem->setPos(ellipseItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawing_aux->append(copiedItem);
            continue;
        }

        QGraphicsPolygonItem* polygonItem = dynamic_cast<QGraphicsPolygonItem*>(draw);
        if (polygonItem) {
            QGraphicsPolygonItem *copiedItem;
            copiedItem = new QGraphicsPolygonItem(polygonItem->polygon());
            copiedItem->setPos(polygonItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawing_aux->append(copiedItem);
            continue;
        }

        QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem*>(draw);
        if (lineItem) {
            QGraphicsLineItem *copiedItem;
            copiedItem = new QGraphicsLineItem(lineItem->line());
            copiedItem->setPos(lineItem->pos());
            copiedItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
            copiedItem->setFlag(QGraphicsItem::ItemIsMovable, true);
            drawing_aux->append(copiedItem);
            continue;
        }
    }

    _gmc_copies = gmc_aux;
    _ports_copies = ports_aux;
    _draw_copy = drawing_aux;
    _group_copy = group_aux;
}



bool MainWindow::_check(bool success)
{
    _insertCommandInConsole("check");

    // reinsere os data definitions no modelo de componentes restauradosapenas se não for um modelo previamente carregado que ja tem seus data definitions
    myScene()->insertRestoredDataDefinitions(_loaded);

    _loaded = false;

    // Ativa visualização de animação
    ui->actionActivateGraphicalSimulation->setChecked(true);

    // Valida o modelo
    bool res = simulator->getModelManager()->current()->check();

    // cria o StatisticsCollector pro EntityType se necessário
    setStatisticsCollector();

    // limpa o valor dos contadores, variáveis e timer na cena
    myScene()->clearAnimationsValues();

    // Atualiza as ações e painéis
    _actualizeActions();
    _actualizeTabPanes();

    if (res) {
        ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
        // Mensagem de sucesso
        if (success) {
            if (!scene->existDiagram()){
                scene->createDiagrams();
            } else {
                scene->destroyDiagram();
                scene->createDiagrams();
            }
            QMessageBox::information(this, "Model Check", "Model successfully checked.");
        }
        // Salva os data definitions dos componentes atuais
        myScene()->saveDataDefinitions();

        // Seta os em uma lista os contadores e variáveis criadas
        myScene()->setCounters();
        myScene()->setVariables();

        _modelCheked = true;
    } else {
        // Mensagem de erro
        QMessageBox::critical(this, "Model Check", "Model has erros. See the console for more information.");
        _modelCheked = false;
    }

    return res;
}

void MainWindow::setStatisticsCollector() {
    std::list<ModelDataDefinition*>* entityTypes = simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
    std::list<ModelDataDefinition*>* stCollectors = simulator->getModelManager()->current()->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>())->list();

    QList<ModelDataDefinition*> qlStCollectors(stCollectors->begin(), stCollectors->end());

    if (!entityTypes->empty()) {
        std::string suffix = ".TotalTimeInSystem";

        for (ModelDataDefinition* stCollector : *entityTypes) {
            if (stCollector->isReportStatistics()) {
                StatisticsCollector* stc = static_cast<EntityType*> (stCollector)->addGetStatisticsCollector(stCollector->getName() + suffix);

                // necessário pois o kernel remove o StatisticsCollector de DataManager mas não de _statisticsCollectors usado
                // por addGetStatisticsCollector para criar ou não (verifica se tem na lista) o Data Definition
                if (!qlStCollectors.contains(stc)) {
                    simulator->getModelManager()->current()->getDataManager()->insert(stc);
                }
            }
        }
    }
}
bool MainWindow::checkSelectedDrawIcons() {
    int alreadyChecked = 0;
    if(ui->actionDrawLine->isChecked()) alreadyChecked++;
    if(ui->actionDrawRectangle->isChecked()) alreadyChecked++;
    if(ui->actionDrawEllipse->isChecked()) alreadyChecked++;
    if(ui->actionDrawPoligon->isChecked()) alreadyChecked++;
    if(ui->actionDrawText->isChecked()) alreadyChecked++;
    if(ui->actionAnimateCounter->isChecked()) alreadyChecked++;
    if(ui->actionAnimateVariable->isChecked()) alreadyChecked++;
    if(ui->actionAnimateSimulatedTime->isChecked()) alreadyChecked++;
    if (alreadyChecked > 1) return true;
    else return false;
}

void MainWindow::unselectDrawIcons() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    ui->actionDrawLine->setChecked(false);
    ui->actionDrawRectangle->setChecked(false);
    ui->actionDrawEllipse->setChecked(false);
    ui->actionDrawPoligon->setChecked(false);
    ui->actionDrawText->setChecked(false);
    ui->actionAnimateCounter->setChecked(false);
    ui->actionAnimateVariable->setChecked(false);
    ui->actionAnimateSimulatedTime->setChecked(false);
    scene->clearDrawingMode();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (_closingApproved || _confirmApplicationExit()) {
        _closingApproved = true;
        // limpando referencia do ultimo elemento selecionado em property editor
        ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();
        event->accept();
        return;
    }
    event->ignore();
}

void MainWindow::_initUiForNewModel(Model* m) {
    _actualizeUndo();
    ui->graphicsView->getScene()->showGrid(); //@TODO: Bad place to be
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();
    ui->textEdit_Console->moveCursor(QTextCursor::End);
    if (m == nullptr) { // a new model. Create the model template
        ui->TextCodeEditor->clear();
        // create a basic initial template for the model
        std::string tempFilename = "./temp.tmp";
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
        bool res = m->save(tempFilename);
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, false);
        if (res) { // read the file saved and copy its contents to the model text editor
            std::string line;
            std::ifstream file(tempFilename);
            if (file.is_open()) {
                ui->TextCodeEditor->appendPlainText("# Genesys Model File");
                ui->TextCodeEditor->appendPlainText("# Simulator, ModelInfo and ModelSimulation");
                while (std::getline(file, line)) {
                    ui->TextCodeEditor->appendPlainText(QString::fromStdString(line));
                }
                file.close();
                //QMessageBox::information(this, "New Model", "Model successfully created");
            } else {
                ui->textEdit_Console->append(QString("Error reading template model file"));
            }
            _actualizeModelTextHasChanged(true);
            _setOnEventHandlers();
        } else {
            ui->textEdit_Console->append(QString("Error saving template model file"));
        }
        _modelfilename = "";
    } else {	// beind loaded
        _setOnEventHandlers();
    }
    _actualizeActions();
    _actualizeTabPanes();
}

void MainWindow::_actualizeUndo() {
    undoView = new QUndoView(ui->graphicsView->getScene()->getUndoStack());
    undoView->setWindowTitle(tr("Command List"));
    undoView->setVisible(false);
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
}
