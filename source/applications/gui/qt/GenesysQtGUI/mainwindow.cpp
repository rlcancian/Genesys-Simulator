#include "mainwindow.h"
#include "ui_mainwindow.h"
// Dialogs
#include "dialogs/dialogBreakpoint.h"
#include "dialogs/Dialogmodelinformation.h"
#include "dialogs/dialogsimulationconfigure.h"
#include "dialogs/dialogpluginmanager.h"
#include "dialogs/dialogsystempreferences.h"
#include "dialogs/DialogFind.h"
// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../TraitsApp.h"
// GUI
#include "ModelGraphicsScene.h"
#include "TraitsGUI.h"
#include "graphicals/GraphicalConnection.h"
// PropEditor
#include "qtpropertybrowser/qttreepropertybrowser.h"
// QT
#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
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
#include <actions/PasteUndoCommand.h>
#include <actions/DeleteUndoCommand.h>
#include "animations/AnimationVariable.h"
#include <cstdlib>
#include <QRegularExpression>
// @TODO: Should NOT be hardcoded!!! (Used to visualize variables)
#include "../../../../plugins/data/Variable.h"


#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    //
    // Genesys Simulator
    simulator = new Simulator();
    simulator->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    simulator->getTraceManager()->addTraceHandler<MainWindow>(this, &MainWindow::_simulatorTraceHandler);
    simulator->getTraceManager()->addTraceErrorHandler<MainWindow>(this, &MainWindow::_simulatorTraceErrorHandler);
    simulator->getTraceManager()->addTraceReportHandler<MainWindow>(this, &MainWindow::_simulatorTraceReportsHandler);
    simulator->getTraceManager()->addTraceSimulationHandler<MainWindow>(this, &MainWindow::_simulatorTraceSimulationHandler);

    simulator->getPluginManager()->autoInsertPlugins(_autoLoadPluginsFilename.toStdString());
    // now complete the information
    for (unsigned int i = 0; i < simulator->getPluginManager()->size(); i++) {
        //@TODO: now it's the opportunity to adjust template
        _insertPluginUI(simulator->getPluginManager()->getAtRank(i));
    }

	propertyGenesys = new PropertyEditorGenesys();
    propertyList = new std::map<SimulationControl*, DataComponentProperty*>();
    propertyEditorUI = new std::map<SimulationControl*, DataComponentEditor*>();
    propertyBox = new std::map<SimulationControl*, ComboBoxEnum*>();

    //_insertFakePlugins(); // todo hate this

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
    //
    // property editor
    ui->treeViewPropertyEditor->setAlternatingRowColors(true);
    // finally
    _actualizeActions();
    // another try to start maximized (it should not be that hard)
    QRect screenGeometry = QApplication::primaryScreen()->availableGeometry();
    this->resize(screenGeometry.width(), screenGeometry.height());
    //
    // FOR TESTS ONLY
    //ui->treeViewPropertyEditor->set
    //this->on_actionModelNew_triggered();
    //this->_loadGraphicalModel("./models/Smart_AnElectronicAssemblyAndTestSystem.gen"); //("../../../../../models/Smart_Delay.gen"); // Smart_AnElectronicAssemblyAndTestSystem.gen");
    //ui->tabWidget_Model->setCurrentIndex(CONST.TabModelGraphicEditIndex);
}

MainWindow::~MainWindow() {
    delete ui;
}

ModelGraphicsScene* MainWindow::myScene() const {
    return ui->graphicsView->getScene();
}
//-----------------------------------------------------------------

