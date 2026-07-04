#include "WebWorkerDialog.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("GenESyS"));
    QCoreApplication::setApplicationName(QStringLiteral("Genesys-WebWorker-GUI"));

    WebWorkerDialog dialog(nullptr, true);
    dialog.show();
    return app.exec();
}
