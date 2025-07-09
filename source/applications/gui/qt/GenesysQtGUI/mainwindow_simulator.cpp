#include "mainwindow.h"
#include "ui_mainwindow.h"


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
