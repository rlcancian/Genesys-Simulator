#include "RuntimeSelector.h"

#include "LauncherLogger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace genesys::launcher {
namespace {

bool isPathWithin(const QString& root, const QString& candidate) {
    if (root.isEmpty() || candidate.isEmpty()) {
        return false;
    }
    const QString cleanRoot = QDir::cleanPath(root);
    const QString cleanCandidate = QDir::cleanPath(candidate);
    return cleanCandidate == cleanRoot || cleanCandidate.startsWith(cleanRoot + QLatin1Char('/'));
}

QString readTrimmedFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

} // namespace

RuntimeSelector::RuntimeSelector(const RuntimePaths& paths, LauncherLogger* logger)
    : paths_(paths), logger_(logger) {}

bool RuntimeSelector::isSupportedApplicationName(const QString& application) {
    static const QRegularExpression valid(QStringLiteral("^genesys-[a-z0-9][a-z0-9-]*$"));
    return valid.match(application).hasMatch();
}

QStringList RuntimeSelector::applicationCandidates(const QString& requestedApplication) {
    if (requestedApplication == QStringLiteral("genesys-web")) {
        return {QStringLiteral("genesys-web"), QStringLiteral("genesys-worker")};
    }
    return {requestedApplication};
}

RuntimeSelector::UserRuntime RuntimeSelector::inspectCurrentRuntime() const {
    UserRuntime result;
    if (!paths_.hasUsableHome()) {
        result.reason = QStringLiteral("HOME is unavailable; user runtime disabled");
        return result;
    }

    const QString appsRoot = paths_.appsRoot();
    const QString current = paths_.currentRuntimeLink();
    const QFileInfo currentInfo(current);
    if (!currentInfo.exists() && !currentInfo.isSymLink()) {
        result.reason = QStringLiteral("current runtime link is absent");
        return result;
    }
    if (!currentInfo.isSymLink()) {
        result.reason = QStringLiteral("current runtime entry is not a symbolic link");
        return result;
    }

    const QString canonicalApps = QFileInfo(appsRoot).canonicalFilePath();
    const QString canonicalRuntime = currentInfo.canonicalFilePath();
    if (canonicalApps.isEmpty() || canonicalRuntime.isEmpty() || !isPathWithin(canonicalApps, canonicalRuntime)) {
        result.reason = QStringLiteral("current runtime resolves outside the allowed apps root or is broken");
        return result;
    }

    QDir parent(canonicalRuntime);
    if (!parent.cdUp() || QDir::cleanPath(parent.canonicalPath()) != QDir::cleanPath(canonicalApps)) {
        result.reason = QStringLiteral("current runtime is not a direct child of the apps root");
        return result;
    }

    const Version version = Version::parse(readTrimmedFile(QDir(canonicalRuntime).filePath(QStringLiteral("VERSION"))));
    if (!version.isValid()) {
        result.reason = QStringLiteral("runtime VERSION is missing or invalid");
        return result;
    }

    QFile manifestFile(QDir(canonicalRuntime).filePath(QStringLiteral("MANIFEST.json")));
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        result.reason = QStringLiteral("runtime MANIFEST.json is missing");
        return result;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.reason = QStringLiteral("runtime MANIFEST.json is invalid JSON");
        return result;
    }
    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("schema_version")).toInt(-1) != 1 ||
        object.value(QStringLiteral("product")).toString() != QStringLiteral("genesys-simulator") ||
        Version::parse(object.value(QStringLiteral("version")).toString()) != version ||
        !object.value(QStringLiteral("applications")).isArray()) {
        result.reason = QStringLiteral("runtime MANIFEST.json contract mismatch");
        return result;
    }

    QStringList applications;
    const QJsonArray array = object.value(QStringLiteral("applications")).toArray();
    for (const QJsonValue& value : array) {
        if (!value.isString() || !isSupportedApplicationName(value.toString())) {
            result.reason = QStringLiteral("runtime MANIFEST.json contains an invalid application name");
            return result;
        }
        const QString app = value.toString();
        if (applications.contains(app)) {
            result.reason = QStringLiteral("runtime MANIFEST.json contains duplicate applications");
            return result;
        }
        applications.push_back(app);
    }
    if (applications.isEmpty()) {
        result.reason = QStringLiteral("runtime MANIFEST.json declares no applications");
        return result;
    }

    result.valid = true;
    result.root = canonicalRuntime;
    result.version = version;
    result.declaredApplications = applications;
    return result;
}

