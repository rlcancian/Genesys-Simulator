#include "mainwindow.h"
#include <QApplication>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>
#include <exception>
#include <cstdlib>
#include <execinfo.h>

#include "../../../GenesysApplication_if.h"
#include "../../../TraitsApp.h"
#include "../../../terminal/TraitsTerminalApp.h"
#include "GuiCrashDiagnostics.h"
#include "guithememanager.h"
#include "systempreferences.h"

namespace {
QMutex _logMutex;
QString _logPath;

QString _defaultLogPath() {
    QString basePath = QDir::currentPath();
    QDir baseDir(basePath);
    baseDir.mkpath(".");
    return baseDir.absoluteFilePath("crashesAndLogs.log");
}

void _appendLogLine(const QString& line) {
    QMutexLocker locker(&_logMutex);
    QFile file(_logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream stream(&file);
    stream << line << Qt::endl;
}

void _qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    const QString line = qFormatLogMessage(type, context, msg);
    _appendLogLine(line);
    if (type == QtFatalMsg) {
        abort();
    }
}

void _terminateHandler() {
    _appendLogLine(QString("[%1] [FATAL] std::terminate invoked").arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)));
    void *trace[64];
    int size = backtrace(trace, 64);
    char **messages = backtrace_symbols(trace, size);
    if (messages != nullptr) {
        for (int i = 0; i < size; ++i) {
            _appendLogLine(QString("  at %1").arg(messages[i]));
        }
        free(messages);
    }
    std::_Exit(EXIT_FAILURE);
}

void _installCrashAndLogHandlers() {
    // Installs temporary fatal-signal crash diagnostics before GUI startup.
    GuiCrashDiagnostics::installFatalSignalHandlers();
    qSetMessagePattern("[%{time yyyy-MM-ddTHH:mm:ss.zzz}] [%{if-debug}DEBUG%{endif}%{if-info}INFO%{endif}%{if-warning}WARN%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}] [tid:%{threadid}] [%{file}:%{line}] [%{function}] %{message}");
    _logPath = _defaultLogPath();
    _appendLogLine(QString("[%1] [INFO] Logging started at %2")
                   .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), _logPath));
    qInstallMessageHandler(_qtMessageHandler);
    std::set_terminate(_terminateHandler);
}
}

int mainGraphicQtApp(int argc, char *argv[]) {
    _installCrashAndLogHandlers();
	QApplication a(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GenESyS"));
    QCoreApplication::setApplicationName(QStringLiteral("Genesys-Simulator"));
    SystemPreferences::load();
    GuiThemeManager::applyApplicationTheme(&a);
	MainWindow w;
    if (SystemPreferences::startMaximized()) {
        w.setWindowState(Qt::WindowMaximized);
        QRect screenGeometry = QApplication::primaryScreen()->availableGeometry(); //QApplication::desktop()->availableGeometry();
        w.resize(screenGeometry.width(), screenGeometry.height());
        //w.showNormal();
        w.showMaximized();
    } else {
        w.show();
    }
	return a.exec();
}

int mainTerminalApp(int argc, char *argv[]) {
	GenesysApplication_if *app = new TraitsTerminalApp<GenesysApplication_if>::Application();
    return app->main(argc, argv);
}


// Função auxiliar genérica
template <bool runGUI>
int runApp(int argc, char** argv);

// Especialização para true
template <>
int runApp<true>(int argc, char** argv) {
    return mainGraphicQtApp(argc, argv);
}

// Especialização para false
template <>
int runApp<false>(int argc, char** argv) {
    return mainTerminalApp(argc, argv);
}

/**
 *  THIS IS THE GENESYS MAIN FUNCTION
 */
int main(int argc, char** argv) {
    return runApp<TraitsApp<GenesysApplication_if>::runGraphicalUserInterface>(argc, argv);
}


/**
 *  THIS IS THE GENESYS MAIN FUNCTION

int main(int argc, char *argv[]) {
    if constexpr (TraitsApp<GenesysApplication_if>::runGraphicalUserInterface) {
		return mainGraphicQtApp(argc, argv);
	} else {
		return mainTerminalApp(argc, argv);
	}
}
*/
