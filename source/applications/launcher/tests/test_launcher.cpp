#include "ArchiveExtractor.h"
#include "DispatchController.h"
#include "LauncherConfig.h"
#include "LauncherController.h"
#include "NetworkTransport.h"
#include "PlatformInfo.h"
#include "ProcessLauncher.h"
#include "RuntimePaths.h"
#include "RuntimeSelector.h"
#include "UpdateChecker.h"
#include "UpdateDialog.h"
#include "UpdateDownloader.h"
#include "UpdateManifest.h"
#include "UpdateVerifier.h"
#include "UserRuntimeManager.h"
#include "Version.h"

#include <gtest/gtest.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QUrl>

#include <map>
#include <utility>

using namespace genesys::launcher;

namespace {

const QStringList kCompleteApplications = {
    QStringLiteral("genesys-gui"),
    QStringLiteral("genesys-shell"),
    QStringLiteral("genesys-worker"),
    QStringLiteral("genesys-mcp")
};

bool writeFile(const QString& path, const QByteArray& contents, const bool executable = false) {
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    if (file.write(contents) != contents.size()) {
        return false;
    }
    file.close();
    if (executable) {
        return QFile::setPermissions(path,
                                     QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner |
                                     QFileDevice::ReadGroup | QFileDevice::ExeGroup |
                                     QFileDevice::ReadOther | QFileDevice::ExeOther);
    }
    return true;
}

QByteArray runtimeManifestJson(const QString& version, const QStringList& applications) {
    QJsonArray array;
    for (const QString& application : applications) {
        array.append(application);
    }
    QJsonObject object;
    object.insert(QStringLiteral("schema_version"), 1);
    object.insert(QStringLiteral("product"), QStringLiteral("genesys-simulator"));
    object.insert(QStringLiteral("version"), version);
    object.insert(QStringLiteral("applications"), array);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

bool createRuntimeTree(const QString& root,
                       const QString& version,
                       const QStringList& applications = kCompleteApplications) {
    if (!QDir().mkpath(QDir(root).filePath(QStringLiteral("bin")))) {
        return false;
    }
    if (!writeFile(QDir(root).filePath(QStringLiteral("VERSION")), version.toUtf8() + '\n')) {
        return false;
    }
    if (!writeFile(QDir(root).filePath(QStringLiteral("MANIFEST.json")), runtimeManifestJson(version, applications))) {
        return false;
    }
    for (const QString& application : applications) {
        if (!writeFile(QDir(root).filePath(QStringLiteral("bin/") + application),
                       QByteArray("#!/bin/sh\nexit 0\n"),
                       true)) {
            return false;
        }
    }
    return true;
}

RuntimePaths makePaths(const QTemporaryDir& temp) {
    const QString home = QDir(temp.path()).filePath(QStringLiteral("home"));
    QDir().mkpath(home);
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("HOME"), home);
    return RuntimePaths(environment,
                        QDir(temp.path()).filePath(QStringLiteral("system")),
                        QDir(temp.path()).filePath(QStringLiteral("etc/update.conf")),
                        QDir(temp.path()).filePath(QStringLiteral("keys/update.gpg")));
}

QString addSystemExecutable(const RuntimePaths& paths, const QString& application) {
    const QString path = QDir(paths.systemBinRoot()).filePath(application);
    EXPECT_TRUE(writeFile(path, QByteArray("#!/bin/sh\nexit 0\n"), true));
    return path;
}

QString addUserRuntime(const RuntimePaths& paths,
                       const QString& version,
                       const QStringList& applications = kCompleteApplications,
                       const bool activate = true) {
    const QString root = QDir(paths.appsRoot()).filePath(version);
    EXPECT_TRUE(createRuntimeTree(root, version, applications));
    if (activate) {
        QDir().mkpath(paths.dataRoot());
        QFile::remove(paths.currentRuntimeLink());
        EXPECT_TRUE(QFile::link(root, paths.currentRuntimeLink()));
    }
    return root;
}

QByteArray remoteManifestJson(const QString& version,
                              const QString& sha256,
                              const qint64 size,
                              const QStringList& applications = kCompleteApplications,
                              const QString& channel = QStringLiteral("stable"),
                              const QString& platform = QStringLiteral("ubuntu-24.04-x86_64"),
                              const QString& minimumLauncher = QStringLiteral("2026.1.0"),
                              const QString& minimumSystem = QStringLiteral("2026.1.0")) {
    QJsonArray apps;
    for (const QString& application : applications) {
        apps.append(application);
    }
    QJsonObject platformObject;
    platformObject.insert(QStringLiteral("kind"), QStringLiteral("user-runtime"));
    platformObject.insert(QStringLiteral("url"), QStringLiteral("https://example.test/runtime.tar.zst"));
    platformObject.insert(QStringLiteral("sha256"), sha256);
    platformObject.insert(QStringLiteral("signature_url"), QStringLiteral("https://example.test/runtime.tar.zst.sig"));
    platformObject.insert(QStringLiteral("size"), static_cast<double>(size));
    platformObject.insert(QStringLiteral("applications"), apps);

    QJsonObject platforms;
    platforms.insert(platform, platformObject);

    QJsonObject notes;
    notes.insert(QStringLiteral("summary"), QStringLiteral("Fixture release notes"));
    notes.insert(QStringLiteral("url"), QStringLiteral("https://example.test/release"));

    QJsonObject object;
    object.insert(QStringLiteral("schema_version"), 1);
    object.insert(QStringLiteral("product"), QStringLiteral("genesys-simulator"));
    object.insert(QStringLiteral("channel"), channel);
    object.insert(QStringLiteral("version"), version);
    object.insert(QStringLiteral("release_tag"), QStringLiteral("v") + version);
    object.insert(QStringLiteral("published_at"), QStringLiteral("2026-08-20T12:00:00Z"));
    object.insert(QStringLiteral("minimum_launcher_version"), minimumLauncher);
    object.insert(QStringLiteral("minimum_system_package_version"), minimumSystem);
    object.insert(QStringLiteral("platforms"), platforms);
    object.insert(QStringLiteral("notes"), notes);
    return QJsonDocument(object).toJson(QJsonDocument::Compact);
}

UpdateManifest makeManifest(const QString& version,
                            const QString& sha256 = QString(64, QLatin1Char('a')),
                            const qint64 size = 16,
                            const QStringList& applications = kCompleteApplications) {
    UpdateManifest manifest;
    manifest.schemaVersion = 1;
    manifest.product = QStringLiteral("genesys-simulator");
    manifest.channel = QStringLiteral("stable");
    manifest.version = Version::parse(version);
    manifest.releaseTag = QStringLiteral("v") + version;
    manifest.publishedAt = QStringLiteral("2026-08-20T12:00:00Z");
    manifest.minimumLauncherVersion = Version::parse(QStringLiteral("2026.1.0"));
    manifest.minimumSystemPackageVersion = Version::parse(QStringLiteral("2026.1.0"));
    manifest.platform.key = QStringLiteral("ubuntu-24.04-x86_64");
    manifest.platform.kind = QStringLiteral("user-runtime");
    manifest.platform.bundleUrl = QUrl(QStringLiteral("https://example.test/runtime.tar.zst"));
    manifest.platform.signatureUrl = QUrl(QStringLiteral("https://example.test/runtime.tar.zst.sig"));
    manifest.platform.sha256 = sha256;
    manifest.platform.size = size;
    manifest.platform.applications = applications;
    return manifest;
}

class FakeNetworkTransport final : public INetworkTransport {
public:
    std::map<QString, QByteArray> responses;
    int calls = 0;

