#include "RuntimePaths.h"

#include <QDir>
#include <QFileInfo>

#ifndef GENESYS_SYSTEM_RUNTIME_ROOT_DEFAULT
#define GENESYS_SYSTEM_RUNTIME_ROOT_DEFAULT "/usr/libexec/genesys/system"
#endif

#ifndef GENESYS_SYSTEM_CONFIG_PATH_DEFAULT
#define GENESYS_SYSTEM_CONFIG_PATH_DEFAULT "/etc/genesys/update.conf"
#endif

#ifndef GENESYS_UPDATE_KEYRING_PATH_DEFAULT
#define GENESYS_UPDATE_KEYRING_PATH_DEFAULT "/usr/share/genesys/keys/update.gpg"
#endif

namespace genesys::launcher {

RuntimePaths::RuntimePaths()
    : RuntimePaths(QProcessEnvironment::systemEnvironment()) {}

RuntimePaths::RuntimePaths(const QProcessEnvironment& environment,
                           QString systemRuntimeRoot,
                           QString systemConfigPath,
                           QString updateKeyringPath) {
    const QString home = environment.value(QStringLiteral("HOME")).trimmed();
    hasUsableHome_ = !home.isEmpty() && QFileInfo(home).isAbsolute();

    if (hasUsableHome_) {
        configRoot_ = resolveXdgRoot(environment, QStringLiteral("XDG_CONFIG_HOME"), home, QStringLiteral(".config"));
        cacheRoot_ = resolveXdgRoot(environment, QStringLiteral("XDG_CACHE_HOME"), home, QStringLiteral(".cache"));
        dataRoot_ = resolveXdgRoot(environment, QStringLiteral("XDG_DATA_HOME"), home, QStringLiteral(".local/share"));
    }

    systemRuntimeRoot_ = systemRuntimeRoot.isEmpty()
        ? QString::fromUtf8(GENESYS_SYSTEM_RUNTIME_ROOT_DEFAULT)
        : QDir::cleanPath(systemRuntimeRoot);
    systemConfigPath_ = systemConfigPath.isEmpty()
        ? QString::fromUtf8(GENESYS_SYSTEM_CONFIG_PATH_DEFAULT)
        : QDir::cleanPath(systemConfigPath);
    updateKeyringPath_ = updateKeyringPath.isEmpty()
        ? QString::fromUtf8(GENESYS_UPDATE_KEYRING_PATH_DEFAULT)
        : QDir::cleanPath(updateKeyringPath);
}

QString RuntimePaths::resolveXdgRoot(const QProcessEnvironment& environment,
                                     const QString& variable,
                                     const QString& home,
                                     const QString& fallbackSuffix) {
    const QString candidate = environment.value(variable).trimmed();
    if (!candidate.isEmpty() && QFileInfo(candidate).isAbsolute()) {
        return QDir::cleanPath(candidate + QStringLiteral("/genesys"));
    }
    return QDir::cleanPath(home + QLatin1Char('/') + fallbackSuffix + QStringLiteral("/genesys"));
}

QString RuntimePaths::userConfigPath() const {
    return configRoot_.isEmpty() ? QString{} : QDir(configRoot_).filePath(QStringLiteral("update.conf"));
}

QString RuntimePaths::appsRoot() const {
    return dataRoot_.isEmpty() ? QString{} : QDir(dataRoot_).filePath(QStringLiteral("apps"));
}

QString RuntimePaths::currentRuntimeLink() const {
    return dataRoot_.isEmpty() ? QString{} : QDir(dataRoot_).filePath(QStringLiteral("current"));
}

QString RuntimePaths::downloadsRoot() const {
    return cacheRoot_.isEmpty() ? QString{} : QDir(cacheRoot_).filePath(QStringLiteral("downloads"));
}

QString RuntimePaths::logsRoot() const {
    return dataRoot_.isEmpty() ? QString{} : QDir(dataRoot_).filePath(QStringLiteral("logs"));
}

QString RuntimePaths::launcherLogPath() const {
    const QString root = logsRoot();
    return root.isEmpty() ? QString{} : QDir(root).filePath(QStringLiteral("launcher.log"));
}

QString RuntimePaths::updateStatePath() const {
    return cacheRoot_.isEmpty() ? QString{} : QDir(cacheRoot_).filePath(QStringLiteral("update-state.ini"));
}

QString RuntimePaths::updateLockPath() const {
    return dataRoot_.isEmpty() ? QString{} : QDir(dataRoot_).filePath(QStringLiteral("update.lock"));
}

QString RuntimePaths::systemBinRoot() const {
    return QDir(systemRuntimeRoot_).filePath(QStringLiteral("bin"));
}

} // namespace genesys::launcher
