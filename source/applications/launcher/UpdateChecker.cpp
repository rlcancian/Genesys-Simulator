#include "UpdateChecker.h"

#include "LauncherLogger.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace genesys::launcher {

UpdateChecker::UpdateChecker(RuntimePaths paths,
                             LauncherConfig config,
                             PlatformInfo platform,
                             Version currentRuntimeVersion,
                             Version launcherVersion,
                             Version systemPackageVersion,
                             INetworkTransport& transport,
                             LauncherLogger* logger,
                             Clock clock)
    : paths_(std::move(paths)),
      config_(std::move(config)),
      platform_(std::move(platform)),
      currentRuntimeVersion_(std::move(currentRuntimeVersion)),
      launcherVersion_(std::move(launcherVersion)),
      systemPackageVersion_(std::move(systemPackageVersion)),
      transport_(transport),
      logger_(logger),
      clock_(std::move(clock)) {
    if (!clock_) {
        clock_ = [] { return QDateTime::currentDateTimeUtc(); };
    }
}

bool UpdateChecker::isRateLimited(const QDateTime& now) const {
    if (config_.minimumCheckIntervalHours <= 0 || paths_.updateStatePath().isEmpty()) {
        return false;
    }
    QSettings settings(paths_.updateStatePath(), QSettings::IniFormat);
    const QDateTime last = settings.value(QStringLiteral("updates/last_attempt_utc")).toDateTime().toUTC();
    if (!last.isValid()) {
        return false;
    }
    return last.secsTo(now.toUTC()) >= 0 &&
           last.secsTo(now.toUTC()) < static_cast<qint64>(config_.minimumCheckIntervalHours) * 3600;
}

void UpdateChecker::recordAttempt(const QDateTime& now) const {
    const QString statePath = paths_.updateStatePath();
    if (statePath.isEmpty()) {
        return;
    }
    QDir().mkpath(QFileInfo(statePath).absolutePath());
    QSettings settings(statePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("updates/last_attempt_utc"), now.toUTC());
    settings.sync();
}

UpdateCheckResult UpdateChecker::check(const bool force) {
    UpdateCheckResult result;
    if (!config_.updatesEnabled || !config_.allowUserRuntime || config_.manifestUrl.isEmpty()) {
        result.status = UpdateCheckStatus::Disabled;
        result.message = QStringLiteral("Remote updates are disabled by effective policy");
        log(QStringLiteral("update-check-disabled"), result.message);
        return result;
    }

    if (!platform_.valid || platform_.manifestKey.isEmpty()) {
        result.status = UpdateCheckStatus::UnsupportedPlatform;
        result.message = QStringLiteral("Unable to identify a supported update platform");
        log(QStringLiteral("update-check-platform"), result.message);
        return result;
    }

    const QDateTime now = clock_().toUTC();
    if (!force && isRateLimited(now)) {
        result.status = UpdateCheckStatus::RateLimited;
        result.ok = true;
        result.message = QStringLiteral("Update check skipped because the minimum interval has not elapsed");
        log(QStringLiteral("update-check-rate-limited"), result.message);
        return result;
    }

    NetworkRequestOptions options;
    options.maximumBytes = 1024 * 1024;
    options.timeoutMs = 8000;
    options.maximumRedirects = 3;
    options.httpsOnly = true;

    auto [response, bytes] = transport_.get(config_.manifestUrl, options);
    recordAttempt(now);
    if (!response.ok) {
        result.status = UpdateCheckStatus::NetworkError;
        result.message = QStringLiteral("Update manifest request failed: %1").arg(response.error);
        log(QStringLiteral("update-check-network-error"), result.message);
        return result;
    }

    const ManifestParseResult parsed = UpdateManifestParser::parse(
        bytes,
        config_.channel,
        platform_.manifestKey,
        currentRuntimeVersion_,
        launcherVersion_,
        systemPackageVersion_);
    if (!parsed.ok) {
        result.status = UpdateCheckStatus::InvalidManifest;
        result.message = parsed.error;
        log(QStringLiteral("update-check-invalid-manifest"), result.message);
        return result;
    }

    result.ok = true;
    result.manifest = parsed.manifest;
    if (currentRuntimeVersion_.isValid() && parsed.manifest.version <= currentRuntimeVersion_) {
        result.status = UpdateCheckStatus::UpToDate;
        result.message = QStringLiteral("The active runtime is up to date");
        log(QStringLiteral("update-check-up-to-date"), parsed.manifest.version.toString());
        return result;
    }

    result.status = UpdateCheckStatus::UpdateAvailable;
    result.updateAvailable = true;
    result.message = QStringLiteral("Update %1 is available").arg(parsed.manifest.version.toString());
    log(QStringLiteral("update-check-available"), result.message);
    return result;
}

void UpdateChecker::log(const QString& event, const QString& detail) const {
    if (logger_) {
        logger_->log(event, detail);
    }
}

} // namespace genesys::launcher
