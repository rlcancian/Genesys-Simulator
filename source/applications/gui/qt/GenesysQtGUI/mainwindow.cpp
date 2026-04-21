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
#include "kernel/simulator/SinkModelComponent.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Counter.h"
#include "kernel/simulator/StatisticsCollector.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/ComponentManager.h"
#include "kernel/simulator/ModelDataManager.h"
// GUI
#include "graphicals/ModelGraphicsScene.h"
#include "TraitsGUI.h"
#include "graphicals/GraphicalConnection.h"
#include "graphicals/GraphicalModelDataDefinition.h"
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
#include "extensions/GuiExtensionContracts.h"
#include "extensions/GuiExtensionManager.h"
#include "extensions/GuiExtensionPluginCatalog.h"
#include "services/ModelLanguageSynchronizer.h"
#include "services/GraphvizModelExporter.h"
#include "services/CppModelExporter.h"
#include "services/GraphicalModelSerializer.h"
#include "services/GraphicalModelBuilder.h"
#include "UtilGUI.h"
#include "guithememanager.h"
// PropEditor
#include "propertyeditor/qtpropertybrowser/qttreepropertybrowser.h"
#include "animations/AnimationVariable.h"
#include "systempreferences.h"
//#include "actions/PasteUndoCommand.h"
//#include "actions/DeleteUndoCommand.h"
// @TODO: Should NOT be hardcoded!!! (Used to visualize variables)
#include "plugins/data/DiscreteProcessing/Variable.h"
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
#include <QFileInfo>
#include <QDateTime>
#include <QEventLoop>
#include <QTemporaryFile>
#include <Qt>
#include <QGraphicsPixmapItem>
#include <QKeySequence>
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
#include <QMenu>
#include <QPainter>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>
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

