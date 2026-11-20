#pragma once

#include "LauncherConfig.h"
#include "NetworkTransport.h"
#include "PlatformInfo.h"
#include "RuntimePaths.h"
#include "UpdateManifest.h"
#include "Version.h"

#include <QDateTime>

#include <functional>

namespace genesys::launcher {

class LauncherLogger;

enum class UpdateCheckStatus {
    Disabled,
    RateLimited,
    UnsupportedPlatform,
    NetworkError,
    InvalidManifest,
    UpToDate,
    UpdateAvailable
};

struct UpdateCheckResult {
    UpdateCheckStatus status = UpdateCheckStatus::Disabled;
    bool ok = false;
    bool updateAvailable = false;
    UpdateManifest manifest;
    QString message;
};

class UpdateChecker {
public:
    using Clock = std::function<QDateTime()>;

    UpdateChecker(RuntimePaths paths,
                  LauncherConfig config,
                  PlatformInfo platform,
                  Version currentRuntimeVersion,
                  Version launcherVersion,
                  Version systemPackageVersion,
                  INetworkTransport& transport,
                  LauncherLogger* logger = nullptr,
                  Clock clock = {});

    [[nodiscard]] UpdateCheckResult check(bool force = false);

private:
    [[nodiscard]] bool isRateLimited(const QDateTime& now) const;
    void recordAttempt(const QDateTime& now) const;
    void log(const QString& event, const QString& detail = {}) const;

    RuntimePaths paths_;
    LauncherConfig config_;
    PlatformInfo platform_;
    Version currentRuntimeVersion_;
    Version launcherVersion_;
    Version systemPackageVersion_;
    INetworkTransport& transport_;
    LauncherLogger* logger_ = nullptr;
    Clock clock_;
};

} // namespace genesys::launcher