bool MainWindow::_saveTextModel(QFile *saveFile, QString data)
{
    QTextStream out(saveFile);

    try
    {
        static const QRegularExpression regex("[\n]");
        QStringList strList = data.split(regex);
        for (const QString &line : strList)
        {
            out << line << Qt::endl;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

bool MainWindow::_saveGraphicalModel(QString filename)
{
    QFile saveFile(filename);

    try
    {
        if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::information(this, tr("Unable to access file to save"),
                                     saveFile.errorString());
            return false;
        }

        _saveTextModel(&saveFile, ui->TextCodeEditor->toPlainText());

        QTextStream out(&saveFile);
        out << "#Genegys Graphic Model" << Qt::endl;
        QString line = "0\tView\t";
        line += "zoom=" + QString::number(ui->horizontalSlider_ZoomGraphical->value());
        line += ", grid=" + QString::number(ui->actionShowGrid->isChecked()) + ", rule=0, snap="+ QString::number(ui->actionShowGrid->isChecked()) + ", viewpoint=(0,0)";
        out << line << Qt::endl;

        ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->getScene());

        if (scene)
        {
            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                GraphicalModelComponent *gmc = (GraphicalModelComponent *)item;
                if (gmc)
                {
                    line = QString::fromStdString(std::to_string(gmc->getComponent()->getId()) + "\t" + gmc->getComponent()->getClassname() + "\t" + gmc->getComponent()->getName() + "\t" + "color=" + gmc->getColor().name().toStdString() + "\t" + "position=(" + std::to_string(gmc->scenePos().x()) + "," + std::to_string(gmc->scenePos().y() + gmc->getHeight()/2) + ")");
                    out << line << Qt::endl;
                }
            }

            QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();

            if (counters) {
                if (!counters->empty()) {
                    out << Qt::endl;
                    out << "#Counters" << Qt::endl;
                    int id = 0;
                    for (AnimationCounter *counter : *counters) {
                        int idCounter = -1;
                        if (counter->getCounter() != nullptr) {
                            idCounter = counter->getCounter()->getId();
                        }
                        line = QString("Counter_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idCounter)
                                   .arg(counter->scenePos().x(), 0, 'f', 2)
                                   .arg(counter->scenePos().y(), 0, 'f', 2)
                                   .arg(counter->boundingRect().width(), 0, 'f', 2)
                                   .arg(counter->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();

            if (variables) {
                if (!variables->empty()) {
                    out << Qt::endl;
                    out << "#Variables" << Qt::endl;
                    int id = 0;
                    for (AnimationVariable *variable : *variables) {
                        int idVariable = -1;
                        if (variable->getVariable() != nullptr) {
                            idVariable = variable->getVariable()->getId();
                        }
                        line = QString("Variable_%1 \t id=%2 \t position=(%3,%4) \t width=%5 \t height=%6")
                                   .arg(id)
                                   .arg(idVariable)
                                   .arg(variable->scenePos().x(), 0, 'f', 2)
                                   .arg(variable->scenePos().y(), 0, 'f', 2)
                                   .arg(variable->boundingRect().width(), 0, 'f', 2)
                                   .arg(variable->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();

            if (timers) {
                if (!timers->empty()) {
                    out << Qt::endl;
                    out << "#Timers" << Qt::endl;
                    int id = 0;
                    for (AnimationTimer *timer : *timers) {
                        line = QString("Timer_%1 \t hour=%2 \t minute=%3 \t second=%4 \t format=%5 \t position=(%6,%7) \t width=%8 \t height=%9")
                                   .arg(id)
                                   .arg(timer->getInitialHours())
                                   .arg(timer->getInitialMinutes())
                                   .arg(timer->getInitialSeconds())
                                   .arg(static_cast<unsigned int>(timer->getTimeFormat()))
                                   .arg(timer->scenePos().x(), 0, 'f', 2)
                                   .arg(timer->scenePos().y(), 0, 'f', 2)
                                   .arg(timer->boundingRect().width(), 0, 'f', 2)
                                   .arg(timer->boundingRect().height(), 0, 'f', 2);

                        out << line << Qt::endl;
                        id++;
                    }
                }
            }

            /*/Lines
            out << "LINE" << Qt::endl;
            for (QGraphicsItem *item : *ui->graphicsView->getScene()->getGraphicalModelComponents())
            {
                QGraphicsLineItem *drawLine = dynamic_cast<QGraphicsLineItem *> (item);
                if (drawLine)
                {
                    line = QString::fromStdString(std::to_string(drawLine->))
                }
            }*/
        }

        saveFile.close();
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }

    //QString data = QString::fromStdString(dot);
    //QStringList strList = data.split(QRegExp("[\n]"), QString::SkipEmptyParts);
    //for (unsigned int i = 0; i < strList.size(); i++) {
    //	savefile << strList.at(i).toStdString() << std::endl;
    //}
}

Model *MainWindow::_loadGraphicalModel(std::string filename) {
    QFile file(QString::fromStdString(filename));

    Model *model = nullptr;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(this, tr("Unable to access file to save"),
                                file.errorString());
        return nullptr;
    }

    QFileInfo fileInfo(file.fileName());
    QString extension = fileInfo.suffix();

    if (extension != "gui") {
        model = simulator->getModelManager()->loadModel(file.fileName().toStdString());
        if (model != nullptr) _generateGraphicalModelFromModel();
        return model;
    }

    QString content = file.readAll();
    file.close();

    QStringList lines = content.split("\n");

    QStringList simulLang;
    QStringList gui;
    QStringList counters;
    QStringList variables;
    QStringList timers;

    bool guiFlag = false;
    bool counterFlag = false;
    bool variableFlag = false;
    bool timerFlag = false;

    for (const QString &line : lines) {
        if (line.startsWith("#Genegys Graphic Model")) {
            guiFlag = true;

            counterFlag = false;
            variableFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Counters")) {
            counterFlag = true;

            guiFlag = false;
            variableFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Variables")) {
            variableFlag = true;

            guiFlag = false;
            counterFlag = false;
            timerFlag = false;
            continue;
        }

        if (line.startsWith("#Timers")) {
            timerFlag = true;

            guiFlag = false;
            counterFlag = false;
            variableFlag = false;
            continue;
        }

        if (!guiFlag && !timerFlag && !counterFlag && !variableFlag) {
            simulLang.append(line);
        } else {
            if (counterFlag) {
                counters.append(line);
            } else if (variableFlag) {
                variables.append(line);
            } else if (timerFlag) {
                timers.append(line);
            } else {
                gui.append(line);
            }
        }
    }

    file.close();

    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString newFilename = QString("/tempFile-%1.gen").arg(currentDateTime.toString("yyyy-MM-dd-hh-mm-ss"));

    QString filePath = QDir::tempPath() + newFilename;
    QFile tempFile(filePath);
    tempFile.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream outStream(&tempFile);
    for (const QString& line : simulLang) {
        outStream << line << Qt::endl;
    }

    outStream.flush();

    std::string nameTempFile = tempFile.fileName().toStdString();
    model = simulator->getModelManager()->loadModel(nameTempFile);

    tempFile.close();

    QFile::remove(tempFile.fileName());

    if (model != nullptr) {
        std::list<ModelComponent*> c = * model->getComponents()->getAllComponents();

        _clearModelEditors();

        bool firstLine = true;

        for (const QString& line : gui) {
            if (line.trimmed().isEmpty()) {
                continue;
            }

            if (firstLine) {
                QRegularExpression regex("(\\d+)\\s*View\\s*zoom=(\\d+),\\s*grid=(\\d+),\\s*rule=(\\d+),\\s*snap=(\\d+),\\s*viewpoint=\\(([^,]+),([^\\)]+)\\)");
                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int index = match.captured(1).toInt();
                    int zoom = match.captured(2).toInt();
                    int grid = match.captured(3).toInt();
                    int rule = match.captured(4).toInt();
                    int snap = match.captured(5).toInt();
                    qreal viewpointX = match.captured(6).toDouble();
                    qreal viewpointY = match.captured(7).toDouble();

                    if (grid) {
                        ui->actionShowGrid->setChecked(true);
                        myScene()->showGrid();
                    }

                    if (snap) {
                        ui->actionShowSnap->setChecked(true);
                        myScene()->setSnapToGrid(true);
                    }

                    ui->horizontalSlider_ZoomGraphical->setValue(zoom + TraitsGUI<GMainWindow>::zoomButtonChange);
                    double factor = ((double) zoom / 100.0)*(2 - 0.5) + 0.5;
                    double scaleFactor = 1.0;
                    scaleFactor *= factor;
                    //ui->label_ModelGraphic->resize(scaleFactor * ui->label_ModelGraphic->pixmap()->size());
                }
                firstLine = false;
                continue;
            }

            QStringList split = line.split("\t");

            Util::identification id = split[0].toULong();

            // Component
            QString comp = split[1];

            // Color
            QString col = split[3];

            // Posição
            QString pos = split[4];

            // Expressao regular para pegar a cor
            QRegularExpression regexColor("color=#([0-9A-Fa-f]{6})");

            // Cria a expressao regular match
            QRegularExpressionMatch match = regexColor.match(col);

            QString hexColor;

            // Extrai a cor
            if (match.hasMatch()) {hexColor = match.captured(1);}

            // Cria a cor
            QColor color("#"+ hexColor);

            // Expressao regular para pegar a cor
            QRegularExpression regexPos("position=\\((-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*),(-?\\d+\\.?\\d*)\\)");

            // Cria a expressao regular match
            match = regexPos.match(pos);

            QPoint position;

            // Extrai a posição
            if (match.hasMatch()) {
                // Extrai x e y
                qreal x = match.captured(1).toDouble();
                qreal y = match.captured(3).toDouble();

                // Seta x e y em pos
                position.setX(x);
                position.setY(y);
            }

            // Pega a cena para adicionar o componente nela
            ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->scene());

            // Pega o Plugin
            Plugin* plugin = simulator->getPluginManager()->find(comp.toStdString());

            // Cria o componente no modelo
            ModelComponent* component = simulator->getModelManager()->current()->getComponents()->find(id);

            if (!component) continue;
            // Desenha na tela
            scene->addGraphicalModelComponent(plugin, component, position, color);
        }

        QList<QGraphicsItem*> *graphicalComponents = ui->graphicsView->getScene()->getGraphicalModelComponents();

        for (unsigned int i = 0; i < (unsigned int) graphicalComponents->size(); i++) {
            GraphicalModelComponent *source = dynamic_cast<GraphicalModelComponent *> (graphicalComponents->at(i));
            std::map<unsigned int, Connection*> *connections = source->getComponent()->getConnections()->connections();

            for (auto it = connections->begin(); it != connections->end(); ++it) {
                unsigned int portSource = it->first;
                Connection* connection = it->second;

                GraphicalModelComponent* destination = ui->graphicsView->getScene()->findGraphicalModelComponent(connection->component->getId());
                unsigned int portDestination = destination->getGraphicalInputPorts().at(0)->portNum();

                source->setOcupiedOutputPorts(source->getOcupiedOutputPorts() + 1);
                destination->setOcupiedInputPorts(destination->getOcupiedInputPorts() + 1);

                std::string nameSource = source->getComponent()->getName();
                GraphicalComponentPort* sourceport = source->getGraphicalOutputPorts().at(portSource);

                std::string nameDestination = destination->getComponent()->getName();
                GraphicalComponentPort* destport = destination->getGraphicalInputPorts().at(portDestination);

                ui->graphicsView->getScene()->addGraphicalConnection(sourceport, destport, portSource, portDestination);
            }
        }


        if (!counters.empty()) {
            QRegularExpression regex("Counter_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : counters) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationCounter *counter = new AnimationCounter();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    counter->setIdCounter(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    counter->setRect(newRect.normalized());
                    counter->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsCounter()->append(counter);

                    myScene()->addItem(counter);
                } else {
                    delete counter;
                }
            }
        }

        if (!variables.empty()) {
            QRegularExpression regex("Variable_(\\d+) \\t id=(-?\\d+) \\t position=\\(([^,]+),([^\\)]+)\\) \\t width=([^\\t]+) \\t height=([^\\t]+)");

            for (const QString& line : variables) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationVariable *variable = new AnimationVariable();

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int id = match.captured(2).toInt();
                    qreal posX = match.captured(3).toDouble();
                    qreal posY = match.captured(4).toDouble();
                    int width = match.captured(5).toDouble();
                    int height = match.captured(6).toDouble();

                    variable->setIdVariable(id);

                    QRectF newRect = QRectF(0, 0, width, height);
                    variable->setRect(newRect.normalized());
                    variable->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsVariable()->append(variable);

                    myScene()->addItem(variable);
                } else {
                    delete variable;
                }
            }
        }

        if (!timers.empty()) {
            QRegularExpression regex("Timer_(\\d+) \\s* hour=(\\d+) \\s* minute=(\\d+) \\s* second=(\\d+) \\s* format=(\\d+) \\s* position=\\(([^,]+),([^\\)]+)\\) \\s* width=([^\\t]+) \\s* height=([^\\t]+)");

            for (const QString& line : timers) {
                if (line.trimmed().isEmpty()) {
                    continue;
                }

                AnimationTimer *timer = new AnimationTimer(myScene());

                QRegularExpressionMatch match = regex.match(line);

                if (match.hasMatch()) {
                    int hour = match.captured(2).toInt();
                    int minute = match.captured(3).toInt();
                    int second = match.captured(4).toInt();
                    int format = match.captured(5).toInt();
                    qreal posX = match.captured(6).toDouble();
                    qreal posY = match.captured(7).toDouble();
                    int width = match.captured(8).toDouble();
                    int height = match.captured(9).toDouble();

                    timer->setInitialHours(hour);
                    timer->setInitialMinutes(minute);
                    timer->setInitialSeconds(second);
                    timer->setTimeFormat(Util::TimeFormat(format));

                    timer->setTime(0.0);

                    QRectF newRect = QRectF(0, 0, width, height);
                    timer->setRect(newRect.normalized());
                    timer->setPos(QPointF(posX, posY));

                    myScene()->getAnimationsTimer()->append(timer);

                    myScene()->addItem(timer);

                } else {
                    delete timer;
                }
            }
        }

        ui->textEdit_Console->append("\n");
        _modelfilename = QString::fromStdString(filename);
    }
    return model;
}

//-----------------------------------------------------------------


//-----------------
// View
//-----------------

void MainWindow::_recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInLine) {
    PluginManager* pm = simulator->getPluginManager();
    GraphicalModelComponent *gmc;
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    Plugin* plugin = pm->find(component->getClassname());
    assert(plugin!=nullptr);
    // get color from category
    QColor color = _pluginCategoryColor->at(plugin->getPluginInfo()->getCategory());
    gmc = scene->addGraphicalModelComponent(plugin, component, QPoint(*x, *y), color);
    map->insert({component,gmc});
    visited->insert(component);
    int yIni = *y;
    int xIni = *x;
    const int deltaY = TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 1.5;
    GraphicalComponentPort *sourceGraphicalPort, *destinyGraphicalPort;
    for(auto connectionMap: *component->getConnections()->connections()) {
        ModelComponent* nextComp = connectionMap.second->component;
        if (visited->find(nextComp)==visited->list()->end()) { // nextComponent was not visited yet
            if (++sequenceInLine==6) {
                *x -= 5 * TraitsGUI<GModelComponent>::width * 1.5;
                *y+= deltaY;
                sequenceInLine = 0;
            } else {
                *x += TraitsGUI<GModelComponent>::width * 1.5;
            }
            if (*y > *ymax)
                *ymax = *y;
            _recursivalyGenerateGraphicalModelFromModel(nextComp, visited, map, x, y, ymax, sequenceInLine);
            GraphicalModelComponent *destinyGmc = map->at(nextComp);
            sourceGraphicalPort = gmc->getGraphicalOutputPorts().at(connectionMap.first);
            destinyGraphicalPort = destinyGmc->getGraphicalInputPorts().at(connectionMap.second->channel.portNumber);
            scene->addGraphicalConnection(sourceGraphicalPort, destinyGraphicalPort, connectionMap.first, connectionMap.second->channel.portNumber);
            *x = xIni;
            *y+= deltaY;
            sequenceInLine--;
        }
    }
    *y = yIni;
}

void MainWindow::_generateGraphicalModelFromModel() {
    Model* m=simulator->getModelManager()->current();
    if (m!=nullptr) {
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(false);
        //ui->graphicsView->getScene()->showGrid();
        int x, y, ymax;
        x=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().left();
        y=TraitsGUI<GView>::sceneCenter - TraitsGUI<GView>::sceneDistanceCenter*0.8; //ui->graphicsView->sceneRect().top();
        ymax=y;
        ComponentManager* cm = m->getComponents();
        List<ModelComponent*>* visited = new List<ModelComponent*>();
        std::map<ModelComponent*,GraphicalModelComponent*>* map = new std::map<ModelComponent*,GraphicalModelComponent*>();
        for(SourceModelComponent* source: *cm->getSourceComponents()) {
            _recursivalyGenerateGraphicalModelFromModel(source, visited, map, &x, &y, &ymax, 0);
            y= ymax + TraitsGUI<GModelComponent>::width * TraitsGUI<GModelComponent>::heightProportion * 3; // get heigth mapped to scene??
        }
        // check if any component remains unvisited
        bool foundNotVisited;
        do {
            foundNotVisited = false;
            for (ModelComponent* comp: *cm->getAllComponents()) {
                if (visited->find(comp) == visited->list()->end()) { // found a compponent not visited yet
                    foundNotVisited = true;
                    visited->insert(comp);
                    // recursive create
                }
            }
        } while (foundNotVisited);
        delete map;
        delete visited;
        ui->graphicsView->setCanNotifyGraphicalModelEventHandlers(true);
    }
}

void MainWindow::_actualizeActions() {
    bool opened = simulator->getModelManager()->current() != nullptr;
    bool running = false;
    bool paused = false;
    unsigned int numSelectedGraphicals = 0;
    unsigned int actualCommandundoRedo = 0; //@TODO
    unsigned int maxCommandundoRedo = 0; //@TODO
    if (opened) {
        running = simulator->getModelManager()->current()->getSimulation()->isRunning();
        paused = simulator->getModelManager()->current()->getSimulation()->isPaused();
        numSelectedGraphicals = 0;//@TODO get total of selected graphical objects (this should br on another "actualize", I think
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
    // TODO: MUDAR, ESTÁ HARDCODED, DEVERIA SER DISPONIBILIZADO COM UM COMPONENENTE FOSSE
    // TODO: SELECIONADO
    ui->actionEditCopy->setEnabled(0 && !running);
    ui->actionEditCut->setEnabled(0 && !running);
    ui->actionEditDelete->setEnabled((numSelectedGraphicals>0) && !running);

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

void MainWindow::_actualizeModelTextHasChanged(bool hasChanged) {
    if (_textModelHasChanged != hasChanged) {
    }
    _textModelHasChanged = hasChanged;
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
            ui->tableWidget_Breakpoints->setRowCount(++row);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem("Entity");
            ui->tableWidget_Breakpoints->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(entity->getName()));
            ui->tableWidget_Breakpoints->setItem(row, 2, newItem);
        }
        for (double time : *sim->getBreakpointsOnTime()->list()) {
            ui->tableWidget_Breakpoints->setRowCount(++row);
            QTableWidgetItem* newItem;
            newItem = new QTableWidgetItem("Time");
            ui->tableWidget_Breakpoints->setItem(row, 1, newItem);
            newItem = new QTableWidgetItem(QString::fromStdString(std::to_string(time)));
            ui->tableWidget_Breakpoints->setItem(row, 2, newItem);
        }
    }
}

void MainWindow::_actualizeModelComponents(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetComponents->clear();
    if (m == nullptr) {
        return;
    }
    for (ModelComponent* comp : *m->getComponents()->getAllComponents()) {
        QList<QTreeWidgetItem *> items = ui->treeWidgetComponents->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
        if (items.size() == 0) {
            QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetComponents);
            treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
            treeComp->setText(1, QString::fromStdString(comp->getClassname()));
            treeComp->setText(2, QString::fromStdString(comp->getName()));
            std::string properties = "";
            for (auto prop : *comp->getProperties()->list()) {
                properties += prop->getName() + ":" + prop->getValue() + ", ";
            }
            properties = properties.substr(0, properties.length() - 2);
            treeComp->setText(3, QString::fromStdString(properties));
        }
    }
    ui->treeWidgetComponents->resizeColumnToContents(0);
    ui->treeWidgetComponents->resizeColumnToContents(1);
    ui->treeWidgetComponents->resizeColumnToContents(2);
}

void MainWindow::_actualizeModelDataDefinitions(bool force) {
    Model* m = simulator->getModelManager()->current();
    ui->treeWidgetDataDefnitions->clear();
    if (m == nullptr) {
        return;
    }
    for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
        for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {
            QList<QTreeWidgetItem *> items = ui->treeWidgetDataDefnitions->findItems(QString::fromStdString(std::to_string(comp->getId())), Qt::MatchExactly | Qt::MatchRecursive, 0);
            if (items.size() == 0) {
                QTreeWidgetItem* treeComp = new QTreeWidgetItem(ui->treeWidgetDataDefnitions);
                treeComp->setText(0, QString::fromStdString(std::to_string(comp->getId())));
                treeComp->setText(1, QString::fromStdString(comp->getClassname()));
                treeComp->setText(2, QString::fromStdString(comp->getName()));
                std::string properties = "";
                for (auto prop : *comp->getProperties()->list()) {
                    properties += prop->getName() + ":" + prop->getValue() + ", ";
                }
                properties = properties.substr(0, properties.length() - 2);
                treeComp->setText(3, QString::fromStdString(properties));
            }
        }
    }
    ui->treeWidgetDataDefnitions->resizeColumnToContents(0);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(1);
    ui->treeWidgetDataDefnitions->resizeColumnToContents(2);
}