    NetworkResponse download(const QUrl& url,
                             QIODevice& destination,
                             const NetworkRequestOptions& options,
                             ProgressCallback progress,
                             CancellationCallback cancelled) override {
        ++calls;
        NetworkResponse result;
        result.finalUrl = url;
        result.httpStatus = 200;
        const auto it = responses.find(url.toString());
        if (it == responses.end()) {
            result.error = QStringLiteral("fixture URL not found");
            return result;
        }
        if (cancelled && cancelled()) {
            result.cancelled = true;
            result.error = QStringLiteral("cancelled");
            return result;
        }
        const QByteArray& bytes = it->second;
        if (bytes.size() > options.maximumBytes) {
            result.error = QStringLiteral("fixture exceeded limit");
            return result;
        }
        if (destination.write(bytes) != bytes.size()) {
            result.error = QStringLiteral("fixture write failed");
            return result;
        }
        result.bytesReceived = bytes.size();
        if (progress) {
            progress(bytes.size(), bytes.size());
        }
        result.ok = true;
        return result;
    }
};

class FakeSignatureVerifier final : public ISignatureVerifier {
public:
    bool accept = true;
    int calls = 0;

    VerificationResult verify(const QString&, const QString&) override {
        ++calls;
        return accept ? VerificationResult{true, {}}
                      : VerificationResult{false, QStringLiteral("fixture signature rejected")};
    }
};

class FakeExtractor final : public IArchiveExtractor {
public:
    QString version = QStringLiteral("2026.2.0");
    QStringList applications = kCompleteApplications;
    bool fail = false;
    int calls = 0;

    ExtractionResult extract(const QString&, const QString& destinationRoot) override {
        ++calls;
        if (fail) {
            return {false, QStringLiteral("fixture extraction failed")};
        }
        if (!createRuntimeTree(destinationRoot, version, applications)) {
            return {false, QStringLiteral("fixture runtime creation failed")};
        }
        return {true, {}};
    }
};

class FakeUpdateUi final : public IUpdateUi {
public:
    bool accept = true;
    bool continueDownload = true;
    int confirmations = 0;
    int errors = 0;
    int infos = 0;

