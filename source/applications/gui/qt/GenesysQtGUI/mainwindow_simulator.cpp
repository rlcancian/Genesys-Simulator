#include "mainwindow.h"
#include "ui_mainwindow.h"
// Include dedicated Phase 4 controllers used by simulator compatibility wrappers.
#include "controllers/TraceConsoleController.h"
#include "controllers/SimulationEventController.h"


//-------------------------
// Simulator Trace Handlers
//-------------------------

void MainWindow::_simulatorTraceHandler(TraceEvent e) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _traceConsoleController->simulatorTraceHandler(e);
}

void MainWindow::_simulatorTraceErrorHandler(TraceErrorEvent e) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _traceConsoleController->simulatorTraceErrorHandler(e);
}

void MainWindow::_simulatorTraceSimulationHandler(TraceSimulationEvent e) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _traceConsoleController->simulatorTraceSimulationHandler(e);
}

void MainWindow::_simulatorTraceReportsHandler(TraceEvent e) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _traceConsoleController->simulatorTraceReportsHandler(e);
}

//
// simulator event handlers
//

void MainWindow::_onModelCheckSuccessHandler(ModelEvent* re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onModelCheckSuccessHandler(re);
}

void MainWindow::_onReplicationStartHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onReplicationStartHandler(re);
}

void MainWindow::_onSimulationStartHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onSimulationStartHandler(re);
}

void MainWindow::_onSimulationPausedHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onSimulationPausedHandler(re);
}

void MainWindow::_onSimulationResumeHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onSimulationResumeHandler(re);
}

void MainWindow::_onSimulationEndHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onSimulationEndHandler(re);
}

void MainWindow::_onProcessEventHandler(SimulationEvent * re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onProcessEventHandler(re);
}

void MainWindow::_onEntityCreateHandler(SimulationEvent* re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onEntityCreateHandler(re);
}

void MainWindow::_onEntityRemoveHandler(SimulationEvent* re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onEntityRemoveHandler(re);
}

void MainWindow::_onMoveEntityEvent(SimulationEvent *re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onMoveEntityEvent(re);
}

void MainWindow::_onAfterProcessEvent(SimulationEvent *re) {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->onAfterProcessEvent(re);
}


void MainWindow::_setOnEventHandlers() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 4 refactor.
    _simulationEventController->setOnEventHandlers(this);
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