void MainWindow::_actualizeGraphicalModel(SimulationEvent * re) {
    Event* event = re->getCurrentEvent();
    if (event != nullptr) {
        ui->graphicsView->selectModelComponent(event->getComponent());
    }
}

void MainWindow::_onMoveEntityEvent(SimulationEvent *re) {
    // Cria as animações de contadores, variáveis e tempo
    myScene()->animateCounter();
    myScene()->animateVariable();

    // Cria a animação de transição
    if (re) {
        if (re->getCurrentEvent()) {
            if (re->getCurrentEvent()->getComponent()) {
                ModelComponent *source = re->getCurrentEvent()->getComponent();
                ModelComponent *destination = re->getDestinationComponent();

                // Remove animação de fila se for o caso
                myScene()->animateQueueRemove(source);

                myScene()->animateTransition(source, destination, ui->actionActivateGraphicalSimulation->isChecked(), re->getCurrentEvent());
            }
        }
    }
}

void MainWindow::_onAfterProcessEvent(SimulationEvent *re) {
    // Cria as animações de contadores, variáveis e tempo (atualiza assim que termina)
    myScene()->animateCounter();
    myScene()->animateVariable();
    myScene()->animateTimer(simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
}

QColor MainWindow::myrgba(uint64_t color) {
    uint8_t r, g, b, a;
    r = (color&0xFF000000)>>24;
    g = (color&0x00FF0000)>>16;
    b = (color&0x0000FF00)>>8;
    a = (color&0x000000FF);
    return QColor(r, g, b, a);
}

std::string MainWindow::dotColor(uint64_t color) {
    std::stringstream stream;
    stream << std::hex << "#" << color;
    return stream.str();
}

void MainWindow::_insertCommandInConsole(std::string text) {
    ui->textEdit_Console->setTextColor(myrgba(TraitsGUI<GMainWindow>::consoleTextColor));
    QFont font(ui->textEdit_Console->font());
    font.setBold(true);
    ui->textEdit_Console->setFont(font);
    ui->textEdit_Console->append("\n$genesys> " + QString::fromStdString(text));
    ui->textEdit_Console->moveCursor(QTextCursor::MoveOperation::Down, QTextCursor::MoveMode::MoveAnchor);
    font.setBold(false);
    ui->textEdit_Console->setFont(font);
}

void MainWindow::_actualizeModelSimLanguage() {
    Model* m = simulator->getModelManager()->current();
    if (m != nullptr) {
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, true);
        std::string tempFilename = "./temp.tmp";
        m->getPersistence()->setOption(ModelPersistence_if::Options::SAVEDEFAULTS, false);
        bool res = m->save(tempFilename);
        std::string line;
        std::ifstream file(tempFilename);
        if (file.is_open()) {
            ui->TextCodeEditor->clear();
            while (std::getline(file, line)) {
                ui->TextCodeEditor->appendPlainText(QString::fromStdString(line));
            }
            file.close();
            _textModelHasChanged = false;
        }
    }
}

void MainWindow::_clearModelEditors() {
    ui->TextCodeEditor->clear();
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();
    ui->graphicsView->clear();
    ui->plainTextEditCppCode->clear();
    ui->treeWidgetComponents->clear();
    ui->treeWidgetDataDefnitions->clear();
}

bool MainWindow::_setSimulationModelBasedOnText() {
    Model* model = simulator->getModelManager()->current();
    if (this->_textModelHasChanged) {
        //@TODO !!!!!!!!!!!!!!
        // simulator->getModels()->remove(model);
        // model = nullptr;
    }
    if (model == nullptr) { // only model text written in UI
        QString modelLanguage = ui->TextCodeEditor->toPlainText();
        if (!simulator->getModelManager()->createFromLanguage(modelLanguage.toStdString())) {
            QMessageBox::critical(this, "Check Model", "Error in the model text. See console for more information.");
        }
        model = simulator->getModelManager()->current();
        if (model != nullptr) {

            _setOnEventHandlers();
        }
    }
    return simulator->getModelManager()->current() != nullptr;
}

std::string MainWindow::_adjustDotName(std::string name) {
    std::string text = Util::StrReplace(name, "[", "_");
    text = Util::StrReplace(text, "]", "");
    return Util::StrReplace(text, ".", "_");
}

void MainWindow::_insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode) {
    if (dotmap->find(compLevel) == dotmap->end()) {
        dotmap->insert({compLevel, new std::map<unsigned int, std::list<std::string>*>()});
    }
    std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotPair = (*dotmap->find(compLevel));
    if (dotPair.second->find(compRank) == dotPair.second->end()) {
        dotPair.second->insert({compRank, new std::list<std::string>()});
    }
    std::pair<unsigned int, std::list<std::string>*> dotPair2 = *(dotPair.second->find(compRank));
    if (isNode) {
        dotPair2.second->insert(dotPair2.second->begin(), text);
    } else {

        dotPair2.second->insert(dotPair2.second->end(), text);
    }
}

void MainWindow::_recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) {

    /*
    const struct DOT_STYLES {
        std::string nodeComponent = std::string("shape=record")+
                                    std::string(", fontsize=")+std::to_string(TraitsGUI<GModelGraphicPic>::nodeComponentFontSize)+
                                    std::string(", fontcolor=")+dotColor(TraitsGUI<GModelGraphicPic>::nodeComponentFontColor)+
                                    std::string(", style=filled")+
                                    std::string(", fillcolor=")+dotColor(TraitsGUI<GModelGraphicPic>::nodeComponentFillColor); //fillcolor=goldenrod3
        std::string edgeComponent = std::string("style=solid")+
                                    std::string(", arrowhead=\"normal\" color=")+dotColor(TraitsGUI<GModelGraphicPic>::edgeComponentEdgeColor)+
                                    std::string(", fontcolor=")+dotColor(TraitsGUI<GModelGraphicPic>::edgeComponentFontColor)+
                                    std::string(", fontsize=")+dotColor(TraitsGUI<GModelGraphicPic>::edgeComponentFontSize);
        std::string nodeDataDefInternal = std::string("shape=record")+
                                          std::string(", fontsize=")+std::to_string(TraitsGUI<GModelGraphicPic>::nodeDatadefInternalFontSize)+
                                          std::string(", color=")+dotColor(TraitsGUI<GModelGraphicPic>::nodeDatadefInternalColor)+
                                          std::string(", fontcolor=")+dotColor(TraitsGUI<GModelGraphicPic>::nodeDatadefInternalFontColor);
        std::string nodeDataDefAttached = std::string("shape=record")+
                                          std::string(", fontsize=10")+ //@TODO Continue replacing fized styles by TraitsGUI...
                                          std::string(", color=gray50")+
                                          std::string(", fontcolor=gray50")+
                                          std::string(", style=filled")+
                                          std::string(", fillcolor=darkolivegreen3");
        std::string edgeDataDefInternal = std::string("style=dashed")+
                                          std::string(", arrowhead=\"diamond\"")+
                                          std::string(", color=gray55")+
                                          std::string(", fontcolor=gray55")+
                                          std::string(", fontsize=7");
        std::string edgeDataDefAttached = std::string("style=dashed")+
                                          std::string(", arrowhead=\"ediamond\"")+
                                          std::string(", color=gray50")+
                                          std::string(", fontcolor=gray50")+
                                          std::string(", fontsize=7");
        unsigned int rankSource = 0;
        unsigned int rankSink = 1;
        unsigned int rankComponent = 99;
        unsigned int rankComponentOtherLevel = 99;
        unsigned int rankDataDefInternal = 99;
        unsigned int rankDataDefAttached = 99;
        unsigned int rankDataDefRecursive = 99;
        unsigned int rankEdge = 99;
    } DOT;
    */
    const struct DOT_STYLES {
        std::string nodeComponent = "shape=record, fontsize=12, fontcolor=black, style=filled, fillcolor=bisque";
        //std::string nodeComponentOtherLevel = "shape=record, fontsize=12, fontcolor=black, style=filled, fillcolor=goldenrod3";
        std::string edgeComponent = "style=solid, arrowhead=\"normal\" color=black, fontcolor=black, fontsize=7";
        std::string nodeDataDefInternal = "shape=record, fontsize=8, color=gray50, fontcolor=gray50, style=filled, fillcolor=#d9ebbd";
		std::string nodeDataDefAttached = "shape=record, fontsize=10, color=gray50, fontcolor=gray50, style=filled, fillcolor=#a2cd5a";
        std::string edgeDataDefInternal = "style=dashed, arrowhead=\"diamond\", color=gray55, fontcolor=gray55, fontsize=7";
        std::string edgeDataDefAttached = "style=dashed, arrowhead=\"ediamond\", color=gray50, fontcolor=gray50, fontsize=7";
        unsigned int rankSource = 0;
        unsigned int rankSink = 1;
        unsigned int rankComponent = 99;
        unsigned int rankComponentOtherLevel = 99;
        unsigned int rankDataDefInternal = 99;
        unsigned int rankDataDefAttached = 99;
        unsigned int rankDataDefRecursive = 99;
        unsigned int rankEdge = 99;
    } DOT;


    visited->insert(visited->end(), componentOrData);
    std::string text;
    unsigned int modellevel = simulator->getModelManager()->current()->getLevel();
    std::list<ModelDataDefinition*>::iterator visitedIt;
    ModelComponent* parentComponentSuperLevel = nullptr;
    unsigned int level = componentOrData->getLevel();
    if (dynamic_cast<ModelComponent*> (componentOrData) != nullptr) {
        if (level != modellevel && !ui->checkBox_ShowLevels->isChecked()) {
            // do not show the component itself, but its parent on the model level
            parentComponentSuperLevel = simulator->getModelManager()->current()->getComponents()->find(level);
            assert(parentComponentSuperLevel != nullptr);
            visitedIt = std::find(visited->begin(), visited->end(), parentComponentSuperLevel);
            if (visitedIt == visited->end()) {
                text = "  " + _adjustDotName(parentComponentSuperLevel->getName()) + " [" + DOT.nodeComponent + ", label=\"" + parentComponentSuperLevel->getClassname() + "|" + parentComponentSuperLevel->getName() + "\"]" + ";\n";
                _insertTextInDot(text, level, DOT.rankComponentOtherLevel, dotmap, true);
            }
        } else {
            text = "  " + _adjustDotName(componentOrData->getName()) + " [" + DOT.nodeComponent + ", label=\"" + componentOrData->getClassname() + "|" + componentOrData->getName() + "\"]" + ";\n";
            if (dynamic_cast<SourceModelComponent*> (componentOrData) != nullptr) {
                _insertTextInDot(text, level, DOT.rankSource, dotmap, true);
            } else if (dynamic_cast<SinkModelComponent*> (componentOrData) != nullptr) {
                _insertTextInDot(text, level, DOT.rankSink, dotmap, true);
            } else {
                _insertTextInDot(text, level, DOT.rankComponent, dotmap, true);
            }
        }
    }
    //
    ModelDataDefinition* data;
    std::string dataname, componentName;
    if (parentComponentSuperLevel != nullptr) {
        componentName = parentComponentSuperLevel->getName();
    } else {
        componentName = componentOrData->getName();
    }
    if (ui->checkBox_ShowInternals->isChecked()) {
        for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getInternalData()) {
            dataname = _adjustDotName(dataPair.second->getName());
            level = dataPair.second->getLevel();
            visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
            if (visitedIt == visited->end()) {
                if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                    text = "  " + dataname + " [" + DOT.nodeDataDefInternal + ", label=\"" + dataPair.second->getClassname() + "|" + dataPair.second->getName() + "\"]" + ";\n";
                    _insertTextInDot(text, level, DOT.rankDataDefInternal, dotmap, true);
                    if (ui->checkBox_ShowRecursive->isChecked()) {
                        _recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
                    }
                }
            }
            if (dataPair.second->getLevel() == modellevel || ui->checkBox_ShowLevels->isChecked()) {
                text = "    " + dataname + "->" + _adjustDotName(componentName) + " [" + DOT.edgeDataDefInternal + ", label=\"" + dataPair.first + "\"];\n";
                _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
            }
        }
    }
    if (ui->checkBox_ShowElements->isChecked()) {
        for (std::pair<std::string, ModelDataDefinition*> dataPair : *componentOrData->getAttachedData()) {
            dataname = _adjustDotName(dataPair.second->getName());
            level = dataPair.second->getLevel();
            visitedIt = std::find(visited->begin(), visited->end(), dataPair.second);
            if (visitedIt == visited->end()) {
                if (dynamic_cast<ModelComponent*> (dataPair.second) == nullptr) {
                    text = "  " + dataname + " [" + DOT.nodeDataDefAttached + ", label=\"" + dataPair.second->getClassname() + "|" + dataPair.second->getName() + "\"]" + ";\n";
                    _insertTextInDot(text, level, DOT.rankDataDefAttached, dotmap, true);
                }
                if (ui->checkBox_ShowRecursive->isChecked()) {
                    _recursiveCreateModelGraphicPicture(dataPair.second, visited, dotmap);
                }
            }
            text = "    " + dataname + "->" + _adjustDotName(componentName) + " [" + DOT.edgeDataDefAttached + ", label=\"" + dataPair.first + "\"];\n";
            _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
        }
    }
    ModelComponent* component = dynamic_cast<ModelComponent*> (componentOrData);
    if (component != nullptr) {
        level = component->getLevel();
        Connection* connection;
        for (unsigned short i = 0; i < component->getConnections()->size(); i++) {
            connection = component->getConnections()->getConnectionAtPort(i);
            visitedIt = std::find(visited->begin(), visited->end(), connection->component);
            if (visitedIt == visited->end()) {
                _recursiveCreateModelGraphicPicture(connection->component, visited, dotmap);
            }
            if (connection->component->getLevel() == modellevel || ui->checkBox_ShowLevels->isChecked()) {

                text = "    " + _adjustDotName(componentName) + "->" + _adjustDotName(connection->component->getName()) + "[" + DOT.edgeComponent + "];\n";
                _insertTextInDot(text, modellevel, DOT.rankEdge, dotmap);
            }
        }
    }
}