    bool confirmUpdate(const Version&, const Version&, const QString&) override {
        ++confirmations;
        return accept;
    }
    void beginDownload(qint64) override {}
    bool updateDownloadProgress(qint64, qint64) override { return continueDownload; }
    void finishDownload() override {}
    void showError(const QString&) override { ++errors; }
    void showInfo(const QString&) override { ++infos; }
};

class FakeProcessLauncher final : public IProcessLauncher {
public:
    int returnCode = 0;
    int calls = 0;
    RuntimeSelection selection;
    QStringList arguments;

    int launchReplacingProcess(const RuntimeSelection& selected,
                               const QStringList& args,
                               QString*) override {
        ++calls;
        selection = selected;
        arguments = args;
        return returnCode;
    }
};

LauncherConfig enabledConfig(const QUrl& manifestUrl = QUrl(QStringLiteral("https://example.test/update-manifest.json"))) {
    LauncherConfig config;
    config.updatesEnabled = true;
    config.allowUserRuntime = true;
    config.requireSignature = true;
    config.checkOnGuiStartup = true;
    config.fallbackToSystem = true;
    config.minimumCheckIntervalHours = 0;
    config.maxUserVersions = 2;
    config.manifestUrl = manifestUrl;
    config.hasExplicitPolicy = true;
    return config;
}

PlatformInfo fixturePlatform() {
    return {QStringLiteral("ubuntu"), QStringLiteral("24.04"), QStringLiteral("x86_64"),
            QStringLiteral("ubuntu-24.04-x86_64"), true};
}

} // namespace

TEST(LauncherVersion, ParsesValidVersionsAndVTags) {
    EXPECT_EQ(Version::parse(QStringLiteral("2026.1.0")).toString(), QStringLiteral("2026.1.0"));
    EXPECT_EQ(Version::parse(QStringLiteral("v2026.1.2")).toString(), QStringLiteral("2026.1.2"));
    EXPECT_TRUE(Version::parse(QStringLiteral("1.2.3.4")).isValid());
}

TEST(LauncherVersion, RejectsInvalidVersions) {
    EXPECT_FALSE(Version::parse(QString{}).isValid());
    EXPECT_FALSE(Version::parse(QStringLiteral("v")).isValid());
    EXPECT_FALSE(Version::parse(QStringLiteral("2026..1")).isValid());
    EXPECT_FALSE(Version::parse(QStringLiteral("2026.1-beta")).isValid());
}

TEST(LauncherVersion, ComparesComponentsAndEqualityDeterministically) {
    EXPECT_LT(Version::parse(QStringLiteral("2026.1.2")), Version::parse(QStringLiteral("2026.2.0")));
    EXPECT_GT(Version::parse(QStringLiteral("2026.2.0")), Version::parse(QStringLiteral("2026.1.99")));
    EXPECT_EQ(Version::parse(QStringLiteral("2026.1")), Version::parse(QStringLiteral("2026.1.0")));
    EXPECT_NE(Version::parse(QStringLiteral("2026.1.1")), Version::parse(QStringLiteral("2026.1.0")));
}

TEST(LauncherVersion, DetectsDowngradeOrdering) {
    const Version current = Version::parse(QStringLiteral("2026.2.0"));
    const Version candidate = Version::parse(QStringLiteral("2026.1.9"));
    EXPECT_LT(candidate, current);
}

TEST(LauncherConfiguration, DefaultsAreFailSafeWithoutPolicy) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const LauncherConfig config = LauncherConfig::load(QDir(temp.path()).filePath(QStringLiteral("system.conf")),
                                                        QDir(temp.path()).filePath(QStringLiteral("user.conf")));
    EXPECT_FALSE(config.updatesEnabled);
    EXPECT_TRUE(config.allowUserRuntime);
    EXPECT_TRUE(config.requireSignature);
    EXPECT_TRUE(config.fallbackToSystem);
    EXPECT_EQ(config.minimumCheckIntervalHours, 12);
    EXPECT_EQ(config.maxUserVersions, 2);
}

TEST(LauncherConfiguration, LoadsUserConfiguration) {
    QTemporaryDir temp;
    ASSERT_TRUE(writeFile(QDir(temp.path()).filePath(QStringLiteral("user.conf")),
                          "[updates]\nenabled=true\nchannel=testing\nallow_user_runtime=true\nrequire_signature=false\n"
                          "check_on_gui_startup=false\nfallback_to_system=true\nminimum_check_interval_hours=3\n"
                          "max_user_versions=4\nmanifest_url=https://example.test/manifest.json\n"));
    const LauncherConfig config = LauncherConfig::load({}, QDir(temp.path()).filePath(QStringLiteral("user.conf")));
    EXPECT_TRUE(config.updatesEnabled);
    EXPECT_EQ(config.channel, QStringLiteral("testing"));
    EXPECT_FALSE(config.requireSignature);
    EXPECT_FALSE(config.checkOnGuiStartup);
    EXPECT_EQ(config.minimumCheckIntervalHours, 3);
    EXPECT_EQ(config.maxUserVersions, 4);
}