std::vector<std::string> collectLoadedModelPluginIds(const Simulator* simulator) {
	std::vector<std::string> ids;
	if (simulator == nullptr || simulator->getPluginManager() == nullptr) {
		return ids;
	}

	PluginManager* pluginManager = simulator->getPluginManager();
	ids.reserve(pluginManager->size());
	for (unsigned int index = 0; index < pluginManager->size(); ++index) {
		Plugin* plugin = pluginManager->getAtRank(index);
		if (plugin == nullptr || plugin->getPluginInfo() == nullptr) {
			continue;
		}
		const std::string pluginTypename = plugin->getPluginInfo()->getPluginTypename();
		if (!pluginTypename.empty()) {
			ids.push_back(pluginTypename);
		}
	}
	return ids;
}
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    _actionModelPrevious = new QAction(tr("Previous Model"), this);
    _actionModelPrevious->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    connect(_actionModelPrevious, &QAction::triggered, this, [this]() { _activatePreviousModel(); });
    _actionModelNext = new QAction(tr("Next Model"), this);
    _actionModelNext->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    connect(_actionModelNext, &QAction::triggered, this, [this]() { _activateNextModel(); });
    _actionOpenSelectedSubmodel = new QAction(tr("Open Submodel"), this);
    _actionOpenSelectedSubmodel->setToolTip(tr("Open a graphical tab scoped to the selected item's internal model level."));
    connect(_actionOpenSelectedSubmodel, &QAction::triggered, this, [this]() { _openSelectedSubmodel(); });
    _menuOpenRecent = new QMenu(tr("Open Recent..."), this);
    ui->menuModel->insertAction(ui->actionModelOpen, _actionModelPrevious);
    ui->menuModel->insertAction(ui->actionModelOpen, _actionModelNext);
    ui->menuModel->insertAction(ui->actionModelOpen, _actionOpenSelectedSubmodel);
    ui->menuModel->insertSeparator(ui->actionModelOpen);
    ui->menuModel->insertMenu(ui->actionModelOpen, _menuOpenRecent);

    // The View/Show menu relies on the native checkmark indicator; menu icons made the checked
    // state hard to distinguish, so keep those actions text/checkmark-only and define explicit
    // defaults for the data-definition visibility categories.
    ui->actionShowInternalElements->setIconVisibleInMenu(false);
    ui->actionShowEditableElements->setIconVisibleInMenu(false);
    ui->actionShowAttachedElements->setIconVisibleInMenu(false);
    ui->actionShowRecursiveElements->setIconVisibleInMenu(false);
    ui->actionShowInternalElements->setChecked(false);
    ui->actionShowEditableElements->setChecked(true);
    ui->actionShowAttachedElements->setChecked(false);
    ui->actionShowRecursiveElements->setChecked(true);
    ui->checkBox_ShowInternals->setChecked(false);
    ui->checkBox_ShowEditableElements->setChecked(true);
    ui->checkBox_ShowElements->setChecked(false);
    ui->checkBox_ShowRecursive->setChecked(true);

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
                                                                     ui->checkBox_ShowEditableElements,
                                                                     ui->checkBox_ShowElements,
                                                                     ui->checkBox_ShowRecursive,
                                                                     ui->checkBox_ShowLevels,
                                                                     // Keep synchronization behavior unchanged via a narrow callback dependency.
                                                                     [this]() { return this->_setSimulationModelBasedOnText(); });
    _cppModelExporter = std::make_unique<CppModelExporter>(simulator, ui->plainTextEditCppCode);
    simulator->getTraceManager()->setTraceLevel(SystemPreferences::traceLevel());
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
    _configureModelGraphicsView(ui->graphicsView);
    _initializeModelTabs();
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
    // Initialize the Phase 4 trace controller after trace output widgets are available.
    _traceConsoleController = std::make_unique<TraceConsoleController>(ui->textEdit_Console,
                                                                        ui->textEdit_Simulation,
                                                                        ui->textEdit_Reports);
    // Initialize the Phase 5 plugin-catalog controller after simulator and plugin-tree dependencies are ready.
    _pluginCatalogController = std::make_unique<PluginCatalogController>(simulator,
                                                                         ui->treeWidget_Plugins,
                                                                         ui->TextCodeEditor,
                                                                         _pluginCategoryColor);
    //
    // property editor
    ui->treeViewPropertyEditor->setAlternatingRowColors(true);
    _rebuildViewDependentControllers();
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
    _refreshRecentModelsMenu();
    GuiThemeManager::applyModelGraphicsTheme(ui->graphicsView);
    if (SystemPreferences::autoLoadPlugins()) {
        PluginInsertionOptions options;
        // The main window is still being built here, so autoload must not open install dialogs.
        // Missing dependency diagnostics are recorded and handled later by DialogPluginManager.
        simulator->getPluginManager()->autoInsertPlugins(_autoLoadPluginsFilename.toStdString(), true, options);
        // now complete the information
        for (unsigned int i = 0; i < simulator->getPluginManager()->size(); i++) {
            //@TODO: now it's the opportunity to adjust template
            _insertPluginUI(simulator->getPluginManager()->getAtRank(i));
        }
        _pluginCatalogController->applyCategoryExpansionPolicy();
    }
    if (SystemPreferences::startMaximized()) {
        // another try to start maximized (it should not be that hard)
        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
        this->resize(screenGeometry.width(), screenGeometry.height());
    }
    if (SystemPreferences::startupModelMode() == SystemPreferences::StartupModelMode::NewModel) {
        this->on_actionModelNew_triggered();
    } else if (SystemPreferences::startupModelMode() == SystemPreferences::StartupModelMode::OpenSpecificModel ||
               SystemPreferences::startupModelMode() == SystemPreferences::StartupModelMode::OpenLastModel) {
        const std::string fileName = SystemPreferences::startupModelMode() == SystemPreferences::StartupModelMode::OpenLastModel
                                         ? SystemPreferences::lastModelFilename()
                                         : SystemPreferences::modelfilename();
        if (!fileName.empty()) {
            Model* model = this->_loadGraphicalModel(fileName);
            if (model != nullptr) {
                _loaded = true;
                _initUiForNewModel(model);
                _actualizeModelTextHasChanged(false);
                _graphicalModelHasChanged = false;
                model->setHasChanged(false);
                SystemPreferences::pushRecentModelFile(fileName);
                SystemPreferences::save();
                _refreshRecentModelsMenu();
            } else {
                qWarning() << "Could not open startup model from preferences:" << QString::fromStdString(fileName);
            }
        }
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
    QTimer::singleShot(0, this, [this]() {
        if (_dialogUtilityController == nullptr || simulator == nullptr || simulator->getPluginManager() == nullptr) {
            return;
        }
        List<PluginLoadIssue>* issues = simulator->getPluginManager()->getPluginLoadIssues();
        if (issues != nullptr && !issues->empty()) {
            _dialogUtilityController->onActionSimulatorsPluginManagerTriggered(true);
        }
    });
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

MainWindow::GraphicalModelViewContext* MainWindow::_activeViewContext() const {
    return (ui != nullptr) ? _contextForView(ui->graphicsView) : nullptr;
}

MainWindow::GraphicalModelViewContext* MainWindow::_contextForView(ModelGraphicsView* graphicsView) const {
    auto it = _viewContextsByGraphicsView.find(graphicsView);
    return it != _viewContextsByGraphicsView.end() ? it->second.get() : nullptr;
}

MainWindow::GraphicalModelViewContext* MainWindow::_rootContextForModel(Model* model) const {
    auto it = _rootViewContextsByModel.find(model);
    return it != _rootViewContextsByModel.end() ? it->second : nullptr;
}

MainWindow::GraphicalModelViewContext* MainWindow::_ensureRootModelViewContext(Model* model,
                                                                                ModelGraphicsView* graphicsView) {
    if (model == nullptr || graphicsView == nullptr) {
        return nullptr;
    }

    auto existing = _viewContextsByGraphicsView.find(graphicsView);
    if (existing == _viewContextsByGraphicsView.end()) {
        auto context = std::make_unique<GraphicalModelViewContext>();
        context->graphicsView = graphicsView;
        context->kind = GraphicalModelViewContextKind::RootModel;
        existing = _viewContextsByGraphicsView.emplace(graphicsView, std::move(context)).first;
    }

    GraphicalModelViewContext* context = existing->second.get();
    context->kind = GraphicalModelViewContextKind::RootModel;
    context->rootModel = model;
    context->modelLevel = model->getLevel();
    context->ownerComponent = nullptr;
    context->ownerDataDefinition = nullptr;
    context->graphicsView = graphicsView;
    if (graphicsView->getScene() != nullptr) {
        graphicsView->getScene()->setModelLevelFilter(context->modelLevel);
    }
    _rootViewContextsByModel[model] = context;
    return context;
}

ModelGraphicsView* MainWindow::_ensureSubmodelViewContext(ModelDataDefinition* owner) {
    if (owner == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr || _modelGraphicsTabs == nullptr) {
        return nullptr;
    }

    Model* rootModel = simulator->getModelManager()->current();
    if (rootModel == nullptr || !_canOpenSubmodelFor(owner)) {
        return nullptr;
    }

    for (const auto& contextEntry : _viewContextsByGraphicsView) {
        GraphicalModelViewContext* context = contextEntry.second.get();
        if (context != nullptr
            && context->kind == GraphicalModelViewContextKind::Submodel
            && context->rootModel == rootModel
            && context->ownerDataDefinition == owner) {
            ModelGraphicsView* existingView = context->graphicsView;
            if (existingView != nullptr) {
                const int tabIndex = _modelGraphicsTabs->indexOf(existingView);
                if (tabIndex >= 0) {
                    QSignalBlocker blocker(_modelGraphicsTabs);
                    _changingModelTabProgrammatically = true;
                    _modelGraphicsTabs->setCurrentIndex(tabIndex);
                    _changingModelTabProgrammatically = false;
                }
                _activateModelGraphicsView(existingView);
                return existingView;
            }
        }
    }

    ModelGraphicsView* graphicsView = _createModelGraphicsView(_modelGraphicsTabs);
    auto context = std::make_unique<GraphicalModelViewContext>();
    context->kind = GraphicalModelViewContextKind::Submodel;
    context->rootModel = rootModel;
    context->modelLevel = static_cast<unsigned int>(owner->getId());
    context->ownerDataDefinition = owner;
    context->ownerComponent = dynamic_cast<ModelComponent*>(owner);
    context->graphicsView = graphicsView;
    if (graphicsView->getScene() != nullptr) {
        graphicsView->getScene()->setModelLevelFilter(context->modelLevel);
    }
    GraphicalModelViewContext* rawContext = context.get();
    _viewContextsByGraphicsView.emplace(graphicsView, std::move(context));
    _modelsByGraphicsView[graphicsView] = rootModel;

    const int tabIndex = _modelGraphicsTabs->addTab(graphicsView, _viewContextTitle(*rawContext));
    if (tabIndex >= 0) {
        QSignalBlocker blocker(_modelGraphicsTabs);
        _changingModelTabProgrammatically = true;
        _modelGraphicsTabs->setCurrentIndex(tabIndex);
        _changingModelTabProgrammatically = false;
    }

    _activateModelGraphicsView(graphicsView);
    _generateGraphicalModelFromModel();
    _updateModelTabs();
    _actualizeActions();
    _actualizeTabPanes();
    return graphicsView;
}

void MainWindow::_closeSubmodelViewContext(ModelGraphicsView* graphicsView) {
    if (graphicsView == nullptr || _modelGraphicsTabs == nullptr) {
        return;
    }

    GraphicalModelViewContext* context = _contextForView(graphicsView);
    if (context == nullptr || context->kind != GraphicalModelViewContextKind::Submodel) {
        return;
    }

    const int tabIndex = _modelGraphicsTabs->indexOf(graphicsView);
    if (tabIndex >= 0) {
        _modelGraphicsTabs->removeTab(tabIndex);
    }
    _modelsByGraphicsView.erase(graphicsView);
    _removeViewContext(graphicsView);
    graphicsView->deleteLater();
    _updateModelTabs();
}

void MainWindow::_removeSubmodelViewContextsForModel(Model* model) {
    if (model == nullptr) {
        return;
    }

    QList<ModelGraphicsView*> submodelViews;
    for (const auto& contextEntry : _viewContextsByGraphicsView) {
        GraphicalModelViewContext* context = contextEntry.second.get();
        if (context != nullptr
            && context->kind == GraphicalModelViewContextKind::Submodel
            && context->rootModel == model
            && context->graphicsView != nullptr) {
            submodelViews.append(context->graphicsView);
        }
    }
    for (ModelGraphicsView* graphicsView : submodelViews) {
        _closeSubmodelViewContext(graphicsView);
    }
}

void MainWindow::_removeViewContext(ModelGraphicsView* graphicsView) {
    auto it = _viewContextsByGraphicsView.find(graphicsView);
    if (it == _viewContextsByGraphicsView.end()) {
        return;
    }
    GraphicalModelViewContext* context = it->second.get();
    if (context != nullptr && context->kind == GraphicalModelViewContextKind::RootModel && context->rootModel != nullptr) {
        _rootViewContextsByModel.erase(context->rootModel);
    }
    _viewContextsByGraphicsView.erase(it);
}

QString MainWindow::_viewContextTitle(const GraphicalModelViewContext& context) const {
    switch (context.kind) {
    case GraphicalModelViewContextKind::RootModel:
        return _modelDisplayName(context.rootModel);
    case GraphicalModelViewContextKind::Submodel:
        if (!context.explicitTitle.isEmpty()) {
            return context.explicitTitle;
        }
        if (context.ownerComponent != nullptr) {
            return tr("%1 > %2").arg(_modelDisplayName(context.rootModel),
                                     QString::fromStdString(context.ownerComponent->getName()));
        }
        if (context.ownerDataDefinition != nullptr) {
            return tr("%1 > %2").arg(_modelDisplayName(context.rootModel),
                                     QString::fromStdString(context.ownerDataDefinition->getName()));
        }
        return tr("%1 > Level %2").arg(_modelDisplayName(context.rootModel)).arg(context.modelLevel);
    }
    return _modelDisplayName(context.rootModel);
}

bool MainWindow::_viewContextBelongsToOpenModel(const GraphicalModelViewContext& context) const {
    return simulator != nullptr
           && simulator->getModelManager() != nullptr
           && simulator->getModelManager()->hasModel(context.rootModel);
}

ModelDataDefinition* MainWindow::_selectedSubmodelOwner() const {
    if (ui == nullptr || ui->graphicsView == nullptr || ui->graphicsView->getScene() == nullptr) {
        return nullptr;
    }

    const QList<QGraphicsItem*> selectedItems = ui->graphicsView->getScene()->selectedItems();
    if (selectedItems.size() != 1) {
        return nullptr;
    }

    QGraphicsItem* item = selectedItems.first();
    if (auto* graphicalComponent = dynamic_cast<GraphicalModelComponent*>(item)) {
        return graphicalComponent->getComponent();
    }
    if (auto* graphicalDataDefinition = dynamic_cast<GraphicalModelDataDefinition*>(item)) {
        return graphicalDataDefinition->getDataDefinition();
    }
    return nullptr;
}

bool MainWindow::_canOpenSubmodelFor(ModelDataDefinition* owner) const {
    if (owner == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr) {
        return false;
    }

    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        return false;
    }

    const unsigned int submodelLevel = static_cast<unsigned int>(owner->getId());
    if (model->getComponentManager() != nullptr) {
        for (ModelComponent* component : *model->getComponentManager()->getAllComponents()) {
            if (component != nullptr && component->getLevel() == submodelLevel) {
                return true;
            }
        }
    }

    if (model->getDataManager() != nullptr) {
        for (const std::string& dataTypename : model->getDataManager()->getDataDefinitionClassnames()) {
            List<ModelDataDefinition*>* definitions = model->getDataManager()->getDataDefinitionList(dataTypename);
            if (definitions == nullptr) {
                continue;
            }
            for (ModelDataDefinition* dataDefinition : *definitions->list()) {
                if (dataDefinition != nullptr && dataDefinition->getLevel() == submodelLevel) {
                    return true;
                }
            }
        }
    }
    return false;
}

