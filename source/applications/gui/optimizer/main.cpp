#include "tools/optimizer/OptimizerWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include "kernel/simulator/Simulator.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GenESyS"));
    QCoreApplication::setApplicationName(QStringLiteral("Genesys-Optimizer-GUI"));

    Simulator simulator;
    OptimizerWindow window(&simulator, nullptr);
    window.show();
    return app.exec();
}
