#ifndef TRACECONSOLECONTROLLER_H
#define TRACECONSOLECONTROLLER_H

#include "kernel/simulator/TraceManager.h"

class QTextEdit;

// Document the trace rendering bridge used by MainWindow simulator callbacks.
/**
 * @brief Controller that routes simulator trace events to GUI text widgets.
 *
 * This controller is the Phase-4 extraction for trace presentation. MainWindow keeps
 * compatibility handlers registered in the kernel trace manager and delegates rendering to
 * this class, reducing direct UI formatting logic in the façade.
 *
 * Responsibilities:
 * - render regular/error traces to the main console pane;
 * - render simulation traces to the simulation output pane;
 * - render report traces to the reports pane.
 *
 * Boundaries:
 * - it does not subscribe handlers by itself (registration stays in MainWindow/event layer);
 * - it does not change simulation flow, model state, or controller orchestration;
 * - it acts as a UI bridge, not as a domain service.
 */
class TraceConsoleController {
public:
    /** @brief Creates the trace-output bridge used by MainWindow trace wrappers. */
    TraceConsoleController(QTextEdit* console,
                           QTextEdit* simulationText,
                           QTextEdit* reportsText);

    /** @brief Renders generic traces to the main console pane. */
    void simulatorTraceHandler(TraceEvent e) const;
    /** @brief Renders error traces to the main console pane. */
    void simulatorTraceErrorHandler(TraceErrorEvent e) const;
    /** @brief Renders simulation traces to the simulation output pane. */
    void simulatorTraceSimulationHandler(TraceSimulationEvent e) const;
    /** @brief Renders reports traces to the reports output pane. */
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
