#include "LauncherController.h"

#include "LauncherLogger.h"

#include <QFile>

namespace genesys::launcher {

LauncherController::LauncherController(LauncherConfig config,
                                       UpdateChecker& checker,
                                       UpdateDownloader& downloader,
                                       ISignatureVerifier* signatureVerifier,
                                       UserRuntimeManager& runtimeManager,
                                       IUpdateUi& ui,
                                       LauncherLogger* logger)
    : config_(std::move(config)),
      checker_(checker),
      downloader_(downloader),
      signatureVerifier_(signatureVerifier),
      runtimeManager_(runtimeManager),
      ui_(ui),
      logger_(logger) {}

LauncherUpdateResult LauncherController::failUpdate(QString message, const bool interactive) {
    LauncherUpdateResult result;
    result.checkCompleted = true;
    result.message = std::move(message);
    log(QStringLiteral("update-failed"), result.message);
    if (interactive) {
        ui_.showError(QStringLiteral("The update could not be installed. GenESyS will continue with the previous runtime or the system fallback.\n\n%1")
                          .arg(result.message));
    }
    return result;
}

LauncherUpdateResult LauncherController::checkAndMaybeUpdate(const Version& currentVersion,
                                                              const bool interactive,
                                                              const bool forceCheck,
                                                              const bool checkOnly) {
    LauncherUpdateResult result;
    const UpdateCheckResult check = checker_.check(forceCheck);
    result.checkCompleted = check.ok || check.status == UpdateCheckStatus::Disabled ||
                            check.status == UpdateCheckStatus::UnsupportedPlatform ||
                            check.status == UpdateCheckStatus::NetworkError ||
                            check.status == UpdateCheckStatus::InvalidManifest;
    result.updateAvailable = check.updateAvailable;
    result.message = check.message;

    if (!check.updateAvailable) {
        if (checkOnly && check.ok) {
            ui_.showInfo(check.message);
        } else if (interactive && !check.ok && check.status != UpdateCheckStatus::Disabled &&
                   check.status != UpdateCheckStatus::RateLimited) {
            ui_.showError(QStringLiteral("Update check failed. GenESyS will continue normally.\n\n%1").arg(check.message));
        }
        return result;
    }

    if (checkOnly) {
        ui_.showInfo(check.message);
        return result;
    }
    if (!interactive) {
        log(QStringLiteral("update-deferred"), QStringLiteral("candidate=%1 non-interactive mode").arg(check.manifest.version.toString()));
        return result;
    }
    if (!ui_.confirmUpdate(currentVersion, check.manifest.version, check.manifest.notesSummary)) {
        result.message = QStringLiteral("Update deferred by user");
        log(QStringLiteral("update-deferred"), QStringLiteral("candidate=%1").arg(check.manifest.version.toString()));
        return result;
    }

    QString lockError;
    if (!runtimeManager_.beginUpdate(&lockError, 0)) {
        return failUpdate(lockError, true);
    }

    struct UnlockGuard {
        UserRuntimeManager& manager;
        ~UnlockGuard() { manager.endUpdate(); }
    } unlockGuard{runtimeManager_};

    bool progressCancelled = false;
    ui_.beginDownload(check.manifest.platform.size);
    const DownloadResult download = downloader_.download(
        check.manifest,
        config_.requireSignature,
        [&](const qint64 received, const qint64 total) {
            if (!ui_.updateDownloadProgress(received, total)) {
                progressCancelled = true;
            }
        },
        [&] { return progressCancelled; });
    ui_.finishDownload();

    if (!download.ok) {
        return failUpdate(download.cancelled ? QStringLiteral("Update download was cancelled") : download.error, true);
    }

    const VerificationResult checksum = UpdateVerifier::verifySha256(download.bundlePartPath,
                                                                     check.manifest.platform.sha256);
    if (!checksum.ok) {
        downloader_.cleanup(download);
        return failUpdate(checksum.error, true);
    }

    if (config_.requireSignature) {
        if (!signatureVerifier_) {
            downloader_.cleanup(download);
            return failUpdate(QStringLiteral("Signature verification is required but no verifier is available"), true);
        }
        const VerificationResult signature = signatureVerifier_->verify(download.bundlePartPath,
                                                                         download.signaturePartPath);
        if (!signature.ok) {
            downloader_.cleanup(download);
            return failUpdate(signature.error, true);
        }
    }

    QString verifiedBundle;
    QString verifiedSignature;
    QString promotionError;
    if (!downloader_.promoteVerified(check.manifest,
                                     download,
                                     &verifiedBundle,
                                     &verifiedSignature,
                                     &promotionError)) {
        downloader_.cleanup(download);
        return failUpdate(promotionError, true);
    }

    const RuntimeInstallResult installation = runtimeManager_.installAndActivate(
        verifiedBundle,
        check.manifest,
        config_.maxUserVersions);

    QFile::remove(verifiedBundle);
    if (!verifiedSignature.isEmpty()) {
        QFile::remove(verifiedSignature);
    }

    if (!installation.ok) {
        return failUpdate(installation.error, true);
    }

    result.installed = true;
    result.installedVersion = check.manifest.version;
    result.message = QStringLiteral("GenESyS runtime %1 was installed and activated")
        .arg(check.manifest.version.toString());
    log(QStringLiteral("update-installed"), result.message);
    ui_.showInfo(result.message);
    return result;
}

void LauncherController::log(const QString& event, const QString& detail) const {
    if (logger_) {
        logger_->log(event, detail);
    }
}

} // namespace genesys::launcher
