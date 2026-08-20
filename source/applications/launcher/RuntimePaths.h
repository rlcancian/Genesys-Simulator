#pragma once

#include <QProcessEnvironment>
#include <QString>

namespace genesys::launcher {

class RuntimePaths {
public:
    RuntimePaths();
    explicit RuntimePaths(const QProcessEnvironment& environment,
                          QString systemRuntimeRoot = {},
                          QString systemConfigPath = {},
                          QString updateKeyringPath = {});

    [[nodiscard]] bool hasUsableHome() const noexcept { return hasUsableHome_; }
    [[nodiscard]] const QString& configRoot() const noexcept { return configRoot_; }
    [[nodiscard]] const QString& cacheRoot() const noexcept { return cacheRoot_; }
    [[nodiscard]] const QString& dataRoot() const noexcept { return dataRoot_; }
    [[nodiscard]] QString userConfigPath() const;
    [[nodiscard]] QString systemConfigPath() const { return systemConfigPath_; }
    [[nodiscard]] QString appsRoot() const;
    [[nodiscard]] QString currentRuntimeLink() const;
    [[nodiscard]] QString downloadsRoot() const;
    [[nodiscard]] QString logsRoot() const;
    [[nodiscard]] QString launcherLogPath() const;
    [[nodiscard]] QString updateStatePath() const;
    [[nodiscard]] QString updateLockPath() const;
    [[nodiscard]] QString systemRuntimeRoot() const { return systemRuntimeRoot_; }
    [[nodiscard]] QString systemBinRoot() const;
    [[nodiscard]] QString updateKeyringPath() const { return updateKeyringPath_; }

private:
    static QString resolveXdgRoot(const QProcessEnvironment& environment,
                                  const QString& variable,
                                  const QString& home,
                                  const QString& fallbackSuffix);

    bool hasUsableHome_ = false;
    QString configRoot_;
    QString cacheRoot_;
    QString dataRoot_;
    QString systemRuntimeRoot_;
    QString systemConfigPath_;
    QString updateKeyringPath_;
};

} // namespace genesys::launcher