std::string MainWindow::_addCppCodeLine(std::string line, unsigned int indent) {
    std::string text = "";
    for (unsigned int i = 0; i < indent; i++) {
        text += "\t";
    }
    text += line + "\n";
    return text;
}

void MainWindow::_actualizeModelCppCode() {
    Model* m = simulator->getModelManager()->current();
    if (m != nullptr) {
        unsigned short tabs = 0;
        std::string text, text2, name;
        std::map<std::string, std::string>* code = new std::map<std::string, std::string>();
        text = _addCppCodeLine("/*");
        text += _addCppCodeLine(" * This C++ source code was automatically generated by GenESyS");
        text += _addCppCodeLine(" */");
        code->insert({"1begin", text});

        text = _addCppCodeLine("#include \"kernel/simulator/Simulator.h\"");
        text = _addCppCodeLine("#include \"kernel/simulator/PropertyGenesys.h\"");
        List<std::string>* included = new List<std::string>();
        for (ModelComponent* comp : *m->getComponents()->getAllComponents()) {
            name = comp->getClassname();
            if (included->find(name) == included->list()->end()) {
                included->insert(name);
                text += _addCppCodeLine("#include \"plugins/components/" + name + ".h\"");
            }
        }
        for (std::string ddClassname : *m->getDataManager()->getDataDefinitionClassnames()) {
            text += _addCppCodeLine("#include \"plugins/data/" + ddClassname + ".h\"");
            for (ModelDataDefinition* modeldata : *m->getDataManager()->getDataDefinitionList(ddClassname)->list()) {
                name = modeldata->getName();
                if (name.find(".") == std::string::npos) {
                    if (included->find(name) == included->list()->end()) {
                        included->insert(ddClassname);
                        text += _addCppCodeLine("#include \"plugins/data/" + ddClassname + "\"");
                    }
                }
            }
        }
        code->insert({"2include", text});

        text = _addCppCodeLine("\nint main(int argc, char** argv) {");
        tabs++;
        text += _addCppCodeLine("// Create simulator, a property editor, a model and get acess to plugins", tabs);
        text += _addCppCodeLine("Simulator* genesys = new Simulator();", tabs);
		text += _addCppCodeLine("PropertyEditorGenesys* propertyEditor = new PropertyEditorGenesys();", tabs);
        text += _addCppCodeLine("Model* model = genesys->getModels()->newModel();", tabs);
        text += _addCppCodeLine("PluginManager* plugins = genesys->getPlugins();", tabs);
        text += _addCppCodeLine("model->getTracer()->setTraceLevel(TraceManager::TraceLevel::L9_mostDetailed);", tabs);
        code->insert({"3main", text});

        text = _addCppCodeLine("// Create model components and setting properties", tabs);
        for (std::string ddClassname : *m->getDataManager()->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* modeldata : *m->getDataManager()->getDataDefinitionList(ddClassname)->list()) {
                name = modeldata->getName();
                if (name.find(".") == std::string::npos) {
                    text += _addCppCodeLine(ddClassname + "* " + name + " = plugins->newInstance<" + ddClassname + ">(model, \"" + name + "\");", tabs);
                }
				for (auto prop : *modeldata->getProperties()->list()) {
					// Fazer um loop até encontrar propriedade não alterável?
					text += _addCppCodeLine("SimulationControl* property = propertyEditor->findProperty(" + std::to_string(modeldata->getId()) + ", " + prop->getName() + ");", tabs);
					text += _addCppCodeLine("propertyEditor->changeProperty(property, " + prop->getValue() + ", false);", tabs);
				};
				text += _addCppCodeLine("", tabs);
            }
        }
        code->insert({"4datadef", text});

        text = _addCppCodeLine("// Create model components", tabs);
        for (ModelComponent* comp : *m->getComponents()->getAllComponents()) {
            name = comp->getName();
            if (name.find(".") == std::string::npos) {
                text += _addCppCodeLine(comp->getClassname() + "* " + name + " = plugins->newInstance<" + comp->getClassname() + ">(model, \"" + name + "\");", tabs);
            }
        }
        code->insert({"5modelcompdef", text});

        text = _addCppCodeLine("// Connect the components in the model", tabs);
        Connection* conn;
        for (ModelComponent* comp : *m->getComponents()->getAllComponents()) {
            name = comp->getName();
            if (name.find(".") == std::string::npos) {
                for (std::pair<unsigned int, Connection*> pair : *comp->getConnections()->connections()) {//unsigned int i=0; i<comp->getConnections()->size(); i++) {
                    conn = pair.second; //comp->getConnections()->getConnectionAtPort(i);
                    text2 = conn->component->getName(); // + conn->second==0?"":","+std::to_string(conn->second);
                    text += _addCppCodeLine(name + "->getConnections()->insertAtPort(" + std::to_string(pair.first) + "," + text2 + ");", tabs);
                }
            }
        }
        code->insert({"6modelconnect", text});

        ModelSimulation* sim = m->getSimulation();
        text = _addCppCodeLine("// Define simulation options", tabs);
        text += _addCppCodeLine("ModelSimulation* sim = model->getSimulation();", tabs);
        text += _addCppCodeLine("sim->setReplicationLength(" + std::to_string(sim->getReplicationLength()) + ")", tabs);
        text += _addCppCodeLine("sim->setReplicationLengthTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getReplicationLengthTimeUnit()) + ");", tabs);
        text += _addCppCodeLine("sim->setWarmUpPeriod(" + std::to_string(sim->getWarmUpPeriod()) + ");", tabs);
        text += _addCppCodeLine("sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getWarmUpPeriodTimeUnit()) + ");", tabs);
        text += _addCppCodeLine("sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::" + Util::StrTimeUnitLong(sim->getReplicationBaseTimeUnit()) + ");", tabs);
        text2 = sim->isShowReportsAfterSimulation()?"true":"false";
        text += _addCppCodeLine("sim->setsetShowReportsAfterSimulation("+text2+");", tabs);
        code->insert({"7simulation", text});

        text = _addCppCodeLine("// simulate and show report", tabs);
        text += _addCppCodeLine("sim->start();", tabs);
        text += _addCppCodeLine("return 0;", tabs);
        tabs--;
        text += _addCppCodeLine("}", tabs);
        code->insert({"8end", text});

        // Show
        ui->plainTextEditCppCode->clear();
        for (std::pair<std::string, std::string> codeSection : *code) {
            //ui->plainTextEditCppCode->appendPlainText(QString::fromStdString("// " + codeSection.first+"\n"));
            ui->plainTextEditCppCode->appendPlainText(QString::fromStdString(codeSection.second));
        }
    } else {

    }
}

bool MainWindow::graphicalModelHasChanged() const {
    return _graphicalModelHasChanged;
}

void MainWindow::setGraphicalModelHasChanged(bool graphicalModelHasChanged) {
    _graphicalModelHasChanged = graphicalModelHasChanged;
    _actualizeTabPanes();
}

bool MainWindow::_createModelImage() {
    bool res = this->_setSimulationModelBasedOnText();
    std::string dot = "digraph G {\n";
    dot += "  compound=true; rankdir=LR; \n";
    std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap = new std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>();

    std::list<SourceModelComponent*>* sources = simulator->getModelManager()->current()->getComponents()->getSourceComponents();
    std::list<ModelDataDefinition*>* visited = new std::list<ModelDataDefinition*>();
    std::list<ModelDataDefinition*>::iterator visitedIt;
    for (SourceModelComponent* source : *sources) {
        visitedIt = std::find(visited->begin(), visited->end(), source);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(source, visited, dotmap);
        }
    }
    std::list<ModelComponent*>* transfers = simulator->getModelManager()->current()->getComponents()->getTransferInComponents();
    for (ModelComponent* transfer : *transfers) {
        visitedIt = std::find(visited->begin(), visited->end(), transfer);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(transfer, visited, dotmap);
        }
    }
    std::list<ModelComponent*>* allComps = simulator->getModelManager()->current()->getComponents()->getAllComponents();
    for (ModelComponent* comp : *allComps) {
        visitedIt = std::find(visited->begin(), visited->end(), comp);
        if (visitedIt == visited->end()) {
            _recursiveCreateModelGraphicPicture(comp, visited, dotmap);
        }
    }
    // combine all level subgraphs
    unsigned int modelLevel = simulator->getModelManager()->current()->getLevel();
    for (std::pair<unsigned int, std::map<unsigned int, std::list<std::string>*>*> dotpair : *dotmap) {
        if (dotpair.first == modelLevel) {
            dot += "\n  // model level\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0)
                    dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1)
                    dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10)
                    dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
        } else if (ui->checkBox_ShowLevels->isChecked()) {
            dot += "\n\n // submodel level  " + std::to_string(dotpair.first) + "\n";
            dot += " subgraph cluster_level_" + std::to_string(dotpair.first) + " {\n";
            dot += "   graph[style=filled; fillcolor=mistyrose2] label=\"" + simulator->getModelManager()->current()->getComponents()->find(dotpair.first)->getName() + "\";\n";
            for (std::pair<unsigned int, std::list<std::string>*> dotpair2 : *dotpair.second) {
                dot += "  {\n";
                if (dotpair2.first == 0)
                    dot += "     rank=min  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first == 1)
                    dot += "     rank=max  // " + std::to_string(dotpair2.first) + "\n";
                else if (dotpair2.first < 10)
                    dot += "     rank=same  // " + std::to_string(dotpair2.first) + "\n";
                for (std::string str : *dotpair2.second) {
                    dot += "   " + str;
                }
                dot += "  }\n";
            }
            dot += "\n }\n";
        }
    }
    dot += "}\n";
    visited->clear();
    std::string basefilename = "./.tempFixedGraphicalModelRepresentation";
    std::string dotfilename = basefilename + ".dot";
    std::string pngfilename = basefilename + ".png";
    try {
        std::ofstream savefile;
        savefile.open(dotfilename, std::ofstream::out);
        QString data = QString::fromStdString(dot);
        QStringList strList = data.split(QRegularExpression("[\n]"), Qt::SkipEmptyParts); //data.split(QRegExp("[\n]"), QString::SkipEmptyParts);
        for (unsigned int i = 0; i < strList.size(); i++) {
            savefile << strList.at(i).toStdString() << std::endl;
        }
        savefile.close();
        try {
            std::remove(pngfilename.c_str());
        } catch (...) {

        }
        try {
            std::string command = "dot -Tpng " + dotfilename + " -o " + pngfilename;
            system(command.c_str());
            QPixmap pm(QString::fromStdString(pngfilename)); // <- path to image file
            //int w = ui->label_ModelGraphic->width();
            //int h = ui->label_ModelGraphic->height();
            ui->label_ModelGraphic->setPixmap(pm); //.scaled(w, h, Qt::IgnoreAspectRatio));
            ui->label_ModelGraphic->setScaledContents(false);
            try {
                //std::remove(dotfilename.c_str());
                //std::remove(pngfilename.c_str());
            } catch (...) {

            }
            return true;
        } catch (...) {
        }
    } catch (...) {
    }

    return false;
}



//-------------------------
// Simulator Trace Handlers
//-------------------------

