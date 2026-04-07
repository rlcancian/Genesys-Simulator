#include "TraceConsoleController.h"

#include <QColor>
#include <QCoreApplication>
#include <QTextCursor>
#include <QTextEdit>

#include <iostream>
#include <string>

// Build the trace controller with narrow widget dependencies.
TraceConsoleController::TraceConsoleController(QTextEdit* console,
                                               QTextEdit* simulationText,
                                               QTextEdit* reportsText)
    : _console(console),
      _simulationText(simulationText),
      _reportsText(reportsText) {
}

// Preserve legacy console trace routing, coloring and UI refresh behavior.
void TraceConsoleController::simulatorTraceHandler(TraceEvent e) const {
    std::cout << e.getText() << std::endl;
    if (e.getTracelevel() == TraceManager::Level::L1_errorFatal) {
        _console->setTextColor(QColor::fromRgb(255, 0, 0));
    } else if (e.getTracelevel() == TraceManager::Level::L2_results) {
        _console->setTextColor(QColor::fromRgb(0, 0, 255));
    } else if (e.getTracelevel() == TraceManager::Level::L3_errorRecover) {
        _console->setTextColor(QColor::fromRgb(223, 0, 0));
    } else if (e.getTracelevel() == TraceManager::Level::L4_warning) {
        _console->setTextColor(QColor::fromRgb(128, 0, 0));
    } else {
        unsigned short grayVal = 20 * (static_cast<unsigned int>(e.getTracelevel()) - 5);
        _console->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    _console->append(QString::fromStdString(e.getText()));
    _console->moveCursor(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::MoveAnchor);
    QCoreApplication::processEvents();
}

// Preserve legacy error-trace rendering in the console output.
void TraceConsoleController::simulatorTraceErrorHandler(TraceErrorEvent e) const {
    std::cout << e.getText() << std::endl;
    _console->setTextColor(QColor::fromRgb(255, 0, 0));
    _console->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

// Preserve legacy simulation-trace rendering and coloring behavior.
void TraceConsoleController::simulatorTraceSimulationHandler(TraceSimulationEvent e) const {
    std::cout << e.getText() << std::endl;
    if (e.getText().find("Event {time=") != std::string::npos) {
        _simulationText->setTextColor(QColor::fromRgb(0, 0, 128));
    } else {
        unsigned short grayVal = 20 * (static_cast<unsigned int>(e.getTracelevel()) - 5);
        _simulationText->setTextColor(QColor::fromRgb(grayVal, grayVal, grayVal));
    }
    _simulationText->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}

// Preserve legacy reports-trace routing and UI refresh behavior.
void TraceConsoleController::simulatorTraceReportsHandler(TraceEvent e) const {
    std::cout << e.getText() << std::endl;
    _reportsText->append(QString::fromStdString(e.getText()));
    QCoreApplication::processEvents();
}