TEST(LauncherConfiguration, SystemConfigurationOverridesUserValues) {
    QTemporaryDir temp;
    const QString user = QDir(temp.path()).filePath(QStringLiteral("user.conf"));
    const QString system = QDir(temp.path()).filePath(QStringLiteral("system.conf"));
    ASSERT_TRUE(writeFile(user, "[updates]\nenabled=true\nallow_user_runtime=true\nchannel=testing\nmanifest_url=https://example.test/manifest.json\n"));
    ASSERT_TRUE(writeFile(system, "[updates]\nenabled=false\nallow_user_runtime=false\nrequire_signature=true\n"));
    const LauncherConfig config = LauncherConfig::load(system, user);
    EXPECT_FALSE(config.updatesEnabled);
    EXPECT_FALSE(config.allowUserRuntime);
    EXPECT_TRUE(config.requireSignature);
    EXPECT_EQ(config.channel, QStringLiteral("testing"));
}

TEST(LauncherConfiguration, InvalidSystemSecurityValuesFailSafe) {
    QTemporaryDir temp;
    const QString system = QDir(temp.path()).filePath(QStringLiteral("system.conf"));
    ASSERT_TRUE(writeFile(system,
                          "[updates]\nenabled=maybe\nallow_user_runtime=maybe\nrequire_signature=maybe\n"
                          "minimum_check_interval_hours=-2\nmax_user_versions=0\nmanifest_url=https://example.test/manifest.json\n"));
    const LauncherConfig config = LauncherConfig::load(system, {});
    EXPECT_FALSE(config.updatesEnabled);
    EXPECT_FALSE(config.allowUserRuntime);
    EXPECT_TRUE(config.requireSignature);
    EXPECT_EQ(config.minimumCheckIntervalHours, 12);
    EXPECT_EQ(config.maxUserVersions, 2);
    EXPECT_FALSE(config.warnings.isEmpty());
}

TEST(LauncherXdgPaths, UsesHomeDefaults) {
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("HOME"), QStringLiteral("/tmp/genesys-home"));
    const RuntimePaths paths(environment, QStringLiteral("/system"), QStringLiteral("/etc/test"), QStringLiteral("/keyring"));
    EXPECT_EQ(paths.configRoot(), QStringLiteral("/tmp/genesys-home/.config/genesys"));
    EXPECT_EQ(paths.cacheRoot(), QStringLiteral("/tmp/genesys-home/.cache/genesys"));
    EXPECT_EQ(paths.dataRoot(), QStringLiteral("/tmp/genesys-home/.local/share/genesys"));
}

TEST(LauncherXdgPaths, HonorsAbsoluteXdgOverrides) {
    QProcessEnvironment environment;
    environment.insert(QStringLiteral("HOME"), QStringLiteral("/home/test"));
    environment.insert(QStringLiteral("XDG_CONFIG_HOME"), QStringLiteral("/cfg"));
    environment.insert(QStringLiteral("XDG_CACHE_HOME"), QStringLiteral("/cache"));
    environment.insert(QStringLiteral("XDG_DATA_HOME"), QStringLiteral("/data"));
    const RuntimePaths paths(environment, QStringLiteral("/system"), QStringLiteral("/etc/test"), QStringLiteral("/keyring"));
    EXPECT_EQ(paths.userConfigPath(), QStringLiteral("/cfg/genesys/update.conf"));
    EXPECT_EQ(paths.downloadsRoot(), QStringLiteral("/cache/genesys/downloads"));
    EXPECT_EQ(paths.appsRoot(), QStringLiteral("/data/genesys/apps"));
}

TEST(LauncherXdgPaths, DisablesUserRuntimeWhenHomeIsAbsent) {
    QProcessEnvironment environment;
    const RuntimePaths paths(environment, QStringLiteral("/system"), QStringLiteral("/etc/test"), QStringLiteral("/keyring"));
    EXPECT_FALSE(paths.hasUsableHome());
    EXPECT_TRUE(paths.dataRoot().isEmpty());
    EXPECT_TRUE(paths.currentRuntimeLink().isEmpty());
}

TEST(LauncherManifest, AcceptsValidManifest) {
    const QByteArray json = remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, QLatin1Char('a')), 123);
    const ManifestParseResult result = UpdateManifestParser::parse(
        json, QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
        Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")),
        Version::parse(QStringLiteral("2026.1.0")));
    ASSERT_TRUE(result.ok) << result.error.toStdString();
    EXPECT_EQ(result.manifest.version, Version::parse(QStringLiteral("2026.2.0")));
    EXPECT_EQ(result.manifest.platform.applications, kCompleteApplications);
}

