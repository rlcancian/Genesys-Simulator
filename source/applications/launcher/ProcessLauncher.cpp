#include "ProcessLauncher.h"

#include "LauncherLogger.h"

#include <QProcess>

#include <cerrno>
#include <cstring>
#include <vector>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace genesys::launcher {
namespace {

#ifdef Q_OS_UNIX
int execPath(const QString& executable, const QStringList& arguments, int* errorNumber) {
    QByteArray executableBytes = QFile::encodeName(executable);
    std::vector<QByteArray> encoded;
    encoded.reserve(static_cast<std::size_t>(arguments.size()) + 1);
    encoded.push_back(executableBytes);
    for (const QString& argument : arguments) {
        encoded.push_back(argument.toLocal8Bit());
    }

    std::vector<char*> argv;
    argv.reserve(encoded.size() + 1);
    for (QByteArray& value : encoded) {
        argv.push_back(value.data());
    }
    argv.push_back(nullptr);

    ::execv(executableBytes.constData(), argv.data());
    *errorNumber = errno;
    return -1;
}
#endif

} // namespace

ProcessLauncher::ProcessLauncher(LauncherLogger* logger)
    : logger_(logger) {}

int ProcessLauncher::launchReplacingProcess(const RuntimeSelection& selection,
                                            const QStringList& arguments,
                                            QString* error) const {
    if (!selection.ok || selection.primaryExecutable.isEmpty()) {
        if (error) {
            *error = selection.reason.isEmpty() ? QStringLiteral("No executable selected") : selection.reason;
        }
        return 127;
    }

#ifdef Q_OS_UNIX
    int errorNumber = 0;
    execPath(selection.primaryExecutable, arguments, &errorNumber);
    if (logger_) {
        logger_->log(QStringLiteral("exec-failed"),
                     QStringLiteral("path=%1 errno=%2 error=%3")
                         .arg(selection.primaryExecutable)
                         .arg(errorNumber)
                         .arg(QString::fromLocal8Bit(std::strerror(errorNumber))));
    }

    if (!selection.fallbackExecutable.isEmpty() && selection.fallbackExecutable != selection.primaryExecutable) {
        if (logger_) {
            logger_->log(QStringLiteral("exec-fallback"),
                         QStringLiteral("primary=%1 fallback=%2 errno=%3")
                             .arg(selection.primaryExecutable, selection.fallbackExecutable)
                             .arg(errorNumber));
        }
        int fallbackError = 0;
        execPath(selection.fallbackExecutable, arguments, &fallbackError);
        errorNumber = fallbackError;
    }

    if (error) {
        *error = QStringLiteral("Unable to execute application: %1").arg(QString::fromLocal8Bit(std::strerror(errorNumber)));
    }
    return errorNumber == EACCES ? 126 : 127;
#else
    QProcess process;
    process.setProgram(selection.primaryExecutable);
    process.setArguments(arguments);
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start();
    if (!process.waitForStarted()) {
        if (!selection.fallbackExecutable.isEmpty()) {
            process.setProgram(selection.fallbackExecutable);
            process.start();
            if (!process.waitForStarted()) {
                if (error) {
                    *error = process.errorString();
                }
                return 127;
            }
        } else {
            if (error) {
                *error = process.errorString();
            }
            return 127;
        }
    }
    process.waitForFinished(-1);
    return process.exitCode();
#endif
}

} // namespace genesys::launcher
