#pragma once

#include <QUrl>
#include <QString>
#include <QStringList>

namespace genesys::launcher {

struct LauncherConfig {
    bool updatesEnabled = false;
    QString channel = QStringLiteral("stable");
    bool allowUserRuntime = true;
    bool requireSignature = true;
    bool checkOnGuiStartup = true;
    bool fallbackToSystem = true;
    int minimumCheckIntervalHours = 12;
    int maxUserVersions = 2;
    QUrl manifestUrl;
    bool hasExplicitPolicy = false;
    QStringList warnings;

    [[nodiscard]] static LauncherConfig load(const QString& systemConfigPath,
                                             const QString& userConfigPath);
};

} // namespace genesys::launcher