void MainWindow::_openSelectedSubmodel() {
    _ensureSubmodelViewContext(_selectedSubmodelOwner());
}

void MainWindow::_syncCurrentModelDocumentState() {
    if (simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }
    Model* model = simulator->getModelManager()->current();
    if (model == nullptr) {
        return;
    }

    // Multi-model editing keeps a lightweight document state beside each kernel model.
    // The legacy booleans still exist because several extracted controllers receive pointers
    // to them, but they now mirror only the currently selected model.
    _modelFilenames[model] = _modelfilename;
    if (ui != nullptr && ui->TextCodeEditor != nullptr) {
        _modelTextContents[model] = ui->TextCodeEditor->toPlainText();
    }
    _modelTextHasChanged[model] = _textModelHasChanged;
    _modelGraphicalHasChanged[model] = _graphicalModelHasChanged;
}

void MainWindow::_restoreModelDocumentState(Model* model) {
    if (model == nullptr) {
        _modelfilename.clear();
        _textModelHasChanged = false;
        _graphicalModelHasChanged = false;
        return;
    }

    _modelfilename = _modelFilenames[model];
    _textModelHasChanged = _modelTextHasChanged[model];
    _graphicalModelHasChanged = _modelGraphicalHasChanged[model];

    if (ui != nullptr && ui->TextCodeEditor != nullptr && _modelTextContents.find(model) != _modelTextContents.end()) {
        const QSignalBlocker blocker(ui->TextCodeEditor);
        ui->TextCodeEditor->setPlainText(_modelTextContents[model]);
    }
}