void MainWindow::_simulatorTraceHandler(TraceEvent e) {
    std::cout << e.getText() << std::endl;
    if (e.getTracelevel() == TraceManager::Level::L1_errorFatal)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(255, 0, 0));
    else if (e.getTracelevel() == TraceManager::Level::L2_results)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(0, 0, 255));
    else if (e.getTracelevel() == TraceManager::Level::L3_errorRecover)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(223, 0, 0));
    else if (e.getTracelevel() == TraceManager::Level::L4_warning)
        ui->textEdit_Console->setTextColor(QColor::fromRgb(128, 0, 0));
    else {

        unsigned short grayVal = 20 * (static_cast<unsigned int> (e.getTracelevel()) - 5);
        ui->textEdit_Console->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    ui->textEdit_Console->append(QString::fromStdString(e.getText()));
    ui->textEdit_Console->moveCursor(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::MoveAnchor);
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceErrorHandler(TraceErrorEvent e) {

    std::cout << e.getText() << std::endl;
    ui->textEdit_Console->setTextColor(QColor::fromRgb(255, 0, 0));
    ui->textEdit_Console->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceSimulationHandler(TraceSimulationEvent e) {
    std::cout << e.getText() << std::endl;
    if (e.getText().find("Event {time=") != std::string::npos) {
        ui->textEdit_Simulation->setTextColor(QColor::fromRgb(0, 0, 128));
    } else {

        unsigned short grayVal = 20 * (static_cast<unsigned int> (e.getTracelevel()) - 5);
        ui->textEdit_Simulation->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    ui->textEdit_Simulation->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

void MainWindow::_simulatorTraceReportsHandler(TraceEvent e) {

    std::cout << e.getText() << std::endl;
    ui->textEdit_Reports->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

//
// simulator event handlers
//

void MainWindow::_onModelCheckSuccessHandler(ModelEvent* re) {
    // create (and positione and draw) or remove GraphicalModelDataDefinitions based on what actually exists on the model
    Model* model = simulator->getModelManager()->current();
    if (simulator->getModelManager()->current() == re->getModel()) { // the current model is the one changed
        ModelDataManager* dm = model->getDataManager();
        ModelGraphicsView* modelGraphView = ((ModelGraphicsView*)(ui->graphicsView));
        for(auto elemclassname: *dm->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* elem: *dm->getDataDefinitionList(elemclassname)->list()) {
                Util::identification id = elem->getId();
                //modelGraphView->;
            }
        }
    }
}

void MainWindow::_onReplicationStartHandler(SimulationEvent * re) {

    ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
    QString text = QString::fromStdString(std::to_string(sim->getCurrentReplicationNumber())) + "/" + QString::fromStdString(std::to_string(sim->getNumberOfReplications()));
    ui->label_ReplicationNum->setText(text);
    int row = ui->tableWidget_Simulation_Event->rowCount();
    ui->tableWidget_Simulation_Event->setRowCount(row + 1);
    QTableWidgetItem* newItem;
    newItem = new QTableWidgetItem(QString::fromStdString("Replication " + std::to_string(re->getCurrentReplicationNumber())));
    ui->tableWidget_Simulation_Event->setItem(row, 2, newItem);

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationStartHandler(SimulationEvent * re) {
    _actualizeActions();
    ui->progressBarSimulation->setMaximum(simulator->getModelManager()->current()->getSimulation()->getReplicationLength());
    ui->tableWidget_Simulation_Event->setRowCount(0);
    ui->tableWidget_Entities->setRowCount(0);
    ui->tableWidget_Variables->setRowCount(0);
    ui->textEdit_Simulation->clear();
    ui->textEdit_Reports->clear();

    // Fator de conversão para segundos
    Util::TimeUnit replicationBaseTimeUnit = simulator->getModelManager()->current()->getSimulation()->getReplicationBaseTimeUnit();
    double conversionFactorToSeconds = Util::TimeUnitConvert(replicationBaseTimeUnit, Util::TimeUnit(5));
    AnimationTimer::setConversionFactorToSeconds(conversionFactorToSeconds);

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationPausedHandler(SimulationEvent * re) {
    _actualizeActions();
    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationResumeHandler(SimulationEvent * re) {
    _actualizeActions();

    if (myScene()->getAnimationPaused()) {
        if (!myScene()->getAnimationPaused()->empty()) {
            QList<AnimationTransition *> *animationPaused = myScene()->getAnimationPaused()->value(re->getCurrentEvent());

            if (animationPaused) {
                if (!animationPaused->empty()) {
                    for (AnimationTransition *animation : *animationPaused) {
                        myScene()->runAnimateTransition(animation, re->getCurrentEvent(), true);
                    }
                    animationPaused->clear();
                }
            }
            myScene()->getAnimationPaused()->clear();
        }
    }

    QCoreApplication::processEvents();
}

void MainWindow::_onSimulationEndHandler(SimulationEvent * re) {
    myScene()->getAnimationPaused()->clear();
    _actualizeActions();
    ui->tabWidgetCentral->setCurrentIndex(CONST.TabCentralReportsIndex);
    for (unsigned int i = 0; i < 50; i++) {
        QCoreApplication::processEvents();
    }

    myScene()->clearAnimationsQueue();

    _modelCheked = false;
}

void MainWindow::_onProcessEventHandler(SimulationEvent * re) {
    ui->progressBarSimulation->setValue(simulator->getModelManager()->current()->getSimulation()->getSimulatedTime());
    _actualizeSimulationEvents(re);
    _actualizeDebugEntities(false);
    _actualizeDebugVariables(false);
    _actualizeGraphicalModel(re);
    QCoreApplication::processEvents();
}

void MainWindow::_onEntityCreateHandler(SimulationEvent* re) {

}

void MainWindow::_onEntityRemoveHandler(SimulationEvent* re) {

}
//-----------------------------------------

void MainWindow::_onSceneMouseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
    QPointF pos = mouseEvent->scenePos();
    ui->labelMousePos->setText(QString::fromStdString("<" + std::to_string((int) pos.x()) + "," + std::to_string((int) pos.y()) + ">"));
}

void MainWindow::_onSceneWheelInEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value + TraitsGUI<GMainWindow>::zoomButtonChange);
}

void MainWindow::_onSceneWheelOutEvent() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value - TraitsGUI<GMainWindow>::zoomButtonChange);
}

void MainWindow::_onSceneGraphicalModelEvent(GraphicalModelEvent* event) {
    _actualizeTabPanes();
}

//-----------------------------------------

void MainWindow::sceneChanged(const QList<QRectF> &region) {
    bool canUndo = ui->graphicsView->getScene()->getUndoStack()->canUndo();
    bool canRedo = ui->graphicsView->getScene()->getUndoStack()->canRedo();

    ui->actionEditUndo->setEnabled(canUndo);
    ui->actionEditRedo->setEnabled(canRedo);

    _textModelHasChanged = canUndo;

    ui->graphicsView->scene()->update();

    bool res = _checkItemsScene();

    if (res) {
        ui->actionEditCut->setEnabled(true);
        ui->actionEditCopy->setEnabled(true);
    } else {
        ui->actionEditCut->setEnabled(false);
        ui->actionEditCopy->setEnabled(false);
    }

    if (!_draw_copy->empty() || !_gmc_copies->empty() || !_group_copy->empty() || !_ports_copies->empty()) {
        ui->actionEditPaste->setEnabled(true);
    } else {
        ui->actionEditPaste->setEnabled(false);
    }

    if (ui->graphicsView->getScene()->connectingStep() == 0)
        ui->actionGModelShowConnect->setChecked(false);

    ui->graphicsView->scene()->update();
}

bool MainWindow::_checkItemsScene() {
    bool res = false;

    QList<QGraphicsItem *> *components = myScene()->getGraphicalModelComponents();
    QList<QGraphicsItem *> *geometries = myScene()->getGraphicalGeometries();
    QList<AnimationCounter *> *counters = myScene()->getAnimationsCounter();
    QList<AnimationVariable *> *variables = myScene()->getAnimationsVariable();
    QList<AnimationTimer *> *timers = myScene()->getAnimationsTimer();

    if (!components->empty() || !geometries->empty() || !counters->empty() || !variables->empty() || !timers->empty()) {
        res = true;
    }

    return res;
}
void MainWindow::sceneFocusItemChanged(QGraphicsItem *newFocusItem, QGraphicsItem *oldFocusItem, Qt::FocusReason reason) {
    // int a = 0;
}
//void sceneRectChanged(const QRectF &rect){}

void MainWindow::sceneSelectionChanged() {
    QGraphicsItem * item;
    GraphicalModelComponent* gmc;

    if (!ui->graphicsView->selectedItems().isEmpty()) {
        if (ui->graphicsView->selectedItems().size() == 1) {
            item = ui->graphicsView->selectedItems().at(0);
            gmc = dynamic_cast<GraphicalModelComponent*> (item);
            if (gmc != nullptr) {
                ui->treeViewPropertyEditor->setActiveObject(gmc, gmc->getComponent(), propertyGenesys, propertyList, propertyEditorUI, propertyBox);
                return;
            }
        }
    }
    // Se nenhum item estiver selecionado ou se mais de um item estiver selecionado
    ui->treeViewPropertyEditor->clear();
}

//-----------------------------------------

void MainWindow::sceneGraphicalModelChanged() {
    _actualizeTabPanes();
}

//-----------------------------------------

void MainWindow::_initModelGraphicsView() {
    ((ModelGraphicsView*) (ui->graphicsView))->setSceneMouseEventHandler(this, &MainWindow::_onSceneMouseEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelInEventHandler(this, &MainWindow::_onSceneWheelInEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelOutEventHandler(this, &MainWindow::_onSceneWheelOutEvent);
    ((ModelGraphicsView*) (ui->graphicsView))->setGraphicalModelEventHandler(this, &MainWindow::_onSceneGraphicalModelEvent);
    connect(ui->graphicsView->scene(), &QGraphicsScene::changed, this, &MainWindow::sceneChanged);
    connect(ui->graphicsView->scene(), &QGraphicsScene::focusItemChanged, this, &MainWindow::sceneFocusItemChanged);
    connect(ui->graphicsView->scene(), &QGraphicsScene::selectionChanged, this, &MainWindow::sceneSelectionChanged);

    // Cria uma stack undo/redo
    ui->graphicsView->getScene()->setUndoStack(new QUndoStack(this));
}

void MainWindow::_setOnEventHandlers() {
    OnEventManager* eventManager = simulator->getModelManager()->current()->getOnEvents();
    eventManager->addOnAfterProcessEventHandler(this, &MainWindow::_onAfterProcessEvent);
    eventManager->addOnEntityCreateHandler(this, &MainWindow::_onEntityCreateHandler);
    eventManager->addOnEntityRemoveHandler(this, &MainWindow::_onEntityRemoveHandler);
    eventManager->addOnEntityMoveHandler(this, &MainWindow::_onMoveEntityEvent);
    eventManager->addOnProcessEventHandler(this, &MainWindow::_onProcessEventHandler);
    eventManager->addOnReplicationStartHandler(this, &MainWindow::_onReplicationStartHandler);
    eventManager->addOnSimulationStartHandler(this, &MainWindow::_onSimulationStartHandler);
    eventManager->addOnSimulationPausedHandler(this, &MainWindow::_onSimulationPausedHandler);
    eventManager->addOnSimulationResumeHandler(this, &MainWindow::_onSimulationResumeHandler);
    eventManager->addOnSimulationEndHandler(this, &MainWindow::_onSimulationEndHandler);
    //@Todo: Check for new events that were created later
}

//-------------------------
// Simulator Fake Plugins
//-------------------------

void MainWindow::_insertPluginUI(Plugin * plugin) {
    if (plugin != nullptr) {
        if (plugin->isIsValidPlugin()) {
            QTreeWidgetItem *treeItemChild = new QTreeWidgetItem();
            //QTreeWidgetItem *treeItem = new QTreeWidgetItem; //(ui->treeWidget_Plugins);
            std::string plugtextAdds = "[" + plugin->getPluginInfo()->getCategory() + "]: ";
            QBrush brush;
            if (plugin->getPluginInfo()->isComponent()) {
                plugtextAdds += " Component";
                //brush.setColor(Qt::white);
                //treeItemChild->setBackground(brush);
                //treeItemChild->setBackgroundColor(Qt::white);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/component.ico"));
            } else {
                plugtextAdds += " DataDefinition";
                //brush.setColor(Qt::lightGray);
                //treeItemChild->setBackground(brush);
                //treeItemChild->setBackgroundColor(Qt::lightGray);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/calendarred.ico"));
                //treeItemChild->setFont(QFont::Style::StyleItalic);
            }
            if (plugin->getPluginInfo()->isSink()) {
                plugtextAdds += ", Sink";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
            }
            if (plugin->getPluginInfo()->isSource()) {
                plugtextAdds += ", Source";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
            }
            if (plugin->getPluginInfo()->isReceiveTransfer()) {
                plugtextAdds += ", ReceiveTransfer";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/load.ico"));
            }
            if (plugin->getPluginInfo()->isSendTransfer()) {
                plugtextAdds += ", SendTransfer";
                treeItemChild->setForeground(0, Qt::blue); //setTextColor(0, Qt::blue);
                treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/loadinv.ico"));
            }
            //treeItem->setText(0,QString::fromStdString(plugtextAdds));
            plugtextAdds += "\n\nDescrption: " + plugin->getPluginInfo()->getDescriptionHelp();
            plugtextAdds += "\n\nTemplate: " + plugin->getPluginInfo()->getLanguageTemplate() + " (double click to add to model)";

            QTreeWidgetItem *treeRootItem;
            QString category;
            if (plugin->getPluginInfo()->isComponent())
                category = QString::fromStdString(plugin->getPluginInfo()->getCategory());
            else
                category = "Data Definition";
            QList<QTreeWidgetItem*> founds = ui->treeWidget_Plugins->findItems(category, Qt::MatchContains);
            if (founds.size() == 0) {
                QFont font("Nimbus Sans", 12, QFont::Bold);
                treeRootItem = new QTreeWidgetItem(ui->treeWidget_Plugins);
                treeRootItem->setText(0, category);
                QBrush bforeground(Qt::white);
                treeRootItem->setForeground(0, bforeground);
                QBrush bbackground(Qt::black);
                if (category == "Data Definition") {
                    bbackground.setColor(Qt::darkRed);
                } else if (category == "Discrete Processing") {
                    bbackground.setColor(Qt::darkGreen);
                } else if (category == "Decisions") {
                    bbackground.setColor(Qt::darkYellow);
                } else if (category == "Grouping") {
                    bbackground.setColor(Qt::magenta);
                } else if (category == "Input Output") {
                    bbackground.setColor(Qt::darkCyan);
                } else if (category == "Material Handling") {
                    bbackground.setColor(Qt::darkBlue);
                }
                treeRootItem->setBackground(0, bbackground);
                treeRootItem->setFont(0, font);
                treeRootItem->setExpanded(false); //(true);
                //treeRootItem->sortChildren(0, Qt::AscendingOrder);
                if (plugin->getPluginInfo()->getCategory() == category.toStdString()) {
                    _pluginCategoryColor->insert({plugin->getPluginInfo()->getCategory(), bbackground.color()});
                }
            } else {
                treeRootItem = *founds.begin();
            }
            if (plugin->getPluginInfo()->isComponent() && !plugin->getPluginInfo()->isSendTransfer() && !plugin->getPluginInfo()->isReceiveTransfer() && !plugin->getPluginInfo()->isSink() && !plugin->getPluginInfo()->isSource()) {
                if (treeRootItem->background(0).color().blue() < 32 && treeRootItem->background(0).color().green() < 32 && treeRootItem->background(0).color().red() < 32) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblack.ico"));
                } else if (treeRootItem->background(0).color().red() >= treeRootItem->background(0).color().blue() &&
                        treeRootItem->background(0).color().red() > treeRootItem->background(0).color().green()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentred.ico"));
                } else if (treeRootItem->background(0).color().blue() > treeRootItem->background(0).color().red() &&
                        treeRootItem->background(0).color().blue() > treeRootItem->background(0).color().green()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentblue.ico"));
                } else if (treeRootItem->background(0).color().red() > treeRootItem->background(0).color().blue() &&
                        treeRootItem->background(0).color().green() > treeRootItem->background(0).color().blue()) {
                    treeItemChild->setIcon(0, QIcon(":/icons3/resources/icons/pack3/ico/componentyellow.ico"));
                }
            }
            treeItemChild->setWhatsThis(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
            /* TODO: Qt6 has no more setTextColor */
            //treeItemChild->setTextColor(0, treeRootItem->background(0).color());
            //treeItemChild->setTextColor(0, treeRootItem->backgroundColor(0));
            treeItemChild->setText(0, QString::fromStdString(plugin->getPluginInfo()->getPluginTypename()));
            treeItemChild->setToolTip(0, QString::fromStdString(plugtextAdds));
            treeItemChild->setStatusTip(0, QString::fromStdString(plugin->getPluginInfo()->getLanguageTemplate()));
            //treeItemChild->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);
            treeRootItem->addChild(treeItemChild);
        }
    }
}

/*
void MainWindow::_insertFakePlugins() {
    PluginManager* pm = simulator->getPlugins();
    // TRYING SOME NEW ORGANIZATION (BASED ON ARENA 16..20)
    // ...
    //-----------------------------------------------------

    // OLD ORGANIZATION
    // model components
    // arena basic process
    (pm->insert("create.so"));
    (pm->insert("dispose.so"));
    (pm->insert("decide.so"));
    (pm->insert("batch.so"));
    (pm->insert("separate.so"));
    (pm->insert("assign.so"));
    (pm->insert("record.so"));
    (pm->insert("process.so"));
    (pm->insert("submodel.so"));
    (pm->insert("entitygroup.so"));
    (pm->insert("queue.so"));
    (pm->insert("set.so"));
    (pm->insert("resource.so"));
    (pm->insert("variable.so"));
    (pm->insert("schedule.so"));
    (pm->insert("entitygroup.so"));
    // arena advanced process
    (pm->insert("delay.so"));
    (pm->insert("dropoff.so"));
    (pm->insert("hold.so"));
    (pm->insert("match.so"));
    (pm->insert("pickup.so"));
    (pm->insert("read.so"));
    (pm->insert("write.so"));
    (pm->insert("release.so"));
    (pm->insert("remove.so"));
    (pm->insert("seize.so"));
    (pm->insert("search.so"));
    (pm->insert("signal.so"));
    (pm->insert("store.so"));
    (pm->insert("unstore.so"));
    (pm->insert("expression.so"));
    (pm->insert("failure.so"));
    (pm->insert("file.so"));
    (pm->insert("storage.so"));
    // arena transfer station
    (pm->insert("enter.so"));
    (pm->insert("leave.so"));
    (pm->insert("pickstation.so"));
    (pm->insert("route.so"));
    (pm->insert("sequence.so"));
    (pm->insert("station.so"));
    (pm->insert("label.so"));
    // arena transfer conveyour
    (pm->insert("access.so"));
    (pm->insert("exit.so"));
    (pm->insert("start.so"));
    (pm->insert("stop.so"));
    (pm->insert("conveyour.so"));
    (pm->insert("segment.so"));
    // arena transfer transport
    (pm->insert("alocate.so"));
    (pm->insert("free.so"));
    (pm->insert("halt.so"));
    (pm->insert("move.so"));
    (pm->insert("request.so"));
    (pm->insert("transporter.so"));
    (pm->insert("distance.so"));
    (pm->insert("network.so"));
    (pm->insert("networklink.so"));
    // others
    (pm->insert("dummy.so"));
    (pm->insert("lsode.so"));
    (pm->insert("biochemical.so"));
    (pm->insert("markovchain.so"));
    (pm->insert("cellularautomata.so"));
    (pm->insert("cppforg.so"));
    // now complete the information
    simulator->getPlugins()->completePluginsFieldsAndTemplates();
    for (unsigned int i = 0; i < simulator->getPlugins()->size(); i++) {
        //@TODO: now it's the opportunity to adjust template
        _insertPluginUI(simulator->getPlugins()->getAtRank(i));
    }
}
*/

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

//-------------------------
// PRIVATE SLOTS
//-------------------------


// -------------------------------------
// on Widgets
// -------------------------------------

void MainWindow::on_tabWidget_Model_tabBarClicked(int index) {

}

void MainWindow::on_checkBox_ShowElements_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_checkBox_ShowInternals_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_horizontalSlider_Zoom_valueChanged(int value) {
    double factor = ((double) value / 100.0)*(2 - 0.5) + 0.5;
    double scaleFactor = 1.0;
    // @TODO: Qt5 -- Q_ASSERT(ui->label_ModelGraphic->pixmap());
    scaleFactor *= factor;
    ui->label_ModelGraphic->resize(scaleFactor * ui->label_ModelGraphic->pixmap().size());
    //adjustScrollBar(ui->scrollArea_Graphic->horizontalScrollBar(), factor);
    //adjustScrollBar(ui->scrollArea_Graphic->verticalScrollBar(), factor);

    //void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor){
    //    scrollBar->setValue(int(factor * scrollBar->value()
    //                            + ((factor - 1) * scrollBar->pageStep()/2)));
    //}

    //zoomInAct->setEnabled(scaleFactor < 3.0);
    //zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void MainWindow::on_checkBox_ShowRecursive_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_checkBox_ShowLevels_stateChanged(int arg1) {
    bool result = _createModelImage();
}

void MainWindow::on_tabWidget_Debug_currentChanged(int index) {
    _actualizeActions();
}

void MainWindow::on_pushButton_Breakpoint_Insert_clicked() {
    //ModelSimulation* sim = simulator->getModels()->current()->getSimulation();
    dialogBreakpoint* dialog = new dialogBreakpoint();
    dialog->setMVCModel(simulator);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();
    std::string type, on;
    dialogBreakpoint::MVCResult* result = dialog->getMVCResult();
    if (result->type == "Time") {

    } else if (result->type == "Entity") {

    } else if (result->type == "Component") {

    }

    dialog->~dialogBreakpoint();
    _actualizeDebugBreakpoints(true);
}

void MainWindow::on_pushButton_Breakpoint_Remove_clicked() {
    ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
}

void MainWindow::on_tabWidgetCentral_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetCentral_tabBarClicked(int index) {
}

void MainWindow::on_treeWidget_Plugins_itemDoubleClicked(QTreeWidgetItem *item, int column) {
    if (ui->TextCodeEditor->isEnabled()) { // add text to modelsimulation
        /*
        if (item->toolTip(0).contains("DataDefinition")) {
            QTextCursor cursor = ui->TextCodeEditor->textCursor();
            QTextCursor cursorSaved = cursor;
            cursor.movePosition(QTextCursor::Start);
            ui->TextCodeEditor->setTextCursor(cursor);
            if (ui->TextCodeEditor->find("# Model Components")) {
                ui->TextCodeEditor->moveCursor(QTextCursor::MoveOperation::Left, QTextCursor::MoveMode::MoveAnchor);
                ui->TextCodeEditor->moveCursor(QTextCursor::MoveOperation::Up, QTextCursor::MoveMode::MoveAnchor);
                ui->TextCodeEditor->insertPlainText(item->statusTip(0) + "\n");
            } else {
                ui->TextCodeEditor->appendPlainText(item->statusTip(0));
            }
        } else {
            ui->TextCodeEditor->appendPlainText(item->statusTip(0));
        }
         */
    } else {
        // treeRoot? Always?
        for (int i = 0; i < ui->treeWidget_Plugins->topLevelItemCount(); i++) {
            //if (ui->treeWidget_Plugins->topLevelItem(i) != item) {
            ui->treeWidget_Plugins->topLevelItem(i)->setExpanded(false);
            //} else {
            //	ui->treeWidget_Plugins->expandItem(item);
            //	//ui->treeWidget_Plugins->topLevelItem(i)->setExpanded(true);
            //}
        }
        //ui->treeWidget_Plugins->setAnimated(true);
        ui->treeWidget_Plugins->expandItem(item);
    }
}

void MainWindow::on_graphicsView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint) {
    _showMessageNotImplemented();
}

void MainWindow::on_horizontalSlider_ZoomGraphical_valueChanged(int value) {
    double factor = (value - _zoomValue)*0.002;
    _zoomValue = value;
    _gentle_zoom(1.0 + factor);
}

void MainWindow::on_actionConnect_triggered() {
    ((ModelGraphicsView*) ui->graphicsView)->beginConnection();
}

void MainWindow::on_pushButton_Export_clicked() {
    _showMessageNotImplemented();
}

void MainWindow::on_tabWidgetModelLanguages_currentChanged(int index) {
    if (index == CONST.TabModelSimLangIndex) {
        if (_graphicalModelHasChanged) {
            _actualizeModelSimLanguage();
        }
    } else if (index == CONST.TabModelCppCodeIndex) {
        _actualizeModelCppCode();
    }
    _actualizeActions();
}

void MainWindow::on_actionComponent_Breakpoint_triggered() {
    if (ui->graphicsView->selectedItems().size() == 1) {
        QGraphicsItem* gi = ui->graphicsView->selectedItems().at(0);
        GraphicalModelComponent* gmc = dynamic_cast<GraphicalModelComponent*> (gi);
        if (gmc != nullptr) {
            ModelComponent* mc = gmc->getComponent();
            ModelSimulation* sim = simulator->getModelManager()->current()->getSimulation();
            if (sim->getBreakpointsOnComponent()->find(mc) == sim->getBreakpointsOnComponent()->list()->end()) {
                sim->getBreakpointsOnComponent()->insert(mc);
            } else {
                sim->getBreakpointsOnComponent()->remove(mc);
            }
        }
        _actualizeDebugBreakpoints(false);
    }
}

void MainWindow::on_treeWidgetComponents_itemSelectionChanged() {
    _showMessageNotImplemented();
}

void MainWindow::on_treeWidget_Plugins_itemClicked(QTreeWidgetItem *item, int column) {
    //showMessageNotImplemented();
}

void MainWindow::on_TextCodeEditor_textChanged() {
    this->_actualizeModelTextHasChanged(true);
}

void MainWindow::on_tabWidgetModel_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetSimulation_currentChanged(int index) {
    _actualizeTabPanes();
}

void MainWindow::on_tabWidgetReports_currentChanged(int index) {
    _actualizeTabPanes();
}

//-------------------------
// PRIVATE SLOTS
//-------------------------

// -------------------------------------------------
//  menu actions
// -------------------------------------------------


void MainWindow::on_actionSimulationStop_triggered() {
    AnimationTransition::setRunning(false);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("stop");

    simulator->getModelManager()->current()->getSimulation()->stop();

    _actualizeActions();
}

void MainWindow::on_actionSimulationStart_triggered() {
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);

    bool res = true;

    // Checha o modelo antes de começar
    if (!_modelCheked) {
        res = _check(false);
    }

    if (res) {
        _insertCommandInConsole("start");
        if (_setSimulationModelBasedOnText())
            simulator->getModelManager()->current()->getSimulation()->start();
    }
}

void MainWindow::on_actionSimulationStep_triggered() {
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);

    bool res = true;

    if (!_modelCheked) {
        res = _check(false);
    }

    if (res) {
        _insertCommandInConsole("step");

        if (_setSimulationModelBasedOnText())
            simulator->getModelManager()->current()->getSimulation()->step();
    }
}

void MainWindow::on_actionSimulationPause_triggered() {
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(true);

    _insertCommandInConsole("pause");
    simulator->getModelManager()->current()->getSimulation()->pause();
}

void MainWindow::on_actionSimulationResume_triggered() {
    AnimationTransition::setRunning(true);
    AnimationTransition::setPause(false);

    _insertCommandInConsole("resume");

    if (_setSimulationModelBasedOnText())
        simulator->getModelManager()->current()->getSimulation()->start();
}


void MainWindow::on_actionAboutAbout_triggered() {
    QMessageBox::about(this, "About Genesys", "Genesys is a result of teaching and research activities of Professor Dr. Ing Rafael Luiz Cancian. It began in early 2002 as a way to teach students the basics and simulation techniques of systems implemented by other comercial simulation tools, such as Arena. In Genesys development he replicated all the SIMAN language, used by Arena software, and Genesys has become a clone of that tool, including its graphical interface. Genesys allowed the inclusion of new simulation components through dynamic link libraries and also the parallel execution of simulation models in a distributed environment. The development of Genesys continued until 2009, when the professor stopped teaching systems simulation classes. Ten years later the professor starts again to teach systems simulation classes and to carry out scientific research in the area. So in 2019 Genesys is reborn, with new language and programming techniques, and even more ambitious goals.");
}

void MainWindow::on_actionAboutLicence_triggered() {
    LicenceManager* licman = simulator->getLicenceManager();
    std::string text = licman->showLicence() + "\n";
    text += licman->showLimits() + "\n";
    text += licman->showActivationCode();
    QMessageBox::about(this, "About Licence", QString::fromStdString(text));
}

void MainWindow::on_actionAboutGetInvolved_triggered() {
    QMessageBox::about(this, "Get Inveolved", "Genesys is a free open-source simulator (and tools) available at 'https://github.com/rlcancian/Genesys-Simulator'. Help us by submiting your pull requests containing code improvements. Contact: rafael.cancian@ufsc.br");
}

void MainWindow::on_actionEditUndo_triggered() {
    if (ui->graphicsView->getScene()->getUndoStack()) {
        ui->graphicsView->getScene()->getUndoStack()->undo();
    }
}


void MainWindow::on_actionEditRedo_triggered() {
    if (ui->graphicsView->getScene()->getUndoStack()) {
        ui->graphicsView->getScene()->getUndoStack()->redo();
    }
}


void MainWindow::on_actionEditFind_triggered() {
    // Cria um novo diálogo para Buscar componentes
    DialogFind *find = new DialogFind(this, ui->graphicsView->getScene());

    // Mostra esse dialogo na tela
    find->show();

    if (find->exec() == QDialog::Accepted) find->setFocus();
}


void MainWindow::on_actionEditReplace_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditCut_triggered() {
    _gmc_copies->clear();
    _ports_copies->clear();
    _group_copy->clear();
    _draw_copy->clear();

    QList<QGraphicsItem *> selecteds =  ui->graphicsView->scene()->selectedItems();

    // Verifica se tem itens selecionados
    if (selecteds.size() > 0) {

        // Pega a cena
        ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->getScene());

        // Seta o cut
        _cut = true;

        // Adiciona na lista de cópias (conexões, componentes e desenhos)
        foreach (QGraphicsItem *item , ui->graphicsView->scene()->selectedItems()) {
            QList<GraphicalModelComponent*> groupComponents  = QList<GraphicalModelComponent*>();
            QList<GraphicalConnection*> * connGroup = new QList<GraphicalConnection*>();

            // Tenta transformar em um componente gráfico de modelo
            if (GraphicalModelComponent *gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                // Adiciona em uma lista de cópias de componentes
                _gmc_copies->append(gmc);
            }
            else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(group->childItems().at(i));

                    if (!component->getGraphicalInputPorts().empty() && !component->getGraphicalInputPorts().at(0)->getConnections()->empty()) {
                        for (int j = 0; j < component->getGraphicalInputPorts().at(0)->getConnections()->size(); ++j) {
                            connGroup->append(component->getGraphicalInputPorts().at(0)->getConnections()->at(j));
                        }
                    }

                    for (int j = 0; j < component->getGraphicalOutputPorts().size(); ++j) {
                        GraphicalComponentPort *port = component->getGraphicalOutputPorts().at(j);

                        if (!port->getConnections()->empty()) {
                            connGroup->append(port->getConnections()->at(0));
                        }
                    }

                    _gmc_copies->append(component);
                    groupComponents.append(component);
                }
                saveItemForCopy(&groupComponents, connGroup);

                _group_copy->append(group);
                ui->graphicsView->getScene()->insertComponentGroup(group, groupComponents);
                for (unsigned int k = 0; k < (unsigned int) connGroup->size(); k++) {
                    _ports_copies->append(connGroup->at(k));
                }
            } else if (GraphicalConnection *port = dynamic_cast<GraphicalConnection*>(item)) {
                _ports_copies->append(port);
            } else {
                _draw_copy->append(item);
            }

            delete connGroup;
        }

        // Removendo as conexoes do modelo e graficamente
        // Só não é removido a conexão quando todos os itens estão selecionados
        // (2x componentes e a conexão (similar ao arena)
        saveItemForCopy(_gmc_copies, _ports_copies);

        QUndoCommand *deleteUndoCommand = new DeleteUndoCommand(selecteds, scene);
        scene->getUndoStack()->push(deleteUndoCommand);

    }
}

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


