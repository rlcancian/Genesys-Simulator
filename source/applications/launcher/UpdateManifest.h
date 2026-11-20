#pragma once

#include "Version.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace genesys::launcher {

struct UpdatePlatform {
    QString key;
    QString kind;
    QUrl bundleUrl;
    QString sha256;
    QUrl signatureUrl;
    qint64 size = 0;
    QStringList applications;
};

struct UpdateManifest {
    int schemaVersion = 0;
    QString product;
    QString channel;
    Version version;
    QString releaseTag;
    QString publishedAt;
    Version minimumLauncherVersion;
    Version minimumSystemPackageVersion;
    UpdatePlatform platform;
    QString notesSummary;
    QUrl notesUrl;
};

struct ManifestParseResult {
    bool ok = false;
    UpdateManifest manifest;
    QString error;
};

class UpdateManifestParser {
public:
    static constexpr qint64 MaximumBundleSize = 2LL * 1024 * 1024 * 1024;

    [[nodiscard]] static ManifestParseResult parse(const QByteArray& json,
                                                   const QString& expectedChannel,
                                                   const QString& platformKey,
                                                   const Version& currentRuntimeVersion,
                                                   const Version& launcherVersion,
                                                   const Version& systemPackageVersion);
};

} // namespace genesys::launcher
