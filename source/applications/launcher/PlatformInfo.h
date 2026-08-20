#pragma once

#include <QString>

namespace genesys::launcher {

struct PlatformInfo {
    QString osId;
    QString osVersion;
    QString architecture;
    QString manifestKey;
    bool valid = false;

    [[nodiscard]] static PlatformInfo detect(const QString& osReleasePath = QStringLiteral("/etc/os-release"),
                                             const QString& architectureOverride = {});
};

} // namespace genesys::launcher
