#pragma once

#include <QString>
#include <QStringList>

class QWidget;

namespace ToolLauncher {

bool launchDetached(QWidget* parent,
                    const QString& executableBaseName,
                    const QString& displayName,
                    const QStringList& arguments = {},
                    const QStringList& relativeSearchDirs = {});

}  // namespace ToolLauncher