void MainWindow::on_actionEditCopy_triggered() {
    _gmc_copies->clear();
    _ports_copies->clear();
    _group_copy->clear();
    _draw_copy->clear();

    QList<QGraphicsItem*> selected = ui->graphicsView->scene()->selectedItems();
    QList<GraphicalModelComponent *> gmc_copies_copy = QList<GraphicalModelComponent *>();

    // verifica se tem itens selecionados
    if (selected.size() > 0) {

        // seta o cut
        _cut = false;

        // adiciona na lista de cópias (conexões, componentes e desenhos)
        foreach (QGraphicsItem *item , ui->graphicsView->scene()->selectedItems()) {
            // verifica se é um componente gráfico
            if (GraphicalModelComponent *gmc = dynamic_cast<GraphicalModelComponent*>(item)) {
                // Adiciona em uma lista de cópias de componentes
                gmc->setSelected(false);
                _gmc_copies->append(gmc);
                gmc_copies_copy.append(gmc);
            }
            // verifica se é uma conexão gráfica
            else if (GraphicalConnection *conn = dynamic_cast<GraphicalConnection*>(item)) {
                conn->setSelected(false);
                _ports_copies->append(conn);
            }
            // verifica se é um grupo
            else if (QGraphicsItemGroup *group = dynamic_cast<QGraphicsItemGroup*>(item)) {
                group->setSelected(false);
                _group_copy->append(group);

                for (int i = 0; i < group->childItems().size(); i++) {
                    GraphicalModelComponent * component = dynamic_cast<GraphicalModelComponent *>(group->childItems().at(i));

                    gmc_copies_copy.append(component);
                }
            }
            // se não for nenhum deles é um desenho na tela
            else {
                item->setSelected(false);
                _draw_copy->append(item);
            }
        }

        // removendo as conexões em que os seus componentes não foram selecionados
        saveItemForCopy(&gmc_copies_copy, _ports_copies);

        // limpa a lista auxiliar
        gmc_copies_copy.clear();
    }

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
        simulator->getModelManager()->current()->getComponents()->insert(previousComponent);

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
            simulator->getModelManager()->current()->getComponents()->insert(previousComponent);

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

void MainWindow::on_actionEditPaste_triggered() {

    // se tiver componente copiados
    if (_gmc_copies->size() > 0 || _draw_copy->size() > 0 || _group_copy->size() > 0) {

        // pega a cena
        ModelGraphicsScene *scene = (ModelGraphicsScene *)(ui->graphicsView->getScene());

        // se não for ação de recorte chama o auxiliar do copy
        if (!_cut) {
            _helpCopy();
        }

        // cola na cena o que foi copiado
        QUndoCommand *pasteUndoCommand = new PasteUndoCommand(_gmc_copies, _ports_copies, _group_copy, _draw_copy, scene);
        scene->getUndoStack()->push(pasteUndoCommand);

        // limpa todas as listas
        _gmc_copies->clear();
        _ports_copies->clear();
        _draw_copy->clear();
        _group_copy->clear();
        _cut = false;
    }
}


void MainWindow::on_actionShowGrid_triggered() {
    ui->graphicsView->getScene()->showGrid();
}


void MainWindow::on_actionShowRule_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionShowGuides_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionZoom_In_triggered() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value+TraitsGUI<GMainWindow>::zoomButtonChange);
}