QString MainWindow::_modelDisplayName(Model* model) const {
    if (model == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr) {
        return tr("Model");
    }
    if (model->getInfos() != nullptr && !model->getInfos()->getName().empty()) {
        return QString::fromStdString(model->getInfos()->getName());
    }
    const int index = simulator->getModelManager()->indexOf(model);
    return index >= 0 ? tr("Model %1").arg(index + 1) : tr("Model");
}

QString MainWindow::_modelSaveBaseFilename(QString filename) const {
    filename = filename.trimmed();
    if (filename.endsWith(".gen", Qt::CaseInsensitive) || filename.endsWith(".gui", Qt::CaseInsensitive)) {
        filename.chop(4);
    }
    return filename;
}

bool MainWindow::_modelHasPendingChanges(Model* model) const {
    if (model == nullptr) {
        return false;
    }
    const auto textIt = _modelTextHasChanged.find(model);
    const auto graphIt = _modelGraphicalHasChanged.find(model);
    return (textIt != _modelTextHasChanged.end() && textIt->second)
           || (graphIt != _modelGraphicalHasChanged.end() && graphIt->second)
           || model->hasChanged();
}

bool MainWindow::_saveCurrentModel(bool promptForFilename) {
    if (simulator == nullptr || simulator->getModelManager() == nullptr || simulator->getModelManager()->current() == nullptr) {
        return false;
    }

    _syncCurrentModelDocumentState();
    Model* model = simulator->getModelManager()->current();
    QString baseFileName = _modelSaveBaseFilename(_modelFilenames[model]);

    if (promptForFilename || baseFileName.isEmpty()) {
        QString selectedFileName = QFileDialog::getSaveFileName(this,
                                                                QObject::tr("Save Model"),
                                                                baseFileName,
                                                                QObject::tr("Genesys Model (*.gen)"),
                                                                nullptr,
                                                                QFileDialog::DontUseNativeDialog);
        if (selectedFileName.isEmpty()) {
            return false;
        }
        baseFileName = _modelSaveBaseFilename(selectedFileName);
    }

    _insertCommandInConsole("save " + baseFileName.toStdString());

    const QString textFilename = baseFileName + ".gen";
    QFile saveFile(textFilename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, QObject::tr("Unable to access file to save"), saveFile.errorString());
        return false;
    }

    const QString textToSave = _modelTextContents[model].isNull() ? ui->TextCodeEditor->toPlainText() : _modelTextContents[model];
    if (!_saveTextModel(&saveFile, textToSave)) {
        saveFile.close();
        QMessageBox::warning(this, "Save Model", "Error while saving model text.");
        return false;
    }
    saveFile.close();

    const QString graphicalFilename = baseFileName + ".gui";
    if (!_saveGraphicalModel(graphicalFilename)) {
        QMessageBox::warning(this, "Save Model", "Error while saving graphical model.");
        return false;
    }

    _modelFilenames[model] = baseFileName;
    _modelfilename = baseFileName;
    _modelTextContents[model] = textToSave;
    _modelTextHasChanged[model] = false;
    _modelGraphicalHasChanged[model] = false;
    _textModelHasChanged = false;
    _graphicalModelHasChanged = false;
    model->setHasChanged(false);

    if (!_setSimulationModelBasedOnText()) {
        QMessageBox::warning(this, "Save Model", "Model was saved, but the simulation model could not be synchronized.");
        return false;
    }

    SystemPreferences::pushRecentModelFile(graphicalFilename.toStdString());
    SystemPreferences::save();
    if (ui != nullptr && ui->graphicsView != nullptr && ui->graphicsView->getScene() != nullptr &&
        ui->graphicsView->getScene()->getUndoStack() != nullptr) {
        ui->graphicsView->getScene()->getUndoStack()->clear();
    }
    _updateModelTabs();
    _actualizeActions();
    return true;
}

