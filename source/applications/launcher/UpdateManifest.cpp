#include "UpdateManifest.h"

#include "RuntimeSelector.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSet>

namespace genesys::launcher {
namespace {

bool isHttpsUrl(const QUrl& url) {
    return url.isValid() && !url.host().isEmpty() &&
           url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0 &&
           url.userInfo().isEmpty();
}

ManifestParseResult fail(QString error) {
    ManifestParseResult result;
    result.error = std::move(error);
    return result;
}

} // namespace

ManifestParseResult UpdateManifestParser::parse(const QByteArray& json,
                                                const QString& expectedChannel,
                                                const QString& platformKey,
                                                const Version& currentRuntimeVersion,
                                                const Version& launcherVersion,
                                                const Version& systemPackageVersion) {
    QJsonParseError jsonError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &jsonError);
    if (jsonError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(QStringLiteral("Manifest is not valid JSON object data"));
    }

    const QJsonObject object = document.object();
    if (!object.value(QStringLiteral("schema_version")).isDouble() ||
        object.value(QStringLiteral("schema_version")).toInt(-1) != 1) {
        return fail(QStringLiteral("Unsupported manifest schema_version"));
    }
    if (object.value(QStringLiteral("product")).toString() != QStringLiteral("genesys-simulator")) {
        return fail(QStringLiteral("Manifest product mismatch"));
    }
    if (object.value(QStringLiteral("channel")).toString() != expectedChannel) {
        return fail(QStringLiteral("Manifest channel mismatch"));
    }

    const Version version = Version::parse(object.value(QStringLiteral("version")).toString());
    if (!version.isValid()) {
        return fail(QStringLiteral("Manifest version is invalid"));
    }
    if (currentRuntimeVersion.isValid() && version < currentRuntimeVersion) {
        return fail(QStringLiteral("Manifest would downgrade the active runtime"));
    }

    const QString releaseTag = object.value(QStringLiteral("release_tag")).toString();
    if (releaseTag.isEmpty() || Version::parse(releaseTag) != version) {
        return fail(QStringLiteral("Manifest release_tag does not match version"));
    }

    const QString publishedAt = object.value(QStringLiteral("published_at")).toString();
    if (publishedAt.isEmpty() || !QDateTime::fromString(publishedAt, Qt::ISODate).isValid()) {
        return fail(QStringLiteral("Manifest published_at is invalid"));
    }

    const Version minimumLauncher = Version::parse(object.value(QStringLiteral("minimum_launcher_version")).toString());
    const Version minimumSystem = Version::parse(object.value(QStringLiteral("minimum_system_package_version")).toString());
    if (!minimumLauncher.isValid() || !minimumSystem.isValid()) {
        return fail(QStringLiteral("Manifest minimum version fields are invalid"));
    }
    if (!launcherVersion.isValid() || launcherVersion < minimumLauncher) {
        return fail(QStringLiteral("Launcher version does not satisfy minimum_launcher_version"));
    }
    if (!systemPackageVersion.isValid() || systemPackageVersion < minimumSystem) {
        return fail(QStringLiteral("System package version does not satisfy minimum_system_package_version"));
    }

    if (!object.value(QStringLiteral("platforms")).isObject()) {
        return fail(QStringLiteral("Manifest platforms object is missing"));
    }
    const QJsonObject platforms = object.value(QStringLiteral("platforms")).toObject();
    if (!platforms.value(platformKey).isObject()) {
        return fail(QStringLiteral("Manifest does not contain the current platform"));
    }
    const QJsonObject platformObject = platforms.value(platformKey).toObject();

    const QString kind = platformObject.value(QStringLiteral("kind")).toString();
    if (kind != QStringLiteral("user-runtime")) {
        return fail(QStringLiteral("Platform artifact kind is not user-runtime"));
    }

    const QUrl bundleUrl(platformObject.value(QStringLiteral("url")).toString(), QUrl::StrictMode);
    if (!isHttpsUrl(bundleUrl)) {
        return fail(QStringLiteral("Bundle URL must be an absolute HTTPS URL without userinfo"));
    }

    const QString sha256 = platformObject.value(QStringLiteral("sha256")).toString().toLower();
    static const QRegularExpression shaPattern(QStringLiteral("^[0-9a-f]{64}$"));
    if (!shaPattern.match(sha256).hasMatch()) {
        return fail(QStringLiteral("Bundle SHA-256 is invalid"));
    }

    QUrl signatureUrl;
    if (platformObject.contains(QStringLiteral("signature_url"))) {
        signatureUrl = QUrl(platformObject.value(QStringLiteral("signature_url")).toString(), QUrl::StrictMode);
        if (!isHttpsUrl(signatureUrl)) {
            return fail(QStringLiteral("Signature URL must be an absolute HTTPS URL without userinfo"));
        }
    }

    if (!platformObject.value(QStringLiteral("size")).isDouble()) {
        return fail(QStringLiteral("Bundle size is missing or invalid"));
    }
    const double sizeAsDouble = platformObject.value(QStringLiteral("size")).toDouble(-1.0);
    const qint64 size = static_cast<qint64>(sizeAsDouble);
    if (sizeAsDouble < 1.0 || sizeAsDouble != static_cast<double>(size) || size > MaximumBundleSize) {
        return fail(QStringLiteral("Bundle size is outside the accepted range"));
    }

    if (!platformObject.value(QStringLiteral("applications")).isArray()) {
        return fail(QStringLiteral("Platform applications list is missing"));
    }
    QStringList applications;
    QSet<QString> seen;
    for (const QJsonValue& value : platformObject.value(QStringLiteral("applications")).toArray()) {
        if (!value.isString()) {
            return fail(QStringLiteral("Platform applications must contain strings only"));
        }
        const QString application = value.toString();
        if (!RuntimeSelector::isSupportedApplicationName(application) || seen.contains(application)) {
            return fail(QStringLiteral("Platform applications contains an invalid or duplicate name"));
        }
        seen.insert(application);
        applications.push_back(application);
    }
    if (applications.isEmpty()) {
        return fail(QStringLiteral("Platform applications list is empty"));
    }

    UpdateManifest manifest;
    manifest.schemaVersion = 1;
    manifest.product = QStringLiteral("genesys-simulator");
    manifest.channel = expectedChannel;
    manifest.version = version;
    manifest.releaseTag = releaseTag;
    manifest.publishedAt = publishedAt;
    manifest.minimumLauncherVersion = minimumLauncher;
    manifest.minimumSystemPackageVersion = minimumSystem;
    manifest.platform.key = platformKey;
    manifest.platform.kind = kind;
    manifest.platform.bundleUrl = bundleUrl;
    manifest.platform.sha256 = sha256;
    manifest.platform.signatureUrl = signatureUrl;
    manifest.platform.size = size;
    manifest.platform.applications = applications;

    if (object.value(QStringLiteral("notes")).isObject()) {
        const QJsonObject notes = object.value(QStringLiteral("notes")).toObject();
        manifest.notesSummary = notes.value(QStringLiteral("summary")).toString().left(4096);
        const QUrl notesUrl(notes.value(QStringLiteral("url")).toString(), QUrl::StrictMode);
        if (!notesUrl.isEmpty() && !isHttpsUrl(notesUrl)) {
            return fail(QStringLiteral("Release notes URL must be HTTPS"));
        }
        manifest.notesUrl = notesUrl;
    }

    ManifestParseResult result;
    result.ok = true;
    result.manifest = std::move(manifest);
    return result;
}

} // namespace genesys::launcher
