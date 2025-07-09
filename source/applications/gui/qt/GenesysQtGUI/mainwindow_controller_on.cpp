#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialogs/dialogBreakpoint.h"
#include "dialogs/Dialogmodelinformation.h"
#include "dialogs/dialogsimulationconfigure.h"
#include "dialogs/dialogpluginmanager.h"
#include "dialogs/dialogsystempreferences.h"
#include "dialogs/DialogFind.h"

#include "actions/DeleteUndoCommand.h"
#include "actions/PasteUndoCommand.h"

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
    simulator->getModelManager()->current()->getComponentManager()->getAllComponents()->clear();
    simulator->getModelManager()->current()->getComponentManager()->clear();
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