bool MainWindow::_saveModelWithActivation(Model* model) {
    if (model == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr) {
        return false;
    }
    _syncCurrentModelDocumentState();
    if (!simulator->getModelManager()->setCurrent(model)) {
        return false;
    }
    _ensureModelTab(model);
    _restoreModelDocumentState(model);
    _actualizeTabPanes();
    return _saveCurrentModel(false);
}

bool MainWindow::_confirmCloseModel(Model* model) {
    if (model == nullptr) {
        return true;
    }
    _syncCurrentModelDocumentState();
    if (!_modelHasPendingChanges(model)) {
        return true;
    }

    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Close Model",
        tr("%1 has changed. Do you want to save it?").arg(_modelDisplayName(model)),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);
    if (reply == QMessageBox::Cancel) {
        return false;
    }
    if (reply == QMessageBox::Yes) {
        return _saveModelWithActivation(model) && !_modelHasPendingChanges(model);
    }
    return true;
}

void MainWindow::_clearModelGraphicsViewForClose(ModelGraphicsView* graphicsView) {
    if (graphicsView == nullptr || graphicsView->getScene() == nullptr) {
        return;
    }
    ModelGraphicsScene* scene = graphicsView->getScene();
    scene->grid()->clear();
    if (scene->getUndoStack() != nullptr) {
        scene->getUndoStack()->clear();
    }
    scene->clearAnimationsQueue();
    scene->getGraphicalModelComponents()->clear();
    scene->getGraphicalConnections()->clear();
    scene->getGraphicalModelDataDefinitions()->clear();
    scene->getGraphicalDiagramsConnections()->clear();
    scene->getAllComponents()->clear();
    scene->getAllConnections()->clear();
    scene->getAllDataDefinitions()->clear();
    scene->getAllGraphicalDiagramsConnections()->clear();
    scene->clearAnimations();
    scene->clear();
    graphicsView->clear();
}

bool MainWindow::_closeModel(Model* model) {
    if (model == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr ||
        !simulator->getModelManager()->hasModel(model)) {
        return false;
    }

    if (!simulator->getModelManager()->setCurrent(model)) {
        return false;
    }
    _ensureModelTab(model);
    _restoreModelDocumentState(model);
    if (!_confirmCloseModel(model)) {
        _actualizeActions();
        _updateModelTabs();
        return false;
    }

    _disconnectSceneSignals("_closeModel(begin)");
    _insertCommandInConsole("close");
    ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();

    ModelGraphicsView* closingView = nullptr;
    auto viewIt = _modelGraphicsViews.find(model);
    if (viewIt != _modelGraphicsViews.end()) {
        closingView = viewIt->second;
    }
    _clearModelGraphicsViewForClose(closingView);
    _removeModelTab(model);
    simulator->getModelManager()->remove(model);
    ui->progressBarSimulation->setValue(0);
    ui->actionActivateGraphicalSimulation->setChecked(false);

    if (Model* current = simulator->getModelManager()->current()) {
        _ensureModelTab(current);
        _restoreModelDocumentState(current);
    } else {
        _clearModelEditors();
        _restoreModelDocumentState(nullptr);
    }

    _updateModelTabs();
    _connectSceneSignals();
    _actualizeActions();
    _actualizeTabPanes();
    return true;
}

void MainWindow::_initializeModelTabs() {
    if (_modelGraphicsTabs != nullptr || ui == nullptr || ui->graphicsView == nullptr) {
        return;
    }

    QSplitter* splitter = qobject_cast<QSplitter*>(ui->graphicsView->parentWidget());
    if (splitter == nullptr) {
        qWarning() << "Could not create model graphics tabs because graphicsView parent is not a QSplitter";
        return;
    }

    const int graphicsIndex = splitter->indexOf(ui->graphicsView);
    _modelGraphicsTabs = new QTabWidget(splitter);
    _modelGraphicsTabs->setObjectName("tabWidgetModelGraphics");
    _modelGraphicsTabs->setTabsClosable(true);
    _modelGraphicsTabs->setMovable(false);
    _modelGraphicsTabs->setDocumentMode(true);
    _modelGraphicsTabs->setMinimumSize(ui->graphicsView->minimumSize());
    _modelGraphicsTabs->setSizePolicy(ui->graphicsView->sizePolicy());

    splitter->insertWidget(graphicsIndex, _modelGraphicsTabs);
    _modelGraphicsTabs->addTab(ui->graphicsView, tr("No model"));

    connect(_modelGraphicsTabs, &QTabWidget::currentChanged, this, [this](int index) {
        _activateModelTab(index);
    });
    connect(_modelGraphicsTabs, &QTabWidget::tabCloseRequested, this, [this](int index) {
        if (_modelGraphicsTabs == nullptr || index < 0) {
            return;
        }
        if (ModelGraphicsView* graphicsView = dynamic_cast<ModelGraphicsView*>(_modelGraphicsTabs->widget(index))) {
            GraphicalModelViewContext* context = _contextForView(graphicsView);
            if (context != nullptr && context->kind == GraphicalModelViewContextKind::RootModel) {
                _closeModel(context->rootModel);
            } else {
                _closeSubmodelViewContext(graphicsView);
            }
        }
    });
}

