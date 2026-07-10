// Document this compilation unit as the simulator-callback partition of MainWindow.
/**
 * @file mainwindow_simulator.cpp
 * @brief Simulator trace/event wrapper partition of MainWindow implementation.
 *
 * This file centralizes MainWindow compatibility wrappers for simulator traces and simulation
 * event callbacks, delegating rendering and reactions to TraceConsoleController and
 * SimulationEventController (plus plugin-catalog related delegation where applicable). It does
 * not define a new class; it is a thematic partition of MainWindow.
 */
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
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_traceConsoleController != nullptr) {
        _traceConsoleController->simulatorTraceHandler(e);
    }
}

void MainWindow::_simulatorTraceErrorHandler(TraceErrorEvent e) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_traceConsoleController != nullptr) {
        _traceConsoleController->simulatorTraceErrorHandler(e);
    }
}

void MainWindow::_simulatorTraceSimulationHandler(TraceSimulationEvent e) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_traceConsoleController != nullptr) {
        _traceConsoleController->simulatorTraceSimulationHandler(e);
    }
}

void MainWindow::_simulatorTraceReportsHandler(TraceEvent e) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_traceConsoleController != nullptr) {
        _traceConsoleController->simulatorTraceReportsHandler(e);
    }
}

//
// simulator event handlers
//

void MainWindow::_onModelCheckSuccessHandler(ModelEvent* re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onModelCheckSuccessHandler(re);
    }
}

void MainWindow::_onReplicationStartHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onReplicationStartHandler(re);
    }
}

void MainWindow::_onReplicationEndHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onReplicationEndHandler(re);
    }
    _showReplicationReportIfSuppressed();
}

void MainWindow::_onSimulationStartHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onSimulationStartHandler(re);
    }
}

void MainWindow::_onSimulationPausedHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onSimulationPausedHandler(re);
    }
}

void MainWindow::_onSimulationResumeHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onSimulationResumeHandler(re);
    }
}

void MainWindow::_onSimulationEndHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onSimulationEndHandler(re);
    }
    _actualizeReportsResultsTable();
    _actualizeReportsPlots();
    _showSimulationReportIfSuppressed();
}

void MainWindow::_onProcessEventHandler(SimulationEvent * re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onProcessEventHandler(re);
    }
}

void MainWindow::_onEntityCreateHandler(SimulationEvent* re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onEntityCreateHandler(re);
    }
}

void MainWindow::_onEntityRemoveHandler(SimulationEvent* re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onEntityRemoveHandler(re);
    }
}

void MainWindow::_onMoveEntityEvent(SimulationEvent *re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onMoveEntityEvent(re);
    }
}

void MainWindow::_onAfterProcessEvent(SimulationEvent *re) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->onAfterProcessEvent(re);
    }
}


void MainWindow::_setOnEventHandlers() {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_simulationEventController != nullptr) {
        _simulationEventController->setOnEventHandlers(this);
    }
}

void MainWindow::_showReplicationReportIfSuppressed() {
    if (simulator == nullptr || simulator->getModelManager() == nullptr || simulator->getModelManager()->current() == nullptr) {
        return;
    }

    ModelSimulation* simulation = simulator->getModelManager()->current()->getSimulation();
    if (simulation == nullptr || simulation->isShowReportsAfterReplication() || simulation->getReporter() == nullptr) {
        return;
    }

    simulation->getReporter()->showReplicationStatistics();
}

void MainWindow::_showSimulationReportIfSuppressed() {
    if (simulator == nullptr || simulator->getModelManager() == nullptr || simulator->getModelManager()->current() == nullptr) {
        return;
    }

    ModelSimulation* simulation = simulator->getModelManager()->current()->getSimulation();
    if (simulation == nullptr || simulation->isShowReportsAfterSimulation() || simulation->getReporter() == nullptr) {
        return;
    }

    simulation->getReporter()->showSimulationStatistics();
}


//-------------------------
// Simulator Fake Plugins
//-------------------------

void MainWindow::_insertPluginUI(Plugin * plugin) {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_pluginCatalogController != nullptr) {
        _pluginCatalogController->insertPluginUI(plugin);
    }
}

void MainWindow::_insertFakePlugins() {
    // Keep this compatibility wrapper in the final façade and guard shutdown-time calls.
    if (_pluginCatalogController != nullptr) {
        _pluginCatalogController->insertFakePlugins();
    }
}
