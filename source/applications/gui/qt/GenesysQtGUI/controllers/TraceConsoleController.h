#ifndef TRACECONSOLECONTROLLER_H
#define TRACECONSOLECONTROLLER_H

#include "../../../../kernel/simulator/TraceManager.h"

class QTextEdit;

// Encapsulate Phase 4 trace rendering for console/simulation/report widgets.
class TraceConsoleController {
public:
    // Inject narrow text-output dependencies used by trace handlers.
    TraceConsoleController(QTextEdit* console,
                           QTextEdit* simulationText,
                           QTextEdit* reportsText);

    // Render generic simulator traces to the main console text area.
    void simulatorTraceHandler(TraceEvent e) const;
    // Render simulator error traces to the main console text area.
    void simulatorTraceErrorHandler(TraceErrorEvent e) const;
    // Render simulation traces to the simulation output text area.
    void simulatorTraceSimulationHandler(TraceSimulationEvent e) const;
    // Render reports traces to the reports output text area.
    void simulatorTraceReportsHandler(TraceEvent e) const;

private:
    // Keep the console trace sink used by normal/error traces.
    QTextEdit* _console;
    // Keep the simulation trace sink used by simulation traces.
    QTextEdit* _simulationText;
    // Keep the reports trace sink used by reports traces.
    QTextEdit* _reportsText;
};

#endif // TRACECONSOLECONTROLLER_H
