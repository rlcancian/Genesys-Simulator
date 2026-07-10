#include "AIAssistantWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include "kernel/simulator/Simulator.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GenESyS"));
    QCoreApplication::setApplicationName(QStringLiteral("Genesys-AI-Assistant-GUI"));

    Simulator simulator;
    AIAssistantWindow window(&simulator, nullptr);
    window.show();
    return app.exec();
}