TEST(LauncherManifest, RejectsWrongProductSchemaAndChannel) {
    QJsonObject base = QJsonDocument::fromJson(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 10)).object();
    base.insert(QStringLiteral("product"), QStringLiteral("other"));
    EXPECT_FALSE(UpdateManifestParser::parse(QJsonDocument(base).toJson(), QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                              Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0"))).ok);
    base = QJsonDocument::fromJson(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 10)).object();
    base.insert(QStringLiteral("schema_version"), 2);
    EXPECT_FALSE(UpdateManifestParser::parse(QJsonDocument(base).toJson(), QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                              Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0"))).ok);
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 10, kCompleteApplications, QStringLiteral("testing")),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                              Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0"))).ok);
}

TEST(LauncherManifest, RejectsMissingPlatformBadShaSizeAndEmptyApplications) {
    const Version v = Version::parse(QStringLiteral("2026.1.0"));
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 10),
                                              QStringLiteral("stable"), QStringLiteral("debian-12-x86_64"), v, v, v).ok);
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.2.0"), QStringLiteral("bad"), 10),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"), v, v, v).ok);
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 0),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"), v, v, v).ok);
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, 'a'), 10, {}),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"), v, v, v).ok);
}

TEST(LauncherManifest, RejectsDowngradeAndMinimumVersionMismatch) {
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.1.0"), QString(64, 'a'), 10),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                              Version::parse(QStringLiteral("2026.2.0")), Version::parse(QStringLiteral("2026.2.0")), Version::parse(QStringLiteral("2026.2.0"))).ok);
    EXPECT_FALSE(UpdateManifestParser::parse(remoteManifestJson(QStringLiteral("2026.3.0"), QString(64, 'a'), 10, kCompleteApplications,
                                                                   QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                                                   QStringLiteral("2026.4.0"), QStringLiteral("2026.1.0")),
                                              QStringLiteral("stable"), QStringLiteral("ubuntu-24.04-x86_64"),
                                              Version::parse(QStringLiteral("2026.2.0")), Version::parse(QStringLiteral("2026.2.0")), Version::parse(QStringLiteral("2026.2.0"))).ok);
}

TEST(LauncherRuntimeSelection, PrefersValidUserRuntime) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    const QString userRoot = addUserRuntime(paths, QStringLiteral("2026.2.0"));
    LauncherConfig config;
    const RuntimeSelection selection = RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config);
    ASSERT_TRUE(selection.ok) << selection.reason.toStdString();
    EXPECT_EQ(selection.source, RuntimeSource::User);
    EXPECT_TRUE(selection.primaryExecutable.startsWith(userRoot));
    EXPECT_FALSE(selection.fallbackExecutable.isEmpty());
}

TEST(LauncherRuntimeSelection, MissingOrBrokenCurrentFallsBackToSystem) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    LauncherConfig config;
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);
    QDir().mkpath(paths.dataRoot());
    ASSERT_TRUE(QFile::link(QDir(paths.appsRoot()).filePath(QStringLiteral("missing")), paths.currentRuntimeLink()));
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);
}

TEST(LauncherRuntimeSelection, MissingOrNonExecutableUserTargetFallsBack) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    const QString root = addUserRuntime(paths, QStringLiteral("2026.2.0"));
    ASSERT_TRUE(QFile::remove(QDir(root).filePath(QStringLiteral("bin/genesys-shell"))));
    LauncherConfig config;
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);

    ASSERT_TRUE(writeFile(QDir(root).filePath(QStringLiteral("bin/genesys-shell")), "not executable", false));
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);
}

TEST(LauncherRuntimeSelection, InvalidInternalManifestFallsBack) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    const QString root = addUserRuntime(paths, QStringLiteral("2026.2.0"));
    ASSERT_TRUE(writeFile(QDir(root).filePath(QStringLiteral("MANIFEST.json")), "{}"));
    LauncherConfig config;
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);
}

TEST(LauncherRuntimeSelection, HonorsFallbackAndAdministrativeUserRuntimeDisable) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    addUserRuntime(paths, QStringLiteral("2026.2.0"));
    LauncherConfig config;
    config.allowUserRuntime = false;
    EXPECT_EQ(RuntimeSelector(paths).select(QStringLiteral("genesys-shell"), config).source, RuntimeSource::System);
    config.fallbackToSystem = false;
    EXPECT_FALSE(RuntimeSelector(paths).select(QStringLiteral("genesys-missing"), config).ok);
}

