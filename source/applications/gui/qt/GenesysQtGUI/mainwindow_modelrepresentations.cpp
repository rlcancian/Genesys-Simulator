#include "mainwindow.h"
#include "ui_mainwindow.h"
// Include the Phase 3 controller interface required by compatibility wrappers.
#include "controllers/ModelInspectorController.h"
#include "services/ModelLanguageSynchronizer.h"
#include "services/GraphvizModelExporter.h"
#include "services/CppModelExporter.h"
#include "services/GraphicalModelSerializer.h"
#include "services/GraphicalModelBuilder.h"

// Kernel
#include "../../../../kernel/simulator/SinkModelComponent.h"

#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
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
#include <QScrollBar>
#include <QTimer>
#include <QUrl>

void MainWindow::_actualizeModelSimLanguage() {
    // This wrapper delegates model-language synchronization to a dedicated phase-1 service.
    _modelLanguageSynchronizer->actualizeModelSimLanguage();
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
    // This wrapper delegates text-to-model synchronization while keeping MainWindow as temporary API surface.
    return _modelLanguageSynchronizer->setSimulationModelBasedOnText();
}

std::string MainWindow::_adjustDotName(std::string name) {
    // This wrapper delegates DOT-name normalization to the phase-1 Graphviz service.
    return _graphvizModelExporter->adjustDotName(std::move(name));
}

void MainWindow::_insertTextInDot(std::string text, unsigned int compLevel, unsigned int compRank, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap, bool isNode) {
    // This wrapper delegates ranked DOT insertion to the phase-1 Graphviz service.
    _graphvizModelExporter->insertTextInDot(std::move(text), compLevel, compRank, dotmap, isNode);
}

void MainWindow::_recursiveCreateModelGraphicPicture(ModelDataDefinition* componentOrData, std::list<ModelDataDefinition*>* visited, std::map<unsigned int, std::map<unsigned int, std::list<std::string>*>*>* dotmap) {
    // This wrapper delegates recursive DOT generation to the phase-1 Graphviz service.
    _graphvizModelExporter->recursiveCreateModelGraphicPicture(componentOrData, visited, dotmap);
}

std::string MainWindow::_addCppCodeLine(std::string line, unsigned int indent) {
    // This wrapper delegates C++ line formatting to the phase-1 exporter service.
    return _cppModelExporter->addCppCodeLine(line, indent);
}

void MainWindow::_actualizeModelCppCode() {
    // This wrapper delegates full C++ code export rendering to the phase-1 exporter service.
    _cppModelExporter->actualizeModelCppCode();
}

bool MainWindow::graphicalModelHasChanged() const {
    return _graphicalModelHasChanged;
}

void MainWindow::setGraphicalModelHasChanged(bool graphicalModelHasChanged) {
    _graphicalModelHasChanged = graphicalModelHasChanged;
    _actualizeTabPanes();
}

bool MainWindow::_createModelImage() {
    // This wrapper delegates model diagram image creation to the phase-1 Graphviz service.
    return _graphvizModelExporter->createModelImage();
}

//-----------------------------------------------------------------

bool MainWindow::_saveTextModel(QFile *saveFile, QString data)
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 2 refactor.
    return _graphicalModelSerializer->saveTextModel(saveFile, data);
}

bool MainWindow::_saveGraphicalModel(QString filename)
{
    // Keep this wrapper temporarily for compatibility during the incremental Phase 2 refactor.
    return _graphicalModelSerializer->saveGraphicalModel(filename);
}

Model *MainWindow::_loadGraphicalModel(std::string filename) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 2 refactor.
    return _graphicalModelSerializer->loadGraphicalModel(filename);
}


void MainWindow::_recursivalyGenerateGraphicalModelFromModel(ModelComponent* component, List<ModelComponent*>* visited, std::map<ModelComponent*,GraphicalModelComponent*>* map, int *x, int *y, int *ymax, int sequenceInLine) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 2 refactor.
    _graphicalModelBuilder->recursivalyGenerateGraphicalModelFromModel(component, visited, map, x, y, ymax, sequenceInLine);
}

void MainWindow::_actualizeModelComponents(bool force) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 3 refactor.
    _modelInspectorController->actualizeModelComponents(force);
}

void MainWindow::_actualizeModelTextHasChanged(bool hasChanged) {
    if (_textModelHasChanged != hasChanged) {
    }
    _textModelHasChanged = hasChanged;
}

void MainWindow::_actualizeModelDataDefinitions(bool force) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 3 refactor.
    _modelInspectorController->actualizeModelDataDefinitions(force);
}

void MainWindow::_actualizeGraphicalModel(SimulationEvent * re) {
    Event* event = re->getCurrentEvent();
    if (event != nullptr) {
        ui->graphicsView->selectModelComponent(event->getComponent());
    }
}

void MainWindow::_generateGraphicalModelFromModel() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 2 refactor.
    _graphicalModelBuilder->generateGraphicalModelFromModel();
}


//-----------------------------------------

void MainWindow::_initModelGraphicsView() {
    // Registers scene/view callbacks used by MainWindow controllers.
    ((ModelGraphicsView*) (ui->graphicsView))->setSceneMouseEventHandler(this, &MainWindow::_onSceneMouseEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelInEventHandler(this, &MainWindow::_onSceneWheelInEvent);
    ((ModelGraphicsView *)(ui->graphicsView))->setSceneWheelOutEventHandler(this, &MainWindow::_onSceneWheelOutEvent);
    ((ModelGraphicsView*) (ui->graphicsView))->setGraphicalModelEventHandler(this, &MainWindow::_onSceneGraphicalModelEvent);
    _connectSceneSignals();

    // Applies persisted overlay states to the graphics view when initializing a scene.
    ui->graphicsView->setRuleVisible(ui->actionShowRule->isChecked());
    ui->graphicsView->setGuidesVisible(ui->actionShowGuides->isChecked());

    // Cria uma stack undo/redo
    ui->graphicsView->getScene()->setUndoStack(new QUndoStack(this));
}
