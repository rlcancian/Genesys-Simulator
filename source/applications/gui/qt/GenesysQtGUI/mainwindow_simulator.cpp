#include "mainwindow.h"
#include "ui_mainwindow.h"
// Include dedicated Phase 4 controllers used by simulator compatibility wrappers.
#include "controllers/TraceConsoleController.h"
#include "controllers/SimulationEventController.h"
// Include the Phase 5 controller used by plugin-catalog compatibility wrappers.
#include "controllers/PluginCatalogController.h"


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
    // Keep this wrapper temporarily for compatibility during the incremental Phase 5 refactor.
    _pluginCatalogController->insertPluginUI(plugin);
}

void MainWindow::_insertFakePlugins() {
    // Keep this wrapper temporarily for compatibility during the incremental Phase 5 refactor.
    _pluginCatalogController->insertFakePlugins();
}