TEST(LauncherRuntimeSelection, HandlesWorkerLegacyWebAliasAndMcp) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addUserRuntime(paths, QStringLiteral("2026.2.0"),
                   {QStringLiteral("genesys-worker"), QStringLiteral("genesys-mcp")});
    LauncherConfig config;
    const RuntimeSelection web = RuntimeSelector(paths).select(QStringLiteral("genesys-web"), config);
    ASSERT_TRUE(web.ok);
    EXPECT_EQ(web.resolvedApplication, QStringLiteral("genesys-worker"));
    EXPECT_TRUE(RuntimeSelector(paths).select(QStringLiteral("genesys-mcp"), config).ok);
}

TEST(LauncherIntegrity, AcceptsCorrectShaAndRejectsMismatch) {
    QTemporaryDir temp;
    const QString path = QDir(temp.path()).filePath(QStringLiteral("bundle"));
    ASSERT_TRUE(writeFile(path, "genesys-runtime-fixture"));
    const QString digest = QString::fromLatin1(QCryptographicHash::hash("genesys-runtime-fixture", QCryptographicHash::Sha256).toHex());
    EXPECT_TRUE(UpdateVerifier::verifySha256(path, digest).ok);
    EXPECT_FALSE(UpdateVerifier::verifySha256(path, QString(64, QLatin1Char('0'))).ok);
}

TEST(LauncherIntegrity, GpgvVerifierFailsClosedWithoutKeyring) {
    GpgvSignatureVerifier verifier(QStringLiteral("/definitely/not/a/genesys/keyring"));
    const VerificationResult result = verifier.verify(QStringLiteral("/missing/data"), QStringLiteral("/missing/signature"));
    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.error.isEmpty());
}

TEST(LauncherArchiveSecurity, RejectsTraversalAbsoluteAndLinks) {
    EXPECT_TRUE(TarZstdArchiveExtractor::validatePathListing("bin/genesys-gui\nVERSION\nMANIFEST.json\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validatePathListing("../escape\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validatePathListing("bin/../escape\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validatePathListing("/absolute\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validatePathListing("bin\\evil\n").ok);
    EXPECT_TRUE(TarZstdArchiveExtractor::validateVerboseListing("-rwxr-xr-x 0/0 10 2026-08-20 12:00 bin/genesys-gui\ndrwxr-xr-x 0/0 0 2026-08-20 12:00 bin/\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validateVerboseListing("lrwxrwxrwx 0/0 0 2026-08-20 12:00 link -> /etc/passwd\n").ok);
    EXPECT_FALSE(TarZstdArchiveExtractor::validateVerboseListing("-rwsr-xr-x 0/0 10 2026-08-20 12:00 bin/app\n").ok);
}

TEST(LauncherInstallation, PartialInstallActivatesAtomically) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    FakeExtractor extractor;
    UserRuntimeManager manager(paths, extractor);
    QString lockError;
    ASSERT_TRUE(manager.beginUpdate(&lockError)) << lockError.toStdString();
    const UpdateManifest manifest = makeManifest(QStringLiteral("2026.2.0"));
    const RuntimeInstallResult installed = manager.installAndActivate(QStringLiteral("fixture.bundle"), manifest, 2);
    ASSERT_TRUE(installed.ok) << installed.error.toStdString();
    EXPECT_TRUE(QFileInfo(paths.currentRuntimeLink()).isSymLink());
    EXPECT_EQ(QFileInfo(paths.currentRuntimeLink()).canonicalFilePath(), QFileInfo(installed.runtimeRoot).canonicalFilePath());
    EXPECT_FALSE(QFileInfo(QDir(paths.appsRoot()).filePath(QStringLiteral("2026.2.0.partial"))).exists());
}

TEST(LauncherInstallation, ExtractionFailureNeverChangesCurrent) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    const QString previous = addUserRuntime(paths, QStringLiteral("2026.1.0"));
    FakeExtractor extractor;
    extractor.fail = true;
    extractor.version = QStringLiteral("2026.2.0");
    UserRuntimeManager manager(paths, extractor);
    ASSERT_TRUE(manager.beginUpdate());
    const RuntimeInstallResult installed = manager.installAndActivate(QStringLiteral("fixture.bundle"), makeManifest(QStringLiteral("2026.2.0")), 2);
    EXPECT_FALSE(installed.ok);
    EXPECT_EQ(QFileInfo(paths.currentRuntimeLink()).canonicalFilePath(), QFileInfo(previous).canonicalFilePath());
    EXPECT_FALSE(QFileInfo(QDir(paths.appsRoot()).filePath(QStringLiteral("2026.2.0.partial"))).exists());
}

TEST(LauncherInstallation, ActivationFailurePreservesExistingCurrentEntry) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    QDir().mkpath(paths.dataRoot());
    ASSERT_TRUE(writeFile(paths.currentRuntimeLink(), "do-not-replace"));
    FakeExtractor extractor;
    UserRuntimeManager manager(paths, extractor);
    ASSERT_TRUE(manager.beginUpdate());
    const RuntimeInstallResult installed = manager.installAndActivate(QStringLiteral("fixture.bundle"), makeManifest(QStringLiteral("2026.2.0")), 2);
    EXPECT_FALSE(installed.ok);
    QFile current(paths.currentRuntimeLink());
    ASSERT_TRUE(current.open(QIODevice::ReadOnly));
    EXPECT_EQ(current.readAll(), QByteArray("do-not-replace"));
}

TEST(LauncherRetention, PreservesActiveAndImmediateRollback) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addUserRuntime(paths, QStringLiteral("2026.0.0"), kCompleteApplications, false);
    const QString rollback = addUserRuntime(paths, QStringLiteral("2026.1.0"));
    FakeExtractor extractor;
    extractor.version = QStringLiteral("2026.2.0");
    UserRuntimeManager manager(paths, extractor);
    ASSERT_TRUE(manager.beginUpdate());
    const RuntimeInstallResult installed = manager.installAndActivate(QStringLiteral("fixture.bundle"), makeManifest(QStringLiteral("2026.2.0")), 1);
    ASSERT_TRUE(installed.ok) << installed.error.toStdString();
    EXPECT_TRUE(QFileInfo(installed.runtimeRoot).isDir());
    EXPECT_TRUE(QFileInfo(rollback).isDir());
    EXPECT_FALSE(QFileInfo(QDir(paths.appsRoot()).filePath(QStringLiteral("2026.0.0"))).exists());
}