void MainWindow::on_actionZoom_Out_triggered() {
    int value = ui->horizontalSlider_ZoomGraphical->value();
    ui->horizontalSlider_ZoomGraphical->setValue(value-TraitsGUI<GMainWindow>::zoomButtonChange);

}


void MainWindow::on_actionZoom_All_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionDrawLine_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawLine->isChecked()) {
        ui->graphicsView->setCursor(Qt::SizeHorCursor);
        ui->actionDrawLine->setChecked(true);
        // Ative a ferramenta de desenho de linha
        scene->setAction(ui->actionDrawLine);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::LINE); // Enumeração que representa o modo de desenho de linha
    } else {
        unselectDrawIcons();
    }
}


void MainWindow::on_actionDrawRectangle_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    // Ative a ferramenta de desenho de retangulo
    if (!checkSelectedDrawIcons() && ui->actionDrawRectangle->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->actionDrawRectangle->setChecked(true);
        scene->setAction(ui->actionDrawRectangle);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::RECTANGLE);
    } else {
        unselectDrawIcons();
    }
}


void MainWindow::on_actionDrawEllipse_triggered() {
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    // Ative a ferramenta de desenho de ellipse
    if (!checkSelectedDrawIcons() && ui->actionDrawEllipse->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        ui->actionDrawEllipse->setChecked(true);
        scene->setAction(ui->actionDrawEllipse);
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::ELLIPSE);
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionDrawText_triggered()
{
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawText->isChecked()) {
        ui->actionDrawText->setChecked(true);
        scene->setAction(ui->actionDrawText);
        // Ative a ferramenta de desenho do texto
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::TEXT);
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionDrawPoligon_triggered()
{
    ModelGraphicsScene* scene = ui->graphicsView->getScene();
    if (!checkSelectedDrawIcons() && ui->actionDrawPoligon->isChecked()) {
        ui->graphicsView->setCursor(Qt::ArrowCursor);
        ui->actionDrawPoligon->setChecked(true);
        scene->setAction(ui->actionDrawPoligon);
        // Ative a ferramenta de desenho do polygon
        scene->setDrawingMode(ModelGraphicsScene::DrawingMode::POLYGON);
    } else {
        unselectDrawIcons();
    }
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

void MainWindow::on_actionAnimateExpression_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateResource_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateQueue_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateStation_triggered() {
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditDelete_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionSimulatorPreferences_triggered()
{
    DialogSystemPreferences* dialog = new DialogSystemPreferences(this);
    dialog->show();
}


void MainWindow::on_actionAlignMiddle_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignTop_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignRight_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignCenter_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAlignLeft_triggered()
{
    _showMessageNotImplemented();
}

void MainWindow::on_actionAnimateCounter_triggered()
{
    if (!checkSelectedDrawIcons() && ui->actionAnimateCounter->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateCounter);
        myScene()->drawingCounter();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateVariable_triggered() {
    if (!checkSelectedDrawIcons() && ui->actionAnimateVariable->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateVariable);
        myScene()->drawingVariable();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateSimulatedTime_triggered()
{
    if (!checkSelectedDrawIcons() && ui->actionAnimateSimulatedTime->isChecked()) {
        ui->graphicsView->setCursor(Qt::CrossCursor);
        myScene()->setAction(ui->actionAnimateSimulatedTime);
        myScene()->drawingTimer();
    } else {
        unselectDrawIcons();
    }
}

void MainWindow::on_actionAnimateEntity_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateEvent_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateAttribute_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimateStatistics_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditGroup_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionEditUngroup_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsParserGrammarChecker_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsExperimentation_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsOptimizator_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionToolsDataAnalyzer_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionAnimatePlot_triggered()
{
    _showMessageNotImplemented();
}


void MainWindow::on_actionViewConfigure_triggered()
{
    _showMessageNotImplemented();
}

//void MainWindow::on_actionConfigure_triggered() {//?????????????????????????
//}
//void MainWindow::on_actionOpen_triggered() {//?????????????????????????
//}

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

void MainWindow::on_actionModelNew_triggered() {
    Model* m;
    if ((m = simulator->getModelManager()->current()) != nullptr) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "New Model", "There is a model already oppened. Do you want to close it and to create new model?", QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        } else {
            this->on_actionModelClose_triggered();
            //return; //@TODO Ceck if needed (since will remove bellow)
        }
    }
    _insertCommandInConsole("new");
    if (m != nullptr) {
        simulator->getModelManager()->remove(m);
    }
    m = simulator->getModelManager()->newModel();
    _initUiForNewModel(m);
}

void MainWindow::on_actionModelOpen_triggered()
{
    Model *m;
    if ((m = simulator->getModelManager()->current()) != nullptr) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("New Model");
        msgBox.setText("There is a model already opened. Do you want to close it and create a new model?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int reply = msgBox.exec();

        if (reply == QMessageBox::No) return;
        else on_actionModelClose_triggered();
    }

    // Obtém o diretório atual
    QString currentDirectory = QDir::currentPath();

    // Navega para o diretório desejado
    QDir parentDir(currentDirectory);

    // Sobe 5 pastas
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();
    parentDir.cdUp();

    // Entra na pasta models
    parentDir.cd("models");

    // Define o diretório inicial como a pasta "models"
    QString initialDirectory = parentDir.absolutePath();

    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Model", initialDirectory,
        tr("Genesys Model (*.gen);;Genesys Graphical User Interface (*.gui);;XML Files (*.xml);;JSON Files (*.json);;C++ Files (*.cpp)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName == "") {
        return;
    }
    _insertCommandInConsole("load " + fileName.toStdString());
    // load Model (in the simulator)
    Model *model = this->_loadGraphicalModel(fileName.toStdString());
    if (model != nullptr) {
        _loaded = true;
        _initUiForNewModel(model);

        QMessageBox::information(this, "Open Model", "Model successfully oppened");
    } else {
        QMessageBox::warning(this, "Open Model", "Error while opening model");
        _actualizeActions();
        _actualizeTabPanes();
    }
    ui->graphicsView->getScene()->getUndoStack()->clear();

}


void MainWindow::on_actionModelSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Model"), _modelfilename,
            tr("Genesys Model (*.gen)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;
    else {
        _insertCommandInConsole("save " + fileName.toStdString());
        QString finalFileName = fileName + ".gen";
        QFile saveFile(finalFileName);

        if (!saveFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to access file to save"),
                    saveFile.errorString());
            return;
        } else {
            _saveTextModel(&saveFile, ui->TextCodeEditor->toPlainText());
            saveFile.close();
        }
        _saveGraphicalModel(fileName + ".gui");
        _modelfilename = fileName;
        QMessageBox::information(this, "Save Model", "Model successfully saved");
        // convert text info Model
        _setSimulationModelBasedOnText();
        //
        _actualizeModelTextHasChanged(false);
    }
    _actualizeActions();
    ui->graphicsView->getScene()->getUndoStack()->clear();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // limpando referencia do ultimo elemento selecionado em property editor
    ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();

    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionModelClose_triggered()
{
    if (_textModelHasChanged || simulator->getModelManager()->current()->hasChanged()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle("Close ModelSyS");
        msgBox.setText("Model has changed. Do you want to save it?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int reply = msgBox.exec();

        if (reply == QMessageBox::Yes) {
            this->on_actionModelSave_triggered();
        }
    }
    _insertCommandInConsole("close");

    // quando a cena é fechada, limpo o grid associado a ela
    ui->graphicsView->getScene()->grid()->clear();
    // volto o botao de grid para "não clicado"
    ui->actionShowGrid->setChecked(false);

    // limpando tudo a que se refere à cena
    ui->graphicsView->getScene()->getUndoStack()->clear();
    ui->graphicsView->getScene()->clearAnimationsQueue();
    ui->graphicsView->getScene()->getGraphicalModelComponents()->clear();
    ui->graphicsView->getScene()->getGraphicalConnections()->clear();
    ui->graphicsView->getScene()->getAllComponents()->clear();
    ui->graphicsView->getScene()->getAllConnections()->clear();
    ui->graphicsView->getScene()->clearAnimations();
    ui->graphicsView->getScene()->clear();
    ui->graphicsView->clear();

    // limpando referencia do ultimo elemento selecionado em property editor
    ui->treeViewPropertyEditor->clearCurrentlyConnectedObject();

    // limpando tudo a que se refere ao modelo
    simulator->getModelManager()->current()->getComponents()->getAllComponents()->clear();
    simulator->getModelManager()->current()->getComponents()->clear();
    ui->progressBarSimulation->setValue(0); // Seta o progresso da simulação para zero
    simulator->getModelManager()->remove(simulator->getModelManager()->current());

    ui->actionActivateGraphicalSimulation->setChecked(false);

    _clearModelEditors();

    _actualizeActions();
    _actualizeTabPanes();
    //QMessageBox::information(this, "Close Model", "Model successfully closed");
}


void MainWindow::on_actionModelInformation_triggered()
{
    DialogModelInformation* diag = new DialogModelInformation(this);
    diag->show();
}


void MainWindow::on_actionModelCheck_triggered()
{
    _check();
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

void MainWindow::on_actionSimulatorExit_triggered()
{
    QMessageBox::StandardButton res;
    if (this->_textModelHasChanged) {
        res = QMessageBox::question(this, "Exit GenESyS", "Model has changed. Do you want to save it?", QMessageBox::Yes | QMessageBox::No);
        if (res == QMessageBox::Yes) {
            this->on_actionModelSave_triggered();
            return;
        }
    }
    res = QMessageBox::question(this, "Exit GenESyS", "Do you want to exit GenESyS?", QMessageBox::Yes | QMessageBox::No);
    if (res == QMessageBox::Yes) {
        std::exit(EXIT_SUCCESS);
    } else {
        // it does not quit, but the window is closed. Check it. @TODO
    }
}


void MainWindow::on_actionSimulationConfigure_triggered()
{
    DialogSimulationConfigure * dialog = new DialogSimulationConfigure(this);
    dialog->setSimulator(simulator);
    dialog->previousConfiguration();
    dialog->show();

}


void MainWindow::on_treeWidgetDataDefnitions_itemDoubleClicked(QTreeWidgetItem *item, int column)
{

    // Check if the column index is 2 (Name column)
    if (column == 2) {

        // Set the Qt::ItemIsEditable flag to enable editing for the specific item
        // It's required to set the flag here because otherwise all the other fields could changed too.
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        // Initiate the editing of the specified item in the specified column in the QTreeWidget
        ui->treeWidgetDataDefnitions->editItem(item, column);

         // Reset the Qt::ItemIsEditable flag to disable further editing after the edit operation
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    }
}

void MainWindow::on_treeWidgetDataDefnitions_itemChanged(QTreeWidgetItem *item, int column)
{

    // Check if the column index is 2 (Name column)
    if (column == 2) {

        // Get the changes
        QString after = item->text(column);
        Model * m = simulator->getModelManager()->current();

        // Save in the model
        for (std::string dataTypename : *m->getDataManager()->getDataDefinitionClassnames()) {
            for (ModelDataDefinition* comp : *m->getDataManager()->getDataDefinitionList(dataTypename)->list()) {

                QString id = QString::fromStdString(Util::StrIndex(comp->getId()));

                if (id.contains(item->text(0)))
                    comp->setName(after.toStdString());
            }
        }
    }

}


void MainWindow::on_actionShowSnap_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    if (scene->getSnapToGrid()) {
        scene->setSnapToGrid(false);
    } else {
        scene->setSnapToGrid(true);
    }
}

void MainWindow::on_actionViewGroup_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->groupComponents(false);
}


void MainWindow::on_actionViewUngroup_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->ungroupComponents();
}

void MainWindow::on_actionArranjeLeft_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(0);
}


void MainWindow::on_actionArranjeCenter_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(4);
}


