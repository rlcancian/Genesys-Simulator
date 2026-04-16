// Document this compilation unit as the MainWindow composition-root partition.
/**
 * @file mainwindow.cpp
 * @brief Composition-root partition of MainWindow implementation.
 *
 * This file contains central MainWindow construction, initialization, and wiring logic,
 * including creation of extracted controllers/services and baseline UI setup. It does not
 * define a new class; it is a physical partition of the same MainWindow implementation used
 * as a compatibility façade in the incremental refactoring.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"

// Dialogs
// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../kernel/simulator/Counter.h"
#include "../../../../kernel/simulator/StatisticsCollector.h"
#include "../../../TraitsApp.h"
// GUI
#include "graphicals/ModelGraphicsScene.h"
#include "TraitsGUI.h"
#include "graphicals/GraphicalConnection.h"
#include "controllers/SimulationController.h"
// Keep explicit controller includes to make MainWindow composition-root wiring clear.
#include "controllers/ModelInspectorController.h"
#include "controllers/TraceConsoleController.h"
#include "controllers/SimulationEventController.h"
// Add Phase 5 controller include for plugin-catalog responsibilities.
#include "controllers/PluginCatalogController.h"
// Add Phase 6 controller include for property-editor and scene-selection orchestration.
#include "controllers/PropertyEditorController.h"
// Add Phase 7 controller include for model/application lifecycle orchestration.
#include "controllers/ModelLifecycleController.h"
// Add Phase 8 controller include for simulation-command orchestration.
#include "controllers/SimulationCommandController.h"
// Add Phase 9 controller include for edit-command orchestration.
#include "controllers/EditCommandController.h"
// Add Phase 10 controller include for scene/view/drawing command orchestration.
#include "controllers/SceneToolController.h"
// Add graphical context-menu controller include for canvas popup orchestration.
#include "controllers/GraphicalContextMenuController.h"
// Add Phase 11 controller include for dialog/utility orchestration.
#include "controllers/DialogUtilityController.h"
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
#include <algorithm>
#include <cmath>
#include <limits>
//#include <streambuf>
// QT
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
// #include <qt5/QtWidgets/qgraphicsitem.h>
#include <QtWidgets/qgraphicsitem.h>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDebug>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QAction>
#include <QFrame>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QTabBar>
#include <QVBoxLayout>

namespace {
struct ReportPlotItem {
    QString label;
    double minimum = 0.0;
    double average = 0.0;
    double maximum = 0.0;
};

class ResultsBoxPlotWidget : public QFrame {
public:
    explicit ResultsBoxPlotWidget(const QVector<ReportPlotItem>& items, QWidget* parent = nullptr)
        : QFrame(parent), _items(items) {
        setMinimumHeight(260);
        setMinimumWidth(std::max(480, 90 * static_cast<int>(_items.size()) + 120));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), palette().base());

        if (_items.isEmpty()) {
            painter.drawText(rect(), Qt::AlignCenter, QObject::tr("No plottable results."));
            return;
        }

        const int left = 76;
        const int right = 24;
        const int top = 18;
        const int bottom = 82;
        const QRect plotRect(left, top, width() - left - right, height() - top - bottom);
        if (plotRect.width() <= 0 || plotRect.height() <= 0) {
            return;
        }

        double minValue = std::numeric_limits<double>::infinity();
        double maxValue = -std::numeric_limits<double>::infinity();
        for (const ReportPlotItem& item : _items) {
            minValue = std::min(minValue, item.minimum);
            maxValue = std::max(maxValue, item.maximum);
        }
        if (!std::isfinite(minValue) || !std::isfinite(maxValue)) {
            return;
        }
        if (minValue == maxValue) {
            const double pad = std::max(1.0, std::abs(minValue) * 0.1);
            minValue -= pad;
            maxValue += pad;
        } else {
            const double pad = (maxValue - minValue) * 0.08;
            minValue -= pad;
            maxValue += pad;
        }

        auto toY = [plotRect, minValue, maxValue](double value) {
            const double ratio = (value - minValue) / (maxValue - minValue);
            return plotRect.bottom() - ratio * plotRect.height();
        };

        painter.setPen(QPen(QColor(210, 210, 210), 1));
        const int tickCount = 5;
        for (int tick = 0; tick <= tickCount; ++tick) {
            const double ratio = static_cast<double>(tick) / tickCount;
            const double value = minValue + ratio * (maxValue - minValue);
            const int y = static_cast<int>(toY(value));
            painter.drawLine(plotRect.left(), y, plotRect.right(), y);
            painter.setPen(QPen(QColor(80, 80, 80), 1));
            painter.drawText(4, y - 9, left - 10, 18, Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(value, 'g', 5));
            painter.setPen(QPen(QColor(210, 210, 210), 1));
        }

        painter.setPen(QPen(QColor(70, 70, 70), 1));
        painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight());
        painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());

        const double slotWidth = static_cast<double>(plotRect.width()) / _items.size();
        const int boxWidth = std::max(12, std::min(42, static_cast<int>(slotWidth * 0.45)));
        const QColor boxColor(100, 149, 237, 95);
        const QPen boxPen(QColor(49, 92, 156), 1.4);
        const QPen meanPen(QColor(190, 70, 55), 2.0);

        for (int index = 0; index < _items.size(); ++index) {
            const ReportPlotItem& item = _items.at(index);
            const double average = std::max(item.minimum, std::min(item.average, item.maximum));
            const double q1 = item.minimum + (average - item.minimum) * 0.5;
            const double q3 = average + (item.maximum - average) * 0.5;
            const int x = plotRect.left() + static_cast<int>((index + 0.5) * slotWidth);
            const int yMin = static_cast<int>(toY(item.minimum));
            const int yMax = static_cast<int>(toY(item.maximum));
            const int yQ1 = static_cast<int>(toY(q1));
            const int yQ3 = static_cast<int>(toY(q3));
            const int yAvg = static_cast<int>(toY(average));

            painter.setPen(boxPen);
            painter.drawLine(x, yMax, x, yMin);
            painter.drawLine(x - boxWidth / 3, yMax, x + boxWidth / 3, yMax);
            painter.drawLine(x - boxWidth / 3, yMin, x + boxWidth / 3, yMin);
            painter.setBrush(boxColor);
            painter.drawRect(QRect(QPoint(x - boxWidth / 2, yQ3),
                                   QPoint(x + boxWidth / 2, yQ1)).normalized());
            painter.setPen(meanPen);
            painter.drawLine(x - boxWidth / 2, yAvg, x + boxWidth / 2, yAvg);

            painter.save();
            painter.translate(x - 6, plotRect.bottom() + 12);
            painter.rotate(-35);
            painter.setPen(QPen(QColor(65, 65, 65), 1));
            painter.drawText(QRect(0, 0, 130, 34), Qt::AlignLeft | Qt::AlignVCenter, item.label);
            painter.restore();
        }
    }

private:
    QVector<ReportPlotItem> _items;
};

static bool fillPlotItem(ModelDataDefinition* data, QString* category, ReportPlotItem* item) {
    if (data == nullptr || category == nullptr || item == nullptr) {
        return false;
    }

    ModelDataDefinition* parent = nullptr;
    if (StatisticsCollector* collector = dynamic_cast<StatisticsCollector*>(data)) {
        parent = collector->getParent();
        Statistics_if* stats = collector->getStatistics();
        if (stats == nullptr || stats->numElements() == 0) {
            return false;
        }
        item->minimum = stats->min();
        item->average = stats->average();
        item->maximum = stats->max();
    } else if (Counter* counter = dynamic_cast<Counter*>(data)) {
        parent = counter->getParent();
        item->minimum = counter->getCountValue();
        item->average = counter->getCountValue();
        item->maximum = counter->getCountValue();
    } else {
        return false;
    }

    if (!std::isfinite(item->minimum) || !std::isfinite(item->average) || !std::isfinite(item->maximum)) {
        return false;
    }
    if (item->minimum > item->maximum) {
        std::swap(item->minimum, item->maximum);
    }

    const QString parentType = parent != nullptr
            ? QString::fromStdString(parent->getClassname())
            : QObject::tr("Global");
    const QString parentName = parent != nullptr
            ? QString::fromStdString(parent->getName())
            : QString();
    *category = parentType;
    item->label = parentName.isEmpty()
            ? QString::fromStdString(data->getName())
            : parentName + "." + QString::fromStdString(data->getName());
    return true;
}
} // namespace

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
    splitDockWidget(ui->dockWidgetPropertyEditor, ui->dockWidgetConsole, Qt::Vertical);
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
    _prepareReportsResultsTable();
    _prepareReportsPlots();
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
    {
        const QSignalBlocker tabWidgetModelBlocker(ui->tabWidgetModel);
        const QSignalBlocker tabBarBlocker(ui->tabWidgetModel->tabBar());
        const int modelDiagramTabIndex = ui->tabWidgetModel->indexOf(ui->tabModelDiagram);
        if (modelDiagramTabIndex >= 0) {
            ui->tabWidgetModel->setTabVisible(modelDiagramTabIndex, false);
        }
    }
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
                                                                            ui->textEdit_Console,
                                                                            &_modelfilename,
                                                                            [this]() { _clearModelEditors(); },
                                                                            [this]() { _generateGraphicalModelFromModel(); },
                                                                            [this]() { on_actionShowInternalElements_triggered(); },
                                                                            [this]() { on_actionShowAttachedElements_triggered(); });
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
    // Initialize the Phase 8 simulation-command controller after simulation controller and callbacks are available.
    _simulationCommandController = std::make_unique<SimulationCommandController>(
        _simulationController.get(),
        [this](const std::string& command) { _insertCommandInConsole(command); },
        [this]() { _actualizeActions(); },
        [this]() { return _check(false); },
        [this]() { return _setSimulationModelBasedOnText(); },
        [this]() {
            ui->textEdit_Reports->clear();
            _clearReportsResultsTable();
            _clearReportsPlots();
        });
    // Initialize the Phase 9 edit-command controller after scene and copy-buffer dependencies are available.
    _editCommandController = std::make_unique<EditCommandController>(
        simulator,
        ui->graphicsView,
        [this]() { _actualizeActions(); },
        &_cut,
        &_gmc_copies,
        &_ports_copies,
        &_draw_copy,
        &_group_copy);

    // Initialize the Phase 10 scene-tool controller after scene/view widgets and callbacks are ready.
    _sceneToolController = std::make_unique<SceneToolController>(
        ui->graphicsView,
        ui,
        [this]() { return ui->graphicsView->getScene(); },
        [this]() { return _createModelImage(); },
        [this]() { unselectDrawIcons(); },
        [this]() { return checkSelectedDrawIcons(); },
        [this](double factor) { _gentle_zoom(factor); },
        [this]() { _actualizeActions(); },
        [this]() { _actualizeTabPanes(); },
        _zoomValue,
        _firstClickShowConnection);

    // Initialize the graphical context-menu controller after edit and scene actions are wired.
    _graphicalContextMenuController = std::make_unique<GraphicalContextMenuController>(
        ui->graphicsView,
        ui,
        [this]() { return ui->graphicsView->getScene(); },
        [this]() { _actualizeActions(); });
    ui->graphicsView->setContextMenuEventHandler(_graphicalContextMenuController.get(),
                                                 &GraphicalContextMenuController::handleGraphicsViewContextMenu);

    // Initialize the Phase 11 dialog-utility controller after UI/simulator dependencies and callbacks are ready.
    _dialogUtilityController = std::make_unique<DialogUtilityController>(
        this,
        simulator,
        ui,
        ui->graphicsView,
        [this]() { _showMessageNotImplemented(); },
        [this](bool force) { _actualizeDebugBreakpoints(force); },
        [this]() { return _createModelImage(); },
        [this]() { _actualizeActions(); },
        [this]() { _actualizeTabPanes(); },
        [this]() {
            if (_pluginCatalogController != nullptr) {
                _pluginCatalogController->reloadFromPluginManager();
            }
        },
        [this]() { return myScene(); },
        _optimizerPrecision,
        _optimizerMaxSteps,
        _parallelizationEnabled,
        _parallelizationThreads,
        _parallelizationBatchSize,
        _lastDataAnalyzerPath);

    // Initialize the Phase 7 model-lifecycle controller after simulator/UI/callback dependencies are ready.
    _modelLifecycleController = std::make_unique<ModelLifecycleController>(
        this,
        simulator,
        ui,
        &_modelfilename,
        &_textModelHasChanged,
        &_graphicalModelHasChanged,
        &_closingApproved,
        &_loaded,
        _parallelizationEnabled,
        _parallelizationThreads,
        _parallelizationBatchSize,
        ModelLifecycleController::Callbacks{
            [this](const std::string& command) { _insertCommandInConsole(command); },
            [this](Model* model) { _initUiForNewModel(model); },
            [this]() { _actualizeActions(); },
            [this]() { _actualizeTabPanes(); },
            [this](bool hasChanged) { _actualizeModelTextHasChanged(hasChanged); },
            [this]() { return _check(); },
            [this]() { return _setSimulationModelBasedOnText(); },
            [this]() { _clearModelEditors(); },
            [this](QString filename) { return _saveGraphicalModel(filename); },
            [this](QFile* file, QString data) { return _saveTextModel(file, data); },
            [this](std::string filename) { return _loadGraphicalModel(filename); },
            [this]() { _connectSceneSignals(); },
            [this](const char* context) { _disconnectSceneSignals(context); }});

    // system preferences
    SystemPreferences::load();
    if (SystemPreferences::autoLoadPlugins()) {
        simulator->getPluginManager()->autoInsertPlugins(_autoLoadPluginsFilename.toStdString());
        // now complete the information
        for (unsigned int i = 0; i < simulator->getPluginManager()->size(); i++) {
            //@TODO: now it's the opportunity to adjust template
            _insertPluginUI(simulator->getPluginManager()->getAtRank(i));
        }
        ui->treeWidget_Plugins->expandAll();
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
    // Proactively disable trace callbacks before QWidget and simulator teardown starts.
    if (simulator != nullptr && simulator->getTraceManager() != nullptr) {
        simulator->getTraceManager()->beginShutdown();
    }
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
    qInfo() << "[MainWindow] _onPropertyEditorModelChanged enter";
    // Keep this wrapper for compatibility during the incremental Phase 6 refactor.
    if (_propertyEditorController != nullptr && !_isDeferredPropertyEditorModelChangedScheduled) {
        _isDeferredPropertyEditorModelChangedScheduled = true;
        qInfo() << "[MainWindow] scheduling deferred property-editor model-changed handling";
        QMetaObject::invokeMethod(this, [this]() {
            _isDeferredPropertyEditorModelChangedScheduled = false;
            if (_propertyEditorController == nullptr) {
                return;
            }
            qInfo() << "[MainWindow] property-editor pipeline active before controller callback="
                    << _propertyEditorController->isPostCommitPipelineActive();
            qInfo() << "[MainWindow] executing deferred property-editor model-changed handling";
            _propertyEditorController->onPropertyEditorModelChanged();
        }, Qt::QueuedConnection);
    }
    qInfo() << "[MainWindow] _onPropertyEditorModelChanged exit";
}


//-----------------------------------------------------------------


void::MainWindow::saveItemForCopy(QList<GraphicalModelComponent*> * gmcList, QList<GraphicalConnection*> * connList) {
    // Keep this wrapper as part of the final compatibility façade from Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->saveItemForCopy(gmcList, connList);
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
            const QList<QGraphicsItem*> userOperableSelection = scene->userOperableItems(selectedItems);
            canCutCopyDelete = !userOperableSelection.isEmpty();
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
    // Lock GUI interactions tied to model editing while simulation is running or paused.
    const bool simulationInteractionLocked = opened && (running || paused);

    //
    ui->graphicsView->setEnabled(opened);
    ui->tabWidgetCentral->setEnabled(opened);
    // model
    // Keep model lifecycle actions locked while simulation remains active (running or paused).
    ui->menuModel->setEnabled(!simulationInteractionLocked);
    ui->actionModelNew->setEnabled(!simulationInteractionLocked);
    ui->actionModelSave->setEnabled(opened && !simulationInteractionLocked);
    ui->actionModelOpen->setEnabled(!simulationInteractionLocked);
    ui->actionModelClose->setEnabled(opened && !simulationInteractionLocked);
    ui->actionModelInformation->setEnabled(opened);
    ui->actionModelCheck->setEnabled(opened && !simulationInteractionLocked);
    //edit
    // Keep structural editing tool surfaces locked while simulation remains active.
    ui->toolBarEdit->setEnabled(opened && !simulationInteractionLocked);
    ui->menuEdit->setEnabled(opened && !simulationInteractionLocked);
    // view
    // Keep scene manipulation and drawing surfaces locked while simulation remains active.
    ui->menuView->setEnabled(opened && !simulationInteractionLocked);
    ui->toolBarView->setEnabled(opened && !simulationInteractionLocked);
    ui->toolBarAnimate->setEnabled(opened && !simulationInteractionLocked);
    ui->toolBarGraphicalModel->setEnabled(opened && !simulationInteractionLocked);
    ui->toolBarDraw->setEnabled(opened && !simulationInteractionLocked);
    // simulation
    ui->menuSimulation->setEnabled(opened);
    // Keep simulation structural controls locked while simulation remains active.
    ui->actionSimulationConfigure->setEnabled(opened && !simulationInteractionLocked);
    ui->actionSimulationStart->setEnabled(opened && !simulationInteractionLocked);
    ui->actionSimulationStep->setEnabled(opened && !simulationInteractionLocked);
    ui->actionSimulationStop->setEnabled(opened && (running || paused));
    ui->actionSimulationPause->setEnabled(opened && running);
    ui->actionSimulationResume->setEnabled(opened && paused);
    ui->actionActivateGraphicalSimulation->setEnabled(opened);
    ui->actionSimulatorsPluginManager->setEnabled(!simulationInteractionLocked);
    ui->actionSimulatorPreferences->setEnabled(!simulationInteractionLocked);
    // Keep plugins tree disabled while simulation interaction is locked.
    ui->treeWidget_Plugins->setEnabled(opened && !simulationInteractionLocked);

    // debug
    // Keep mutable debug surface locked while simulation remains active.
    ui->tableWidget_Breakpoints->setEnabled(opened && !simulationInteractionLocked);
    ui->tableWidget_Entities->setEnabled(opened && !running);
    ui->tableWidget_Variables->setEnabled(opened && !running);

    // Property Editor
    // Keep property editor disabled while simulation interaction is locked.
    ui->treeViewPropertyEditor->setEnabled(opened && !simulationInteractionLocked);

    // based on SELECTED GRAPHICAL OBJECTS or on COMMANDS DONE (UNDO/REDO)
    // Keep arrangement and structural mutation commands locked while simulation remains active.
    ui->toolBarArranje->setEnabled(opened && !simulationInteractionLocked);
    ui->actionEditCopy->setEnabled(canCutCopyDelete && !simulationInteractionLocked);
    ui->actionEditCut->setEnabled(canCutCopyDelete && !simulationInteractionLocked);
    ui->actionEditDelete->setEnabled(canCutCopyDelete && !simulationInteractionLocked);
    ui->actionEditPaste->setEnabled(canPaste && !simulationInteractionLocked);
    ui->actionGModelShowConnect->setEnabled(opened && canConnect && !simulationInteractionLocked);
    ui->actionViewGroup->setEnabled(opened && canGroup && !simulationInteractionLocked);
    ui->actionEditGroup->setEnabled(opened && canGroup && !simulationInteractionLocked);
    ui->actionViewUngroup->setEnabled(opened && canUngroup && !simulationInteractionLocked);
    ui->actionEditUngroup->setEnabled(opened && canUngroup && !simulationInteractionLocked);
    ui->actionEditReplace->setEnabled(opened && !simulationInteractionLocked);

    // sliders
    // Keep zoom/edit navigation control locked while simulation remains active.
    ui->horizontalSlider_ZoomGraphical->setEnabled(opened && !simulationInteractionLocked);
    if (_modelWasOpened && !opened) {
        _clearModelEditors();
    }

    //slider animation speed
    ui->horizontalSliderAnimationSpeed->setEnabled(running && !paused);

    ui->actionSelectAll->setEnabled(opened && !simulationInteractionLocked);

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
        } else if (index == CONST.TabCentralSimulationIndex) {
            index = ui->tabWidgetSimulation->currentIndex();
            if (index == CONST.TabSimulationBreakpointsIndex) {
                _actualizeDebugBreakpoints(true);
            } else if (index == CONST.TabSimulationEntitiesIndex) {
                _actualizeDebugEntities(true);
            } else if (index == CONST.TabSimulationVariablesIndex) {
                _actualizeDebugVariables(true);
            }
        } else if (index == CONST.TabCentralReportsIndex) {
            index = ui->tabWidgetReports->currentIndex();
            if (index == CONST.TabReportResultIndex) {
                _actualizeReportsResultsTable();
            } else if (index == CONST.TabReportPlotIndex) {
                _actualizeReportsPlots();
            }
        }
    } else {
        ui->actionAnimateCounter->setChecked(false);
        ui->actionAnimateVariable->setChecked(false);
        ui->actionAnimateSimulatedTime->setChecked(false);
        ui->actionAnimateExpression->setChecked(false);
        ui->actionAnimateResource->setChecked(false);
        ui->actionAnimateQueue->setChecked(false);
        ui->actionAnimateStation->setChecked(false);
        ui->actionAnimateEntity->setChecked(false);
        ui->actionAnimateEvent->setChecked(false);
        ui->actionAnimateAttribute->setChecked(false);
        ui->actionAnimateStatistics->setChecked(false);
        ui->actionAnimatePlot->setChecked(false);
    }
}

void MainWindow::_prepareReportsResultsTable() {
    QStringList headers;
    headers << tr("Type")
            << tr("ParentType")
            << tr("ParentName")
            << tr("Name")
            << tr("NumElements")
            << tr("Min")
            << tr("Max")
            << tr("Average")
            << tr("Variance")
            << tr("StdDev")
            << tr("VarCoef")
            << tr("HalfWidthCI")
            << tr("ConfidenceLevel");
    ui->tableWidget_ReportsResults->setHorizontalHeaderLabels(headers);
    ui->tableWidget_ReportsResults->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget_ReportsResults->verticalHeader()->setVisible(false);
    ui->tableWidget_ReportsResults->setSortingEnabled(false);
    _clearReportsResultsTable();
}

void MainWindow::_clearReportsResultsTable() {
    ui->tableWidget_ReportsResults->setRowCount(0);
}

void MainWindow::_prepareReportsPlots() {
    if (ui->tabReportsPlots->layout() == nullptr) {
        QVBoxLayout* outerLayout = new QVBoxLayout(ui->tabReportsPlots);
        outerLayout->setContentsMargins(2, 2, 2, 2);

        QScrollArea* scrollArea = new QScrollArea(ui->tabReportsPlots);
        scrollArea->setObjectName("scrollArea_ReportsPlots");
        scrollArea->setWidgetResizable(true);

        QWidget* content = new QWidget(scrollArea);
        content->setObjectName("widget_ReportsPlotsContent");
        QVBoxLayout* contentLayout = new QVBoxLayout(content);
        contentLayout->setContentsMargins(8, 8, 8, 8);
        contentLayout->setSpacing(12);

        scrollArea->setWidget(content);
        outerLayout->addWidget(scrollArea);
    }

    _clearReportsPlots();
}

void MainWindow::_clearReportsPlots() {
    QWidget* content = ui->tabReportsPlots->findChild<QWidget*>("widget_ReportsPlotsContent");
    if (content == nullptr || content->layout() == nullptr) {
        return;
    }

    QLayout* layout = content->layout();
    while (QLayoutItem* child = layout->takeAt(0)) {
        if (QWidget* widget = child->widget()) {
            delete widget;
        }
        delete child;
    }
}

void MainWindow::_actualizeReportsPlots() {
    _clearReportsPlots();

    QWidget* content = ui->tabReportsPlots->findChild<QWidget*>("widget_ReportsPlotsContent");
    if (content == nullptr || content->layout() == nullptr) {
        return;
    }

    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(content->layout());
    if (layout == nullptr) {
        return;
    }

    if (simulator == nullptr || simulator->getModelManager() == nullptr || simulator->getModelManager()->current() == nullptr) {
        QLabel* emptyLabel = new QLabel(tr("No simulation results are available."), content);
        layout->addWidget(emptyLabel);
        layout->addStretch();
        return;
    }

    ModelSimulation* simulation = simulator->getModelManager()->current()->getSimulation();
    const List<ModelDataDefinition*>* aggregates = simulation != nullptr
            ? simulation->getSimulationStatisticsAggregates()
            : nullptr;
    if (aggregates == nullptr) {
        QLabel* emptyLabel = new QLabel(tr("No simulation results are available."), content);
        layout->addWidget(emptyLabel);
        layout->addStretch();
        return;
    }

    QMap<QString, QVector<ReportPlotItem>> groupedItems;
    for (ModelDataDefinition* data : *aggregates->list()) {
        QString category;
        ReportPlotItem item;
        if (fillPlotItem(data, &category, &item)) {
            groupedItems[category].append(item);
        }
    }

    if (groupedItems.isEmpty()) {
        QLabel* emptyLabel = new QLabel(tr("No plottable simulation results are available."), content);
        layout->addWidget(emptyLabel);
        layout->addStretch();
        return;
    }

    QLabel* note = new QLabel(tr("Each chart groups one result category. Whiskers show min/max; the red center line shows the average."), content);
    note->setWordWrap(true);
    layout->addWidget(note);

    for (auto it = groupedItems.cbegin(); it != groupedItems.cend(); ++it) {
        QLabel* title = new QLabel(it.key(), content);
        QFont titleFont = title->font();
        titleFont.setBold(true);
        title->setFont(titleFont);
        layout->addWidget(title);

        ResultsBoxPlotWidget* plot = new ResultsBoxPlotWidget(it.value(), content);
        plot->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        layout->addWidget(plot);
    }
    layout->addStretch();
}

void MainWindow::_actualizeReportsResultsTable() {
    _clearReportsResultsTable();
    if (simulator == nullptr || simulator->getModelManager() == nullptr || simulator->getModelManager()->current() == nullptr) {
        return;
    }
    ModelSimulation* simulation = simulator->getModelManager()->current()->getSimulation();
    if (simulation == nullptr) {
        return;
    }
    const List<ModelDataDefinition*>* aggregates = simulation->getSimulationStatisticsAggregates();
    if (aggregates == nullptr) {
        return;
    }

    auto setCell = [this](int row, int col, const QString& value) {
        ui->tableWidget_ReportsResults->setItem(row, col, new QTableWidgetItem(value));
    };
    auto numberToQString = [](double value) {
        return QString::fromStdString(std::to_string(value));
    };

    int row = 0;
    for (ModelDataDefinition* data : *aggregates->list()) {
        if (data == nullptr) {
            continue;
        }
        ui->tableWidget_ReportsResults->insertRow(row);
        setCell(row, 0, QString::fromStdString(data->getClassname()));
        setCell(row, 3, QString::fromStdString(data->getName()));

        ModelDataDefinition* parent = nullptr;
        if (StatisticsCollector* collector = dynamic_cast<StatisticsCollector*>(data)) {
            parent = collector->getParent();
            Statistics_if* stats = collector->getStatistics();
            if (stats != nullptr) {
                setCell(row, 4, QString::number(stats->numElements()));
                setCell(row, 5, numberToQString(stats->min()));
                setCell(row, 6, numberToQString(stats->max()));
                setCell(row, 7, numberToQString(stats->average()));
                setCell(row, 8, numberToQString(stats->variance()));
                setCell(row, 9, numberToQString(stats->stddeviation()));
                setCell(row, 10, numberToQString(stats->variationCoef()));
                setCell(row, 11, numberToQString(stats->halfWidthConfidenceInterval()));
                setCell(row, 12, numberToQString(stats->confidenceLevel()));
            }
        } else if (Counter* counter = dynamic_cast<Counter*>(data)) {
            parent = counter->getParent();
            setCell(row, 4, numberToQString(counter->getCountValue()));
        }

        if (parent != nullptr) {
            setCell(row, 1, QString::fromStdString(parent->getClassname()));
            setCell(row, 2, QString::fromStdString(parent->getName()));
        }
        row++;
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
    // Keep this wrapper as part of the final compatibility façade from Phase 9 refactor.
    if (_editCommandController != nullptr) {
        _editCommandController->helpCopy();
    }
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
        // Schedule data-definition synchronization outside this check stack to avoid scene teardown reentrancy.
        if (scene != nullptr) {
            scene->requestGraphicalDataDefinitionsSync();
        }
        // Mensagem de sucesso
        if (success) {
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
    if(ui->actionAnimateExpression->isChecked()) alreadyChecked++;
    if(ui->actionAnimateResource->isChecked()) alreadyChecked++;
    if(ui->actionAnimateQueue->isChecked()) alreadyChecked++;
    if(ui->actionAnimateStation->isChecked()) alreadyChecked++;
    if(ui->actionAnimateEntity->isChecked()) alreadyChecked++;
    if(ui->actionAnimateEvent->isChecked()) alreadyChecked++;
    if(ui->actionAnimateAttribute->isChecked()) alreadyChecked++;
    if(ui->actionAnimateStatistics->isChecked()) alreadyChecked++;
    if(ui->actionAnimatePlot->isChecked()) alreadyChecked++;
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
    ui->actionAnimateExpression->setChecked(false);
    ui->actionAnimateResource->setChecked(false);
    ui->actionAnimateQueue->setChecked(false);
    ui->actionAnimateStation->setChecked(false);
    ui->actionAnimateEntity->setChecked(false);
    ui->actionAnimateEvent->setChecked(false);
    ui->actionAnimateAttribute->setChecked(false);
    ui->actionAnimateStatistics->setChecked(false);
    ui->actionAnimatePlot->setChecked(false);
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
