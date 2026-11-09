#include "ToolLauncher.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QWidget>

namespace {
QString executableSuffix() {
#if defined(Q_OS_WIN)
    return QStringLiteral(".exe");
#else
    return QString();
#endif
}

bool validExecutableName(const QString& baseName) {
    // Never accept a path, shell command, or a non-GenESyS program.
    static const QRegularExpression namePattern(QStringLiteral(R"(^genesys-[a-z0-9][a-z0-9-]*$)"));
    return namePattern.match(baseName).hasMatch();
}

QStringList searchDirectories(const QString& applicationDir, const QStringList& relativeSearchDirs) {
    const QString appDir = QDir::cleanPath(QDir(applicationDir).absolutePath());
    QStringList directories{appDir};

    // CMake installs the main GUI under <prefix>/libexec/genesys/system/bin
    // and independently installed Tools GUIs under <prefix>/bin. Portable
    // bundles place all real GUI executables together in system/bin.
    const QString installedGuiSuffix = QStringLiteral("/libexec/genesys/system/bin");
    if (appDir.endsWith(installedGuiSuffix)) {
        QDir prefix(appDir);
        bool foundPrefix = true;
        for (int level = 0; level < 4; ++level) {
            foundPrefix = prefix.cdUp() && foundPrefix;
        }
        if (foundPrefix) {
            directories << QDir::cleanPath(prefix.filePath(QStringLiteral("bin")));
        }
        // Installed applications never consult build-tree fallback paths.
        // This also prevents a missing installed tool from resolving to an
        // unrelated executable elsewhere in the filesystem.
        directories.removeDuplicates();
        return directories;
    }

    // Development-only fallback directories are provided by the GUI handlers.
    // Never search the current working directory or PATH.
    const QDir appDirectory(appDir);
    for (const QString& relativeDir : relativeSearchDirs) {
        if (relativeDir.isEmpty() || QDir::isAbsolutePath(relativeDir)) {
            continue;
        }
        directories << QDir::cleanPath(appDirectory.absoluteFilePath(relativeDir));
    }
    directories.removeDuplicates();
    return directories;
}
}  // namespace

QStringList ToolLauncher::candidateExecutables(const QString& executableBaseName,
                                               const QString& applicationDir,
                                               const QStringList& relativeSearchDirs) {
    if (applicationDir.isEmpty() || !validExecutableName(executableBaseName)) {
        return {};
    }

    QStringList candidates;
    for (const QString& directory : searchDirectories(applicationDir, relativeSearchDirs)) {
        candidates << QDir(directory).absoluteFilePath(executableBaseName + executableSuffix());
    }
    return candidates;
}

QString ToolLauncher::findExecutable(const QString& executableBaseName,
                                     const QString& applicationDir,
                                     const QStringList& relativeSearchDirs) {
    for (const QString& candidate : candidateExecutables(executableBaseName, applicationDir, relativeSearchDirs)) {
        const QFileInfo file(candidate);
        if (file.isFile() && file.isExecutable()) {
            return candidate;
        }
    }
    return {};
}

bool ToolLauncher::launchDetached(QWidget* parent,
                                  const QString& executableBaseName,
                                  const QString& displayName,
                                  const QStringList& arguments,
                                  const QStringList& relativeSearchDirs) {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString executable = findExecutable(executableBaseName, appDir, relativeSearchDirs);
    if (!executable.isEmpty()) {
        if (QProcess::startDetached(executable, arguments)) {
            return true;
        }

        QMessageBox::critical(parent,
                              QObject::tr("Could not launch %1").arg(displayName),
                              QObject::tr("The executable was found, but Qt could not start it:\n%1").arg(executable));
        return false;
    }

    QMessageBox::critical(
        parent,
        QObject::tr("Could not launch %1").arg(displayName),
        QObject::tr("The executable for %1 is not installed or is not executable.\n\nSearched:\n%2")
            .arg(displayName,
                 candidateExecutables(executableBaseName, appDir, relativeSearchDirs)
                     .join(QStringLiteral("\n"))));
    return false;
}