QString RuntimeSelector::findUserExecutable(const UserRuntime& runtime,
                                            const QString& requestedApplication,
                                            QString* resolvedApplication) const {
    if (!runtime.valid) {
        return {};
    }

    const QString canonicalBin = QFileInfo(QDir(runtime.root).filePath(QStringLiteral("bin"))).canonicalFilePath();
    if (canonicalBin.isEmpty() || !isPathWithin(runtime.root, canonicalBin)) {
        return {};
    }

    for (const QString& candidate : applicationCandidates(requestedApplication)) {
        if (!runtime.declaredApplications.contains(candidate)) {
            continue;
        }
        const QString path = QDir(canonicalBin).filePath(candidate);
        const QFileInfo info(path);
        const QString canonicalExecutable = info.canonicalFilePath();
        if (!info.exists() || info.isSymLink() || !info.isFile() || !info.isExecutable() ||
            canonicalExecutable.isEmpty() || !isPathWithin(canonicalBin, canonicalExecutable)) {
            continue;
        }
        if (resolvedApplication) {
            *resolvedApplication = candidate;
        }
        return canonicalExecutable;
    }
    return {};
}

QString RuntimeSelector::findSystemExecutable(const QString& requestedApplication,
                                              QString* resolvedApplication) const {
    for (const QString& candidate : applicationCandidates(requestedApplication)) {
        const QString path = QDir(paths_.systemBinRoot()).filePath(candidate);
        const QFileInfo info(path);
        if (info.exists() && info.isFile() && info.isExecutable()) {
            if (resolvedApplication) {
                *resolvedApplication = candidate;
            }
            return info.absoluteFilePath();
        }
    }
    return {};
}

RuntimeSelection RuntimeSelector::select(const QString& requestedApplication,
                                         const LauncherConfig& config) const {
    RuntimeSelection selection;
    selection.requestedApplication = requestedApplication;
    if (!isSupportedApplicationName(requestedApplication)) {
        selection.reason = QStringLiteral("invalid application name");
        log(QStringLiteral("selection-rejected"), selection.reason + QStringLiteral(" app=") + requestedApplication);
        return selection;
    }

    QString systemResolved;
    const QString systemExecutable = findSystemExecutable(requestedApplication, &systemResolved);

    if (config.allowUserRuntime) {
        const UserRuntime userRuntime = inspectCurrentRuntime();
        if (userRuntime.valid) {
            QString userResolved;
            const QString userExecutable = findUserExecutable(userRuntime, requestedApplication, &userResolved);
            if (!userExecutable.isEmpty()) {
                selection.ok = true;
                selection.resolvedApplication = userResolved;
                selection.primaryExecutable = userExecutable;
                selection.fallbackExecutable = config.fallbackToSystem ? systemExecutable : QString{};
                selection.source = RuntimeSource::User;
                selection.reason = QStringLiteral("valid active user runtime %1").arg(userRuntime.version.toString());
                log(QStringLiteral("runtime-selected"), QStringLiteral("source=user app=%1 path=%2").arg(userResolved, userExecutable));
                return selection;
            }
            log(QStringLiteral("user-runtime-fallback"), QStringLiteral("requested app unavailable or invalid in active runtime: %1").arg(requestedApplication));
        } else {
            log(QStringLiteral("user-runtime-fallback"), userRuntime.reason);
        }
    } else {
        log(QStringLiteral("user-runtime-disabled"), QStringLiteral("administrative/user policy disabled per-user runtime"));
    }

    if (config.fallbackToSystem && !systemExecutable.isEmpty()) {
        selection.ok = true;
        selection.resolvedApplication = systemResolved;
        selection.primaryExecutable = systemExecutable;
        selection.source = RuntimeSource::System;
        selection.reason = QStringLiteral("system fallback selected");
        log(QStringLiteral("runtime-selected"), QStringLiteral("source=system app=%1 path=%2").arg(systemResolved, systemExecutable));
        return selection;
    }

    selection.reason = config.fallbackToSystem
        ? QStringLiteral("application is unavailable in both active user runtime and system installation")
        : QStringLiteral("no valid user application and system fallback is disabled");
    log(QStringLiteral("selection-failed"), QStringLiteral("app=%1 reason=%2").arg(requestedApplication, selection.reason));
    return selection;
}

Version RuntimeSelector::currentUserVersion() const {
    const UserRuntime runtime = inspectCurrentRuntime();
    return runtime.valid ? runtime.version : Version{};
}

void RuntimeSelector::log(const QString& event, const QString& detail) const {
    if (logger_) {
        logger_->log(event, detail);
    }
}

} // namespace genesys::launcher
