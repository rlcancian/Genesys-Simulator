#pragma once

#include "NetworkTransport.h"
#include "RuntimePaths.h"
#include "UpdateManifest.h"

#include <QString>

namespace genesys::launcher {

class LauncherLogger;

struct DownloadResult {
    bool ok = false;
    bool cancelled = false;
    QString bundlePartPath;
    QString signaturePartPath;
    QString error;
};

class UpdateDownloader {
public:
    UpdateDownloader(RuntimePaths paths,
                     INetworkTransport& transport,
                     LauncherLogger* logger = nullptr);

    [[nodiscard]] DownloadResult download(const UpdateManifest& manifest,
                                          bool downloadSignature,
                                          ProgressCallback progress = {},
                                          CancellationCallback cancelled = {});

    [[nodiscard]] bool promoteVerified(const UpdateManifest& manifest,
                                       const DownloadResult& download,
                                       QString* verifiedBundlePath,
                                       QString* verifiedSignaturePath,
                                       QString* error = nullptr) const;

    void cleanup(const DownloadResult& download) const;

private:
    [[nodiscard]] QString bundleBaseName(const UpdateManifest& manifest) const;
    void log(const QString& event, const QString& detail = {}) const;

    RuntimePaths paths_;
    INetworkTransport& transport_;
    LauncherLogger* logger_ = nullptr;
};

} // namespace genesys::launcher