void MainWindow::on_actionArranjeRight_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(1);
}


void MainWindow::on_actionArranjeTop_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(2);
}


void MainWindow::on_actionArranjeMiddle_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(5);
}


void MainWindow::on_actionArranjeBototm_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    scene->arranjeModels(3);
}

void MainWindow::on_actionGModelShowConnect_triggered()
{
    if (!ui->actionGModelShowConnect->isChecked() && !_firstClickShowConnection) {
        ui->actionGModelShowConnect->setChecked(false);
        ui->graphicsView->getScene()->setConnectingStep(0);
        ui->graphicsView->setCursor(Qt::ArrowCursor);
    } else {
        ui->actionGModelShowConnect->setChecked(true);
        ui->graphicsView->getScene()->setConnectingStep(1);
        _firstClickShowConnection = false;
    }
}

void MainWindow::on_actionSimulatorsPluginManager_triggered()
{
    DialogPluginManager* dialog = new DialogPluginManager(this);
    dialog->show();
}

//void MainWindow::on_actionteste_triggered()
//{
//    _graphicalSimulation = (!_graphicalSimulation);
//}


void MainWindow::on_actionActivateGraphicalSimulation_triggered()
{
    bool visivible = true;

    if (!ui->actionActivateGraphicalSimulation->isChecked()) {
        AnimationTransition::setRunning(false);
        visivible = false;
    } else {
        AnimationTransition::setRunning(true);
    }

    // Esconde ou exibe animação de fila
    QList<QGraphicsItem *> *componentes = myScene()->getGraphicalModelComponents();

    for (QGraphicsItem* item : *componentes) {
        if (GraphicalModelComponent *component = dynamic_cast<GraphicalModelComponent *>(item)) {
            component->visivibleImageQueue(visivible);
        }
    }
}


void MainWindow::on_horizontalSliderAnimationSpeed_valueChanged(int value)
{
    double newValue = ((double) value) / 2;

    AnimationTransition::setTimeExecution(newValue);
}


void MainWindow::on_actionDiagrams_triggered()
{
    ModelGraphicsScene* scene = (ModelGraphicsScene*) (ui->graphicsView->scene());
    if (ui->actionDiagrams->isChecked()) {
        if (scene->existDiagram()) scene->showDiagrams();
    } else {
        if (scene->existDiagram()) scene->hideDiagrams();
    }
}


void MainWindow::on_actionSelectAll_triggered()
{
    QList<QGraphicsItem *> itensToScene = myScene()->items();

    foreach (QGraphicsItem* item, itensToScene) {
        item->setSelected(true);
    }
}

void MainWindow::on_actionParallelization_triggered()
{
    _showMessageNotImplemented();
}

void MainWindow::on_horizontalSlider_ZoomGraphical_actionTriggered(int action)
{

}

