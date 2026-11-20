#include "UpdateDialog.h"
#include "Version.h"

#include <QApplication>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);

    QTimer::singleShot(100, [] {
        for (QWidget* widget : QApplication::topLevelWidgets()) {
            if (auto* messageBox = qobject_cast<QMessageBox*>(widget)) {
                messageBox->reject();
            }
        }
    });

    genesys::launcher::UpdateDialog dialog;
    const bool accepted = dialog.confirmUpdate(
        genesys::launcher::Version::parse(QStringLiteral("2026.1.0")),
        genesys::launcher::Version::parse(QStringLiteral("2026.2.0")),
        QStringLiteral("Local fixture release notes; no network is used."));
    return accepted ? 1 : 0;
}
