#include "DispatchController.h"
#include "LauncherConfig.h"
#include "LauncherLogger.h"
#include "ProcessLauncher.h"
#include "RuntimePaths.h"
#include "RuntimeSelector.h"

#include <QCoreApplication>
#include <QTextStream>

#ifndef GENESYS_LAUNCHER_VERSION
#define GENESYS_LAUNCHER_VERSION "0.0.0"
#endif

namespace {

struct CommandLine {
    QString application;
    QStringList forwarded;
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
            continue;
        }
        if (argument == QStringLiteral("--app")) {
            if (i + 1 >= arguments.size() || !result.application.isEmpty()) {
                result.error = QStringLiteral("--app requires exactly one application name");
                return result;
            }
            result.application = arguments.at(++i);
            continue;
        }
        if (argument == QStringLiteral("--help") || argument == QStringLiteral("-h")) {
            result.help = true;
            continue;
        }
        if (argument == QStringLiteral("--version")) {
            result.version = true;
            continue;
        }
        result.forwarded.push_back(argument);
    }
    if (!result.help && !result.version && result.application.isEmpty()) {
        result.error = QStringLiteral("Missing required --app <application>");
    }
    return result;
}

} // namespace

int main(int argc, char* argv[]) {
    QCoreApplication application(argc, argv);
    application.setApplicationName(QStringLiteral("genesys-dispatch"));

    QTextStream out(stdout);
    QTextStream err(stderr);
    const CommandLine command = parseCommandLine(application.arguments());
    if (!command.error.isEmpty()) {
        err << "genesys-dispatch: " << command.error << '\n';
        return 2;
    }
    if (command.help) {
        out << "Usage: genesys-dispatch --app <application> -- [arguments...]\n";
        return 0;
    }
    if (command.version) {
        out << "genesys-dispatch " << GENESYS_LAUNCHER_VERSION << '\n';
        return 0;
    }

    const genesys::launcher::RuntimePaths paths;
    const genesys::launcher::LauncherConfig config = genesys::launcher::LauncherConfig::load(
        paths.systemConfigPath(), paths.userConfigPath());
    genesys::launcher::LauncherLogger logger(paths.launcherLogPath());
    logger.log(QStringLiteral("dispatch-request"), QStringLiteral("app=%1").arg(command.application));

    const genesys::launcher::RuntimeSelector selector(paths, &logger);
    genesys::launcher::ProcessLauncher processLauncher(&logger);
    const genesys::launcher::DispatchController controller(selector, processLauncher);

    QString launchError;
    const int exitCode = controller.dispatch(command.application, command.forwarded, config, &launchError);
    if (exitCode != 0 && !launchError.isEmpty()) {
        if (command.application == QStringLiteral("genesys-mcp") && exitCode == 127) {
            err << "genesys-mcp is not available in the active GenESyS runtime or in the system installation.\n"
                   "Open genesys-gui to check for an updated runtime, or install a GenESyS distribution that includes MCP.\n";
        } else {
            err << "genesys-dispatch: " << launchError << '\n';
        }
    }
    return exitCode;
}
