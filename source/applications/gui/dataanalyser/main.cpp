#include "tools/dataanalyzer/DataAnalyzerWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include "kernel/simulator/Simulator.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GenESyS"));
    QCoreApplication::setApplicationName(QStringLiteral("Genesys-DataAnalyzer-GUI"));

    Simulator simulator;
    QString lastDataAnalyzerPath;
    DataAnalyzerWindow window(&simulator, lastDataAnalyzerPath, nullptr);
    window.show();
    return app.exec();
}