TEST(LauncherDispatcher, PreservesArgumentsAndSelectsSystemFallbackWithoutNetworkLayer) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-shell"));
    LauncherConfig config;
    RuntimeSelector selector(paths);
    FakeProcessLauncher process;
    DispatchController dispatcher(selector, process);
    const QStringList arguments = {QStringLiteral("--model"), QStringLiteral("a model.gen"), QStringLiteral("--flag=x")};
    QString error;
    EXPECT_EQ(dispatcher.dispatch(QStringLiteral("genesys-shell"), arguments, config, &error), 0);
    ASSERT_EQ(process.calls, 1);
    EXPECT_EQ(process.arguments, arguments);
    EXPECT_EQ(process.selection.source, RuntimeSource::System);
}

TEST(LauncherUpdateChecker, RespectsIntervalAndDoesNotRepeatTransportCall) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    const QByteArray bundle = "fixture";
    const QString sha = QString::fromLatin1(QCryptographicHash::hash(bundle, QCryptographicHash::Sha256).toHex());
    FakeNetworkTransport transport;
    transport.responses[QStringLiteral("https://example.test/update-manifest.json")] = remoteManifestJson(QStringLiteral("2026.2.0"), sha, bundle.size());
    LauncherConfig config = enabledConfig();
    config.minimumCheckIntervalHours = 12;
    const QDateTime now = QDateTime::fromString(QStringLiteral("2026-08-20T12:00:00Z"), Qt::ISODate);
    UpdateChecker first(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                        Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")),
                        transport, nullptr, [now] { return now; });
    EXPECT_EQ(first.check().status, UpdateCheckStatus::UpdateAvailable);
    EXPECT_EQ(transport.calls, 1);
    UpdateChecker second(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                         Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")),
                         transport, nullptr, [now] { return now.addSecs(60); });
    EXPECT_EQ(second.check().status, UpdateCheckStatus::RateLimited);
    EXPECT_EQ(transport.calls, 1);
}

TEST(LauncherIntegration, LocalFakeCycleDownloadsVerifiesInstallsActivatesAndSelects) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    addSystemExecutable(paths, QStringLiteral("genesys-gui"));
    const QByteArray bundle = "tiny-local-runtime-bundle";
    const QString sha = QString::fromLatin1(QCryptographicHash::hash(bundle, QCryptographicHash::Sha256).toHex());
    FakeNetworkTransport transport;
    transport.responses[QStringLiteral("https://example.test/update-manifest.json")] = remoteManifestJson(QStringLiteral("2026.2.0"), sha, bundle.size());
    transport.responses[QStringLiteral("https://example.test/runtime.tar.zst")] = bundle;
    transport.responses[QStringLiteral("https://example.test/runtime.tar.zst.sig")] = "fixture-signature";

    LauncherConfig config = enabledConfig();
    UpdateChecker checker(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                          Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), transport);
    UpdateDownloader downloader(paths, transport);
    FakeSignatureVerifier signature;
    FakeExtractor extractor;
    extractor.version = QStringLiteral("2026.2.0");
    UserRuntimeManager runtimeManager(paths, extractor);
    FakeUpdateUi ui;
    LauncherController controller(config, checker, downloader, &signature, runtimeManager, ui);

    const LauncherUpdateResult result = controller.checkAndMaybeUpdate(Version::parse(QStringLiteral("2026.1.0")), true, true, false);
    ASSERT_TRUE(result.installed) << result.message.toStdString();
    EXPECT_EQ(result.installedVersion, Version::parse(QStringLiteral("2026.2.0")));
    EXPECT_EQ(signature.calls, 1);
    EXPECT_EQ(extractor.calls, 1);
    EXPECT_EQ(transport.calls, 3);
    const RuntimeSelection selection = RuntimeSelector(paths).select(QStringLiteral("genesys-gui"), config);
    ASSERT_TRUE(selection.ok);
    EXPECT_EQ(selection.source, RuntimeSource::User);
}