void MainWindow::_configureModelGraphicsView(ModelGraphicsView* graphicsView) {
    if (graphicsView == nullptr) {
        return;
    }
    graphicsView->setParentWidget(ui->centralwidget);
    graphicsView->setSimulator(simulator);
    graphicsView->setPropertyEditor(propertyGenesys);
    graphicsView->setPropertyList(propertyList);
    graphicsView->setPropertyEditorUI(propertyEditorUI);
    graphicsView->setComboBox(propertyBox);
    graphicsView->setFrameShape(QFrame::Box);
    graphicsView->setFrameShadow(QFrame::Sunken);
    graphicsView->setLineWidth(3);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

ModelGraphicsView* MainWindow::_createModelGraphicsView(QWidget* parent) {
    ModelGraphicsView* graphicsView = new ModelGraphicsView(parent);
    graphicsView->setObjectName(QString("graphicsView_model_%1").arg(_modelGraphicsViews.size() + 1));
    graphicsView->setMinimumSize(0, 250);
    graphicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _configureModelGraphicsView(graphicsView);
    return graphicsView;
}

void MainWindow::_activateModelGraphicsView(ModelGraphicsView* graphicsView, bool rebuildControllers) {
    if (graphicsView == nullptr || ui == nullptr) {
        return;
    }

    if (ui->graphicsView != nullptr && ui->graphicsView != graphicsView) {
        _disconnectSceneSignals("_activateModelGraphicsView");
        ui->graphicsView->clearEventHandlers();
    }

    ui->graphicsView = graphicsView;
    _configureModelGraphicsView(ui->graphicsView);
    _initModelGraphicsView();
    GuiThemeManager::applyModelGraphicsTheme(ui->graphicsView);

    if (rebuildControllers) {
        _rebuildViewDependentControllers();
    }
}

ModelGraphicsView* MainWindow::_ensureModelTab(Model* model) {
    if (model == nullptr) {
        return ui != nullptr ? ui->graphicsView : nullptr;
    }

    auto existing = _modelGraphicsViews.find(model);
    if (existing != _modelGraphicsViews.end()) {
        ModelGraphicsView* existingView = existing->second;
        _ensureRootModelViewContext(model, existingView);
        if (_modelGraphicsTabs != nullptr) {
            const int tabIndex = _modelGraphicsTabs->indexOf(existingView);
            if (tabIndex >= 0) {
                QSignalBlocker blocker(_modelGraphicsTabs);
                _changingModelTabProgrammatically = true;
                _modelGraphicsTabs->setCurrentIndex(tabIndex);
                _changingModelTabProgrammatically = false;
            }
        }
        _activateModelGraphicsView(existingView);
        return existingView;
    }

    ModelGraphicsView* graphicsView = nullptr;
    if (_modelGraphicsTabs != nullptr && ui->graphicsView != nullptr &&
        _modelsByGraphicsView.find(ui->graphicsView) == _modelsByGraphicsView.end()) {
        graphicsView = ui->graphicsView;
    } else {
        graphicsView = _createModelGraphicsView(_modelGraphicsTabs);
        if (_modelGraphicsTabs != nullptr) {
            _modelGraphicsTabs->addTab(graphicsView, tr("Model"));
        }
    }

    _modelGraphicsViews[model] = graphicsView;
    _modelsByGraphicsView[graphicsView] = model;
    _ensureRootModelViewContext(model, graphicsView);
    if (_modelFilenames.find(model) == _modelFilenames.end()) {
        _modelFilenames[model] = QString();
    }
    if (_modelTextHasChanged.find(model) == _modelTextHasChanged.end()) {
        _modelTextHasChanged[model] = false;
    }
    if (_modelGraphicalHasChanged.find(model) == _modelGraphicalHasChanged.end()) {
        _modelGraphicalHasChanged[model] = false;
    }
    if (_modelTextContents.find(model) == _modelTextContents.end()) {
        _modelTextContents[model] = QString();
    }

    if (_modelGraphicsTabs != nullptr) {
        const int tabIndex = _modelGraphicsTabs->indexOf(graphicsView);
        if (tabIndex >= 0) {
            QSignalBlocker blocker(_modelGraphicsTabs);
            _changingModelTabProgrammatically = true;
            _modelGraphicsTabs->setCurrentIndex(tabIndex);
            _changingModelTabProgrammatically = false;
        }
    }

    _activateModelGraphicsView(graphicsView);
    _updateModelTabs();
    return graphicsView;
}

void MainWindow::_removeModelTab(Model* model) {
    _removeSubmodelViewContextsForModel(model);

    auto it = _modelGraphicsViews.find(model);
    if (it == _modelGraphicsViews.end()) {
        _modelFilenames.erase(model);
        return;
    }

    ModelGraphicsView* graphicsView = it->second;
    _modelGraphicsViews.erase(it);
    _modelsByGraphicsView.erase(graphicsView);
    _removeViewContext(graphicsView);
    _modelFilenames.erase(model);
    _modelTextContents.erase(model);
    _modelTextHasChanged.erase(model);
    _modelGraphicalHasChanged.erase(model);

    if (_modelGraphicsTabs != nullptr) {
        const int tabIndex = _modelGraphicsTabs->indexOf(graphicsView);
        if (tabIndex >= 0) {
            _modelGraphicsTabs->removeTab(tabIndex);
        }
    }

    if (ui->graphicsView == graphicsView) {
        ui->graphicsView = nullptr;
    }
    graphicsView->deleteLater();
}

void MainWindow::_updateModelTabs() {
    if (_modelGraphicsTabs == nullptr || simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }

    for (auto it = _modelGraphicsViews.begin(); it != _modelGraphicsViews.end();) {
        Model* model = it->first;
        if (!simulator->getModelManager()->hasModel(model)) {
            Model* staleModel = model;
            ++it;
            _removeModelTab(staleModel);
        } else {
            ++it;
        }
    }

    for (auto& pair : _modelGraphicsViews) {
        Model* model = pair.first;
        ModelGraphicsView* graphicsView = pair.second;
        const int tabIndex = _modelGraphicsTabs->indexOf(graphicsView);
        if (tabIndex < 0) {
            continue;
        }
        GraphicalModelViewContext* context = _rootContextForModel(model);
        QString title = context != nullptr
                            ? _viewContextTitle(*context)
                            : _modelDisplayName(model);
        const bool dirty = _modelHasPendingChanges(model);
        if (dirty) {
            title += "*";
        }
        _modelGraphicsTabs->setTabText(tabIndex, title);
        _modelGraphicsTabs->setTabToolTip(tabIndex, _modelFilenames[model]);
    }

    for (const auto& contextEntry : _viewContextsByGraphicsView) {
        GraphicalModelViewContext* context = contextEntry.second.get();
        if (context == nullptr
            || context->kind != GraphicalModelViewContextKind::Submodel
            || context->graphicsView == nullptr) {
            continue;
        }
        const int tabIndex = _modelGraphicsTabs->indexOf(context->graphicsView);
        if (tabIndex < 0) {
            continue;
        }
        QString title = _viewContextTitle(*context);
        if (_modelHasPendingChanges(context->rootModel)) {
            title += "*";
        }
        _modelGraphicsTabs->setTabText(tabIndex, title);
        _modelGraphicsTabs->setTabToolTip(tabIndex, tr("Submodel level %1").arg(context->modelLevel));
    }

    Model* current = simulator->getModelManager()->current();
    if (current != nullptr) {
        // Updating tab labels must not force the root model tab back to the foreground.
        // Submodel tabs share the same current kernel model and must remain active while
        // their view-specific controllers are scoped to the selected model level.
        if (_modelGraphicsViews.find(current) == _modelGraphicsViews.end()) {
            _ensureModelTab(current);
        }
    } else if (_modelGraphicsTabs->count() == 0) {
        ModelGraphicsView* graphicsView = _createModelGraphicsView(_modelGraphicsTabs);
        _modelGraphicsTabs->addTab(graphicsView, tr("No model"));
        _activateModelGraphicsView(graphicsView);
    } else if (ui->graphicsView == nullptr) {
        if (ModelGraphicsView* graphicsView = dynamic_cast<ModelGraphicsView*>(_modelGraphicsTabs->widget(0))) {
            _activateModelGraphicsView(graphicsView);
        }
    }

    if (_actionModelPrevious != nullptr) {
        _actionModelPrevious->setEnabled(simulator->getModelManager()->canGoPrevious());
    }
    if (_actionModelNext != nullptr) {
        _actionModelNext->setEnabled(simulator->getModelManager()->canGoNext());
    }
}

void MainWindow::_activateModelTab(int index) {
    if (_changingModelTabProgrammatically || _modelGraphicsTabs == nullptr || index < 0 ||
        simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }

    ModelGraphicsView* graphicsView = dynamic_cast<ModelGraphicsView*>(_modelGraphicsTabs->widget(index));
    if (graphicsView == nullptr) {
        return;
    }

    _syncCurrentModelDocumentState();

    GraphicalModelViewContext* context = _contextForView(graphicsView);
    if (context != nullptr && context->rootModel != nullptr) {
        if (simulator->getModelManager()->setCurrent(context->rootModel)) {
            _restoreModelDocumentState(context->rootModel);
        }
    }

    _activateModelGraphicsView(graphicsView);
    ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();
    _actualizeActions();
    _actualizeTabPanes();
}

void MainWindow::_activatePreviousModel() {
    if (simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }
    Model* model = simulator->getModelManager()->previous();
    if (model != nullptr) {
        _ensureModelTab(model);
        _actualizeActions();
        _actualizeTabPanes();
    }
}

void MainWindow::_activateNextModel() {
    if (simulator == nullptr || simulator->getModelManager() == nullptr) {
        return;
    }
    Model* model = simulator->getModelManager()->next();
    if (model != nullptr) {
        _ensureModelTab(model);
        _actualizeActions();
        _actualizeTabPanes();
    }
}

void MainWindow::_refreshRecentModelsMenu() {
    if (_menuOpenRecent == nullptr) {
        return;
    }

    _menuOpenRecent->clear();
    _recentModelActions.clear();

    const std::vector<std::string> recentModels = SystemPreferences::recentModelFiles();
    if (recentModels.empty()) {
        QAction* emptyAction = _menuOpenRecent->addAction(tr("(No recent files)"));
        emptyAction->setEnabled(false);
        _recentModelActions.append(emptyAction);
        return;
    }

    for (const std::string& fileName : recentModels) {
        const QString qFileName = QString::fromStdString(fileName);
        const QString displayText = QFileInfo(qFileName).fileName().isEmpty()
                                        ? qFileName
                                        : QFileInfo(qFileName).fileName();
        QAction* action = _menuOpenRecent->addAction(displayText);
        action->setToolTip(qFileName);
        connect(action, &QAction::triggered, this, [this, qFileName]() {
            _openRecentModelFile(qFileName);
        });
        _recentModelActions.append(action);
    }
}

bool MainWindow::_openRecentModelFile(const QString& fileName) {
    if (_modelLifecycleController == nullptr) {
        return false;
    }
    const bool opened = _modelLifecycleController->openModelFile(fileName);
    if (!opened) {
        QMessageBox::warning(this, "Open Recent", tr("Error while opening model:\n%1").arg(fileName));
    }
    _refreshRecentModelsMenu();
    return opened;
}

void MainWindow::_rebuildViewDependentControllers() {
    if (ui == nullptr || ui->graphicsView == nullptr || simulator == nullptr) {
        return;
    }

    _modelInspectorController = std::make_unique<ModelInspectorController>(simulator,
                                                                           ui->treeWidgetComponents,
                                                                           ui->treeWidgetDataDefnitions,
                                                                           ui->graphicsView);
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
    _graphicalModelBuilder = std::make_unique<GraphicalModelBuilder>(simulator,
                                                                      ui->graphicsView,
                                                                      ui->graphicsView->getScene(),
                                                                      _pluginCategoryColor,
                                                                      ui->textEdit_Console);
    if (GraphicalModelViewContext* context = _activeViewContext()) {
        // Keep rebuild services scoped to the same root/submodel level represented by the tab.
        _graphicalModelBuilder->setModelLevelFilter(context->modelLevel);
    } else {
        _graphicalModelBuilder->clearModelLevelFilter();
    }
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
                                                                            ui->actionShowEditableElements,
                                                                            ui->actionShowAttachedElements,
                                                                            ui->actionShowRecursiveElements,
                                                                            ui->textEdit_Console,
                                                                            &_modelfilename,
                                                                            [this]() { _clearModelEditors(); },
                                                                            [this]() { _generateGraphicalModelFromModel(); },
                                                                            [this]() { on_actionShowInternalElements_triggered(); },
                                                                            [this]() { on_actionShowEditableElements_triggered(); },
                                                                            [this]() { on_actionShowAttachedElements_triggered(); });
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
    _editCommandController = std::make_unique<EditCommandController>(
        simulator,
        ui->graphicsView,
        [this]() { _actualizeActions(); },
        &_cut,
        &_gmc_copies,
        &_ports_copies,
        &_draw_copy,
        &_group_copy);
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
    _graphicalContextMenuController = std::make_unique<GraphicalContextMenuController>(
        ui->graphicsView,
        ui,
        _actionOpenSelectedSubmodel,
        [this]() { return ui->graphicsView->getScene(); },
        [this]() { _actualizeActions(); });
    ui->graphicsView->setContextMenuEventHandler(_graphicalContextMenuController.get(),
                                                 &GraphicalContextMenuController::handleGraphicsViewContextMenu);
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
            _refreshGuiExtensions();
        },
        [this]() { return myScene(); },
        _optimizerPrecision,
        _optimizerMaxSteps,
        _parallelizationEnabled,
        _parallelizationThreads,
        _parallelizationBatchSize,
        _lastDataAnalyzerPath);

    _refreshGuiExtensions();
}

