#pragma once

#include <QString>
#include <QStringList>

class QWidget;

namespace ToolLauncher {

// Deterministic, PATH-independent lookup. applicationDir is explicit so supported
// build and installed layouts can be verified without launching a GUI.
QStringList candidateExecutables(const QString& executableBaseName,
                                 const QString& applicationDir,
                                 const QStringList& relativeSearchDirs = {});

QString findExecutable(const QString& executableBaseName,
                       const QString& applicationDir,
                       const QStringList& relativeSearchDirs = {});

bool launchDetached(QWidget* parent,
                    const QString& executableBaseName,
                    const QString& displayName,
                    const QStringList& arguments = {},
                    const QStringList& relativeSearchDirs = {});

}  // namespace ToolLauncher
