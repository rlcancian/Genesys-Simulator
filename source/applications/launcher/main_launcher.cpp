#include "ArchiveExtractor.h"
#include "LauncherConfig.h"
#include "LauncherController.h"
#include "LauncherLogger.h"
#include "NetworkTransport.h"
#include "PlatformInfo.h"
#include "ProcessLauncher.h"
#include "RuntimePaths.h"
#include "RuntimeSelector.h"
#include "UpdateChecker.h"
#include "UpdateDialog.h"
#include "UpdateDownloader.h"
#include "UpdateVerifier.h"
#include "UserRuntimeManager.h"
#include "Version.h"

#include <QApplication>
#include <QTextStream>

#ifndef GENESYS_LAUNCHER_VERSION
#define GENESYS_LAUNCHER_VERSION "0.0.0"
#endif

#ifndef GENESYS_SYSTEM_PACKAGE_VERSION
#define GENESYS_SYSTEM_PACKAGE_VERSION "0.0.0"
#endif

namespace {

struct CommandLine {
    QString application = QStringLiteral("genesys-gui");
    QStringList forwarded;
    bool explicitInteractiveUpdate = false;
    bool noUpdate = false;
    bool checkOnly = false;
    bool help = false;
    bool version = false;
    QString error;
};

CommandLine parseCommandLine(const QStringList& arguments) {
    CommandLine result;
    bool passthrough = false;
    for (qsizetype i = 1; i < arguments.size(); ++i) {
        const QString& argument = arguments.at(i);
        if (passthrough) {
            result.forwarded.push_back(argument);
            continue;
        }
        if (argument == QStringLiteral("--")) {
            passthrough = true;
        } else if (argument == QStringLiteral("--app")) {
            if (i + 1 >= arguments.size()) {
                result.error = QStringLiteral("--app requires an application name");
                return result;
            }
            result.application = arguments.at(++i);
        } else if (argument == QStringLiteral("--interactive-update")) {
            result.explicitInteractiveUpdate = true;
        } else if (argument == QStringLiteral("--no-update")) {
            result.noUpdate = true;
        } else if (argument == QStringLiteral("--check-only")) {
            result.checkOnly = true;
        } else if (argument == QStringLiteral("--help") || argument == QStringLiteral("-h")) {
            result.help = true;
        } else if (argument == QStringLiteral("--version")) {
            result.version = true;
        } else {
            result.forwarded.push_back(argument);
        }
    }
    if (result.noUpdate && result.checkOnly) {
        result.error = QStringLiteral("--no-update and --check-only cannot be used together");
    }
    if ((result.explicitInteractiveUpdate || result.checkOnly) && result.application != QStringLiteral("genesys-gui")) {
        result.error = QStringLiteral("Remote update checks are permitted only for genesys-gui");
    }
    return result;
}

class ConsoleUpdateUi final : public genesys::launcher::IUpdateUi {
public:
    bool confirmUpdate(const genesys::launcher::Version&,
                       const genesys::launcher::Version&,
                       const QString&) override {
        return false;
    }
    void beginDownload(qint64) override {}
    bool updateDownloadProgress(qint64, qint64) override { return true; }
    void finishDownload() override {}
    void showError(const QString& message) override { QTextStream(stderr) << message << '\n'; }
    void showInfo(const QString& message) override { QTextStream(stdout) << message << '\n'; }
};

} // namespace

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("genesys-launcher"));

    QTextStream out(stdout);
    QTextStream err(stderr);
    const CommandLine command = parseCommandLine(application.arguments());
    if (!command.error.isEmpty()) {
        err << "genesys-launcher: " << command.error << '\n';
        return 2;
    }
    if (command.help) {
        out << "Usage: genesys-launcher [--app genesys-gui] [--interactive-update|--check-only|--no-update] -- [arguments...]\n";
        return 0;
    }
    if (command.version) {
        out << "genesys-launcher " << GENESYS_LAUNCHER_VERSION << '\n';
        return 0;
    }

    const genesys::launcher::RuntimePaths paths;
    const genesys::launcher::LauncherConfig config = genesys::launcher::LauncherConfig::load(
        paths.systemConfigPath(), paths.userConfigPath());
    genesys::launcher::LauncherLogger logger(paths.launcherLogPath());
    logger.log(QStringLiteral("launcher-request"), QStringLiteral("app=%1").arg(command.application));
    for (const QString& warning : config.warnings) {
        logger.log(QStringLiteral("config-warning"), warning);
    }

    genesys::launcher::RuntimeSelector selector(paths, &logger);
    const genesys::launcher::Version userVersion = selector.currentUserVersion();
    const genesys::launcher::Version systemVersion = genesys::launcher::Version::parse(QStringLiteral(GENESYS_SYSTEM_PACKAGE_VERSION));
    const genesys::launcher::Version currentVersion = userVersion.isValid() ? userVersion : systemVersion;
    const genesys::launcher::Version launcherVersion = genesys::launcher::Version::parse(QStringLiteral(GENESYS_LAUNCHER_VERSION));

    const bool automaticInteractiveCheck = command.application == QStringLiteral("genesys-gui") &&
                                           config.checkOnGuiStartup && !command.noUpdate && !command.checkOnly;
    const bool shouldCheck = command.application == QStringLiteral("genesys-gui") && !command.noUpdate &&
                             (command.checkOnly || command.explicitInteractiveUpdate || automaticInteractiveCheck);

    if (shouldCheck) {
        genesys::launcher::QtNetworkTransport transport;
        const genesys::launcher::PlatformInfo platform = genesys::launcher::PlatformInfo::detect();
        genesys::launcher::UpdateChecker checker(paths,
                                                  config,
                                                  platform,
                                                  currentVersion,
                                                  launcherVersion,
                                                  systemVersion,
                                                  transport,
                                                  &logger);
        genesys::launcher::UpdateDownloader downloader(paths, transport, &logger);
        genesys::launcher::GpgvSignatureVerifier signatureVerifier(paths.updateKeyringPath(), &logger);
        genesys::launcher::TarZstdArchiveExtractor extractor;
        genesys::launcher::UserRuntimeManager runtimeManager(paths, extractor, &logger);
        genesys::launcher::UpdateDialog dialog;
        ConsoleUpdateUi consoleUi;
        genesys::launcher::IUpdateUi& updateUi = command.checkOnly
            ? static_cast<genesys::launcher::IUpdateUi&>(consoleUi)
            : static_cast<genesys::launcher::IUpdateUi&>(dialog);
        genesys::launcher::LauncherController controller(config,
                                                         checker,
                                                         downloader,
                                                         config.requireSignature ? &signatureVerifier : nullptr,
                                                         runtimeManager,
                                                         updateUi,
                                                         &logger);
        const bool interactive = !command.checkOnly;
        const genesys::launcher::LauncherUpdateResult updateResult = controller.checkAndMaybeUpdate(
            currentVersion,
            interactive,
            command.checkOnly || command.explicitInteractiveUpdate,
            command.checkOnly);
        if (command.checkOnly) {
            return updateResult.checkCompleted ? 0 : 3;
        }
    }

    const genesys::launcher::RuntimeSelection selection = selector.select(command.application, config);
    if (!selection.ok) {
        err << "genesys-launcher: " << selection.reason << '\n';
        return 127;
    }

    genesys::launcher::ProcessLauncher processLauncher(&logger);
    QString launchError;
    const int exitCode = processLauncher.launchReplacingProcess(selection, command.forwarded, &launchError);
    if (exitCode != 0 && !launchError.isEmpty()) {
        err << "genesys-launcher: " << launchError << '\n';
    }
    return exitCode;
}
