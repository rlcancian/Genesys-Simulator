#pragma once

#include "ArchiveExtractor.h"
#include "RuntimePaths.h"
#include "UpdateManifest.h"

#include <QLockFile>
#include <QString>

#include <memory>

namespace genesys::launcher {

class LauncherLogger;

struct RuntimeInstallResult {
    bool ok = false;
    QString runtimeRoot;
    QString previousRuntimeRoot;
    QString error;
};

class UserRuntimeManager {
public:
    UserRuntimeManager(RuntimePaths paths,
                       IArchiveExtractor& extractor,
                       LauncherLogger* logger = nullptr);
    ~UserRuntimeManager();

    UserRuntimeManager(const UserRuntimeManager&) = delete;
    UserRuntimeManager& operator=(const UserRuntimeManager&) = delete;

    [[nodiscard]] bool beginUpdate(QString* error = nullptr, int lockTimeoutMs = 0);
    void endUpdate();
    [[nodiscard]] bool hasUpdateLock() const noexcept { return lock_ && lock_->isLocked(); }

    [[nodiscard]] RuntimeInstallResult installAndActivate(const QString& verifiedBundlePath,
                                                          const UpdateManifest& manifest,
                                                          int maxUserVersions);

    [[nodiscard]] bool validateRuntime(const QString& runtimeRoot,
                                       const UpdateManifest& manifest,
                                       QString* error = nullptr) const;

private:
    [[nodiscard]] QString activeRuntimeRoot() const;
    [[nodiscard]] bool activateAtomically(const QString& runtimeRoot,
                                          QString* error) const;
    void cleanupOldVersions(int maxUserVersions,
                            const QString& activeRuntime,
                            const QString& rollbackRuntime) const;
    void log(const QString& event, const QString& detail = {}) const;

    RuntimePaths paths_;
    IArchiveExtractor& extractor_;
    LauncherLogger* logger_ = nullptr;
    std::unique_ptr<QLockFile> lock_;
};

} // namespace genesys::launcher
