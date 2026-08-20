#include "UpdateDownloader.h"

#include "LauncherLogger.h"

#include <QDir>
#include <QFile>

namespace genesys::launcher {

UpdateDownloader::UpdateDownloader(RuntimePaths paths,
                                   INetworkTransport& transport,
                                   LauncherLogger* logger)
    : paths_(std::move(paths)), transport_(transport), logger_(logger) {}

QString UpdateDownloader::bundleBaseName(const UpdateManifest& manifest) const {
    return QStringLiteral("genesys-runtime-%1-%2.tar.zst")
        .arg(manifest.version.toString(), manifest.platform.key);
}

DownloadResult UpdateDownloader::download(const UpdateManifest& manifest,
                                          const bool downloadSignature,
                                          ProgressCallback progress,
                                          CancellationCallback cancelled) {
    DownloadResult result;
    const QString root = paths_.downloadsRoot();
    if (root.isEmpty() || !QDir().mkpath(root)) {
        result.error = QStringLiteral("Unable to create the GenESyS download cache directory");
        return result;
    }

    const QString baseName = bundleBaseName(manifest);
    result.bundlePartPath = QDir(root).filePath(baseName + QStringLiteral(".part"));
    result.signaturePartPath = downloadSignature
        ? QDir(root).filePath(baseName + QStringLiteral(".sig.part"))
        : QString{};

    QFile::remove(result.bundlePartPath);
    if (!result.signaturePartPath.isEmpty()) {
        QFile::remove(result.signaturePartPath);
    }

    QFile bundle(result.bundlePartPath);
    if (!bundle.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        result.error = QStringLiteral("Unable to create temporary runtime bundle download");
        return result;
    }

    NetworkRequestOptions bundleOptions;
    bundleOptions.maximumBytes = manifest.platform.size;
    bundleOptions.timeoutMs = 60000;
    bundleOptions.maximumRedirects = 3;
    bundleOptions.httpsOnly = true;
    NetworkResponse bundleResponse = transport_.download(
        manifest.platform.bundleUrl,
        bundle,
        bundleOptions,
        std::move(progress),
        cancelled);
    bundle.close();

    if (!bundleResponse.ok || bundleResponse.bytesReceived != manifest.platform.size) {
        result.cancelled = bundleResponse.cancelled;
        result.error = bundleResponse.ok
            ? QStringLiteral("Downloaded runtime size does not match the signed manifest")
            : bundleResponse.error;
        cleanup(result);
        log(QStringLiteral("download-failed"), result.error);
        return result;
    }

    if (downloadSignature) {
        if (manifest.platform.signatureUrl.isEmpty()) {
            result.error = QStringLiteral("A signature is required, but the manifest has no signature_url");
            cleanup(result);
            log(QStringLiteral("download-failed"), result.error);
            return result;
        }

        QFile signature(result.signaturePartPath);
        if (!signature.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            result.error = QStringLiteral("Unable to create temporary signature download");
            cleanup(result);
            return result;
        }
        NetworkRequestOptions signatureOptions;
        signatureOptions.maximumBytes = 1024 * 1024;
        signatureOptions.timeoutMs = 15000;
        signatureOptions.maximumRedirects = 3;
        signatureOptions.httpsOnly = true;
        NetworkResponse signatureResponse = transport_.download(
            manifest.platform.signatureUrl,
            signature,
            signatureOptions,
            {},
            cancelled);
        signature.close();
        if (!signatureResponse.ok || signatureResponse.bytesReceived <= 0) {
            result.cancelled = signatureResponse.cancelled;
            result.error = signatureResponse.ok
                ? QStringLiteral("Downloaded signature is empty")
                : signatureResponse.error;
            cleanup(result);
            log(QStringLiteral("signature-download-failed"), result.error);
            return result;
        }
    }

    result.ok = true;
    log(QStringLiteral("download-complete"), QStringLiteral("bundle=%1 bytes=%2").arg(baseName).arg(manifest.platform.size));
    return result;
}

bool UpdateDownloader::promoteVerified(const UpdateManifest& manifest,
                                       const DownloadResult& download,
                                       QString* verifiedBundlePath,
                                       QString* verifiedSignaturePath,
                                       QString* error) const {
    if (!download.ok || download.bundlePartPath.isEmpty()) {
        if (error) {
            *error = QStringLiteral("No verified download is available for promotion");
        }
        return false;
    }

    const QString root = paths_.downloadsRoot();
    const QString baseName = bundleBaseName(manifest);
    const QString finalBundle = QDir(root).filePath(baseName);
    const QString finalSignature = download.signaturePartPath.isEmpty()
        ? QString{}
        : QDir(root).filePath(baseName + QStringLiteral(".sig"));

    QFile::remove(finalBundle);
    if (!QFile::rename(download.bundlePartPath, finalBundle)) {
        if (error) {
            *error = QStringLiteral("Unable to atomically promote the verified bundle in the download cache");
        }
        return false;
    }

    if (!download.signaturePartPath.isEmpty()) {
        QFile::remove(finalSignature);
        if (!QFile::rename(download.signaturePartPath, finalSignature)) {
            QFile::remove(finalBundle);
            if (error) {
                *error = QStringLiteral("Unable to promote the verified signature in the download cache");
            }
            return false;
        }
    }

    if (verifiedBundlePath) {
        *verifiedBundlePath = finalBundle;
    }
    if (verifiedSignaturePath) {
        *verifiedSignaturePath = finalSignature;
    }
    return true;
}

void UpdateDownloader::cleanup(const DownloadResult& download) const {
    if (!download.bundlePartPath.isEmpty()) {
        QFile::remove(download.bundlePartPath);
    }
    if (!download.signaturePartPath.isEmpty()) {
        QFile::remove(download.signaturePartPath);
    }
}

void UpdateDownloader::log(const QString& event, const QString& detail) const {
    if (logger_) {
        logger_->log(event, detail);
    }
}

} // namespace genesys::launcher
