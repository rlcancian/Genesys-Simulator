#include "mainwindow.h"
#include <QApplication>
//#include <QDesktopWidget> //removed from qt6
#include <QScreen>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QTextStream>
#include <csignal>
#include <exception>
#include <iostream>
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>

#include "../../../GenesysApplication_if.h"
#include "../../../TraitsApp.h"
#include "../../../terminal/TraitsTerminalApp.h"
#include "TraitsGUI.h"

namespace {
QMutex _logMutex;
QString _logPath;

QString _defaultLogPath() {
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (basePath.isEmpty()) {
        basePath = QDir::homePath() + "/.genesys";
    }
    QDir baseDir(basePath);
    baseDir.mkpath(".");
    return baseDir.absoluteFilePath("GenesysQtGUI.log");
}

void _appendLogLine(const QString& line) {
    QMutexLocker locker(&_logMutex);
    QFile file(_logPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << line.toStdString() << std::endl;
        return;
    }
    QTextStream stream(&file);
    stream << line << Qt::endl;
}

void _qtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    const QString now = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    QString level;
    switch (type) {
        case QtDebugMsg: level = "DEBUG"; break;
        case QtInfoMsg: level = "INFO"; break;
        case QtWarningMsg: level = "WARN"; break;
        case QtCriticalMsg: level = "CRITICAL"; break;
        case QtFatalMsg: level = "FATAL"; break;
    }
    QString where = QString("%1:%2").arg(context.file ? context.file : "unknown").arg(context.line);
    QString line = QString("[%1] [%2] [%3] %4").arg(now, level, where, msg);
    _appendLogLine(line);
    std::cerr << line.toStdString() << std::endl;
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

void _signalHandler(int signalNumber) {
    const QString header = QString("[%1] [FATAL] signal %2 received")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
            .arg(signalNumber);
    _appendLogLine(header);
    void *trace[64];
    int size = backtrace(trace, 64);
    int fd = ::open(_logPath.toStdString().c_str(), O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd >= 0) {
        backtrace_symbols_fd(trace, size, fd);
        ::close(fd);
    }
    std::_Exit(128 + signalNumber);
}

void _installCrashAndLogHandlers() {
    _logPath = _defaultLogPath();
    _appendLogLine(QString("[%1] [INFO] Logging started at %2")
                   .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), _logPath));
    qInstallMessageHandler(_qtMessageHandler);
    std::set_terminate(_terminateHandler);
    std::signal(SIGSEGV, _signalHandler);
    std::signal(SIGABRT, _signalHandler);
    std::signal(SIGILL, _signalHandler);
    std::signal(SIGFPE, _signalHandler);
}
}

int mainGraphicQtApp(int argc, char *argv[]) {
    _installCrashAndLogHandlers();
	QApplication a(argc, argv);
	MainWindow w;
    if constexpr (TraitsGUI<GMainWindow>::startMaximized) {
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