TEST(LauncherIntegration, SignatureFailureAndUnavailableVerifierPreserveCurrent) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    const QString previous = addUserRuntime(paths, QStringLiteral("2026.1.0"));
    const QByteArray bundle = "tiny-local-runtime-bundle";
    const QString sha = QString::fromLatin1(QCryptographicHash::hash(bundle, QCryptographicHash::Sha256).toHex());

    auto makeTransport = [&]() {
        FakeNetworkTransport transport;
        transport.responses[QStringLiteral("https://example.test/update-manifest.json")] = remoteManifestJson(QStringLiteral("2026.2.0"), sha, bundle.size());
        transport.responses[QStringLiteral("https://example.test/runtime.tar.zst")] = bundle;
        transport.responses[QStringLiteral("https://example.test/runtime.tar.zst.sig")] = "fixture-signature";
        return transport;
    };

    LauncherConfig config = enabledConfig();
    FakeExtractor extractor;
    extractor.version = QStringLiteral("2026.2.0");
    FakeUpdateUi ui;

    FakeNetworkTransport rejectedTransport = makeTransport();
    UpdateChecker rejectedChecker(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                                  Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), rejectedTransport);
    UpdateDownloader rejectedDownloader(paths, rejectedTransport);
    FakeSignatureVerifier rejectedSignature;
    rejectedSignature.accept = false;
    UserRuntimeManager rejectedManager(paths, extractor);
    LauncherController rejectedController(config, rejectedChecker, rejectedDownloader, &rejectedSignature, rejectedManager, ui);
    EXPECT_FALSE(rejectedController.checkAndMaybeUpdate(Version::parse(QStringLiteral("2026.1.0")), true, true, false).installed);
    EXPECT_EQ(QFileInfo(paths.currentRuntimeLink()).canonicalFilePath(), QFileInfo(previous).canonicalFilePath());

    FakeNetworkTransport unavailableTransport = makeTransport();
    UpdateChecker unavailableChecker(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                                     Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), unavailableTransport);
    UpdateDownloader unavailableDownloader(paths, unavailableTransport);
    UserRuntimeManager unavailableManager(paths, extractor);
    LauncherController unavailableController(config, unavailableChecker, unavailableDownloader, nullptr, unavailableManager, ui);
    EXPECT_FALSE(unavailableController.checkAndMaybeUpdate(Version::parse(QStringLiteral("2026.1.0")), true, true, false).installed);
    EXPECT_EQ(QFileInfo(paths.currentRuntimeLink()).canonicalFilePath(), QFileInfo(previous).canonicalFilePath());
}

TEST(LauncherIntegration, ChecksumMismatchPreservesCurrentAndNeverExtracts) {
    QTemporaryDir temp;
    const RuntimePaths paths = makePaths(temp);
    const QString previous = addUserRuntime(paths, QStringLiteral("2026.1.0"));
    const QByteArray bundle = "tampered-bundle";
    FakeNetworkTransport transport;
    transport.responses[QStringLiteral("https://example.test/update-manifest.json")] = remoteManifestJson(QStringLiteral("2026.2.0"), QString(64, QLatin1Char('0')), bundle.size());
    transport.responses[QStringLiteral("https://example.test/runtime.tar.zst")] = bundle;
    transport.responses[QStringLiteral("https://example.test/runtime.tar.zst.sig")] = "fixture-signature";
    LauncherConfig config = enabledConfig();
    UpdateChecker checker(paths, config, fixturePlatform(), Version::parse(QStringLiteral("2026.1.0")),
                          Version::parse(QStringLiteral("2026.1.0")), Version::parse(QStringLiteral("2026.1.0")), transport);
    UpdateDownloader downloader(paths, transport);
    FakeSignatureVerifier signature;
    FakeExtractor extractor;
    extractor.version = QStringLiteral("2026.2.0");
    UserRuntimeManager manager(paths, extractor);
    FakeUpdateUi ui;
    LauncherController controller(config, checker, downloader, &signature, manager, ui);
    EXPECT_FALSE(controller.checkAndMaybeUpdate(Version::parse(QStringLiteral("2026.1.0")), true, true, false).installed);
    EXPECT_EQ(extractor.calls, 0);
    EXPECT_EQ(signature.calls, 0);
    EXPECT_EQ(QFileInfo(paths.currentRuntimeLink()).canonicalFilePath(), QFileInfo(previous).canonicalFilePath());
}