void MainWindow::_refreshGuiExtensions() {
    if (_guiExtensionManager == nullptr) {
        _guiExtensionManager = std::make_unique<GuiExtensionManager>(this);
    }
    if (ui == nullptr || ui->graphicsView == nullptr) {
        return;
    }

    GuiExtensionRuntimeContext extensionContext;
    extensionContext.simulator = simulator;
    extensionContext.mainWindow = this;
    extensionContext.ui = ui;
    extensionContext.graphicsView = ui->graphicsView;
    extensionContext.graphicsScene = ui->graphicsView->getScene();
    _guiExtensionManager->setLoadedModelPluginIds(collectLoadedModelPluginIds(simulator));
    _guiExtensionManager->setPlugins(GuiExtensionPluginCatalog::resolvedPlugins());
    _guiExtensionManager->rebuild(extensionContext);
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

void MainWindow::refreshGuiExtensions() {
	_refreshGuiExtensions();
}

void MainWindow::refreshPluginCatalog() {
	if (_pluginCatalogController != nullptr) {
		_pluginCatalogController->reloadFromPluginManager();
	}
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
    if (_actionModelPrevious != nullptr) {
        _actionModelPrevious->setEnabled(opened && simulator->getModelManager()->canGoPrevious() && !simulationInteractionLocked);
    }
    if (_actionModelNext != nullptr) {
        _actionModelNext->setEnabled(opened && simulator->getModelManager()->canGoNext() && !simulationInteractionLocked);
    }
    if (_actionOpenSelectedSubmodel != nullptr) {
        _actionOpenSelectedSubmodel->setEnabled(opened
                                                && !simulationInteractionLocked
                                                && _canOpenSubmodelFor(_selectedSubmodelOwner()));
    }
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
    if (m != nullptr) {
        _ensureModelTab(m);
        _modelfilename = _modelFilenames[m];
    }
    _actualizeUndo();
    ui->graphicsView->getScene()->showGrid(); //@TODO: Bad place to be
    ui->graphicsView->centerOn(TraitsGUI<GView>::sceneCenter, TraitsGUI<GView>::sceneCenter);
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
    _updateModelTabs();
    _actualizeActions();
    _actualizeTabPanes();
    _syncCurrentModelDocumentState();
}

void MainWindow::_actualizeUndo() {
    undoView = new QUndoView(ui->graphicsView->getScene()->getUndoStack());
    undoView->setWindowTitle(tr("Command List"));
    undoView->setVisible(false);
    undoView->setAttribute(Qt::WA_QuitOnClose, false);
}
