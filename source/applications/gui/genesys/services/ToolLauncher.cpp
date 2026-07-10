#include "ToolLauncher.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QMessageBox>
#include <QProcess>
#include <QWidget>

namespace {
QString executableSuffix() {
#if defined(Q_OS_WIN)
    return QStringLiteral(".exe");
#else
    return QString();
#endif
}

QStringList normalizedSearchDirs(const QStringList& relativeSearchDirs) {
    QStringList searchDirs;
    searchDirs << QCoreApplication::applicationDirPath();
    for (const QString& relativeDir : relativeSearchDirs) {
        if (relativeDir.isEmpty()) {
            continue;
        }
        searchDirs << QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(relativeDir);
    }
    return searchDirs;
}

QStringList candidateExecutables(const QString& executableBaseName, const QStringList& relativeSearchDirs) {
    QStringList candidates;
    const QString suffix = executableSuffix();
    const QStringList searchDirs = normalizedSearchDirs(relativeSearchDirs);
    for (const QString& searchDir : searchDirs) {
        const QString candidate = QDir(searchDir).absoluteFilePath(executableBaseName + suffix);
        if (!candidates.contains(candidate)) {
            candidates << candidate;
        }
    }
    return candidates;
}
}  // namespace

bool ToolLauncher::launchDetached(QWidget* parent,
                                  const QString& executableBaseName,
                                  const QString& displayName,
                                  const QStringList& arguments,
                                  const QStringList& relativeSearchDirs) {
    const QStringList candidates = candidateExecutables(executableBaseName, relativeSearchDirs);
    for (const QString& candidate : candidates) {
        if (!QFileInfo::exists(candidate)) {
            continue;
        }

        const bool started = QProcess::startDetached(candidate, arguments);
        if (started) {
            return true;
        }

        QMessageBox::critical(parent,
                              QObject::tr("Could not launch %1").arg(displayName),
                              QObject::tr("The executable was found, but Qt could not start it:\n%1").arg(candidate));
        return false;
    }

    QMessageBox::critical(
        parent,
        QObject::tr("Could not launch %1").arg(displayName),
        QObject::tr("Could not find the executable for %1.\n\nSearched:\n%2")
            .arg(displayName, candidates.join(QStringLiteral("\n"))));
    return false;
}
