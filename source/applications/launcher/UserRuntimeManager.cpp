#include "UserRuntimeManager.h"

#include "LauncherLogger.h"
#include "RuntimeSelector.h"
#include "Version.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <QVector>

#include <algorithm>
#include <cerrno>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace genesys::launcher {
namespace {

bool pathWithin(const QString& root, const QString& candidate) {
    const QString cleanRoot = QDir::cleanPath(root);
    const QString cleanCandidate = QDir::cleanPath(candidate);
    return !cleanRoot.isEmpty() && !cleanCandidate.isEmpty() &&
           (cleanCandidate == cleanRoot || cleanCandidate.startsWith(cleanRoot + QLatin1Char('/')));
}

QString readTextFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

QSet<QString> applicationSet(const QStringList& applications) {
    QSet<QString> result;
    for (const QString& application : applications) {
        result.insert(application);
    }
    return result;
}

} // namespace

UserRuntimeManager::UserRuntimeManager(RuntimePaths paths,
                                       IArchiveExtractor& extractor,
                                       LauncherLogger* logger)
    : paths_(std::move(paths)), extractor_(extractor), logger_(logger) {}

UserRuntimeManager::~UserRuntimeManager() {
    endUpdate();
}

bool UserRuntimeManager::beginUpdate(QString* error, const int lockTimeoutMs) {
    if (hasUpdateLock()) {
        return true;
    }
    if (!paths_.hasUsableHome() || paths_.dataRoot().isEmpty()) {
        if (error) {
            *error = QStringLiteral("A usable HOME/XDG data directory is required for user runtime updates");
        }
        return false;
    }
    if (!QDir().mkpath(paths_.dataRoot())) {
        if (error) {
            *error = QStringLiteral("Unable to create the GenESyS user data directory");
        }
        return false;
    }

    lock_ = std::make_unique<QLockFile>(paths_.updateLockPath());
    lock_->setStaleLockTime(30 * 60 * 1000);
    if (!lock_->tryLock(lockTimeoutMs)) {
        if (error) {
            *error = QStringLiteral("Another GenESyS runtime update is already in progress for this user");
        }
        lock_.reset();
        return false;
    }
    log(QStringLiteral("update-lock-acquired"));
    return true;
}

void UserRuntimeManager::endUpdate() {
    if (lock_) {
        if (lock_->isLocked()) {
            lock_->unlock();
        }
        lock_.reset();
        log(QStringLiteral("update-lock-released"));
    }
}

bool UserRuntimeManager::validateRuntime(const QString& runtimeRoot,
                                         const UpdateManifest& manifest,
                                         QString* error) const {
    const QFileInfo rootInfo(runtimeRoot);
    const QString canonicalRoot = rootInfo.canonicalFilePath();
    if (!rootInfo.isDir() || canonicalRoot.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Extracted runtime root is missing or invalid");
        }
        return false;
    }

    const Version version = Version::parse(readTextFile(QDir(canonicalRoot).filePath(QStringLiteral("VERSION"))));
    if (!version.isValid() || version != manifest.version) {
        if (error) {
            *error = QStringLiteral("Extracted runtime VERSION does not match the remote manifest");
        }
        return false;
    }

    QFile manifestFile(QDir(canonicalRoot).filePath(QStringLiteral("MANIFEST.json")));
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QStringLiteral("Extracted runtime MANIFEST.json is missing");
        }
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(manifestFile.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error) {
            *error = QStringLiteral("Extracted runtime MANIFEST.json is invalid JSON");
        }
        return false;
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("schema_version")).toInt(-1) != 1 ||
        object.value(QStringLiteral("product")).toString() != QStringLiteral("genesys-simulator") ||
        Version::parse(object.value(QStringLiteral("version")).toString()) != manifest.version ||
        !object.value(QStringLiteral("applications")).isArray()) {
        if (error) {
            *error = QStringLiteral("Extracted runtime manifest contract does not match the expected runtime");
        }
        return false;
    }

    QStringList internalApplications;
    for (const QJsonValue& value : object.value(QStringLiteral("applications")).toArray()) {
        if (!value.isString() || !RuntimeSelector::isSupportedApplicationName(value.toString()) ||
            internalApplications.contains(value.toString())) {
            if (error) {
                *error = QStringLiteral("Extracted runtime manifest contains an invalid application list");
            }
            return false;
        }
        internalApplications.push_back(value.toString());
    }

    const QSet<QString> internalApplicationSet = applicationSet(internalApplications);
    if (internalApplicationSet != applicationSet(manifest.platform.applications)) {
        if (error) {
            *error = QStringLiteral("Extracted runtime applications do not match the remote manifest");
        }
        return false;
    }

    const QSet<QString> requiredApplications = {
        QStringLiteral("genesys-gui"),
        QStringLiteral("genesys-shell"),
        QStringLiteral("genesys-worker"),
        QStringLiteral("genesys-mcp")
    };
    for (const QString& required : requiredApplications) {
        if (!internalApplicationSet.contains(required)) {
            if (error) {
                *error = QStringLiteral("Runtime bundle is incomplete: gui, shell, worker and MCP applications are required");
            }
            return false;
        }
    }

    const QString binRoot = QDir(canonicalRoot).filePath(QStringLiteral("bin"));
    const QFileInfo binInfo(binRoot);
    const QString canonicalBin = binInfo.canonicalFilePath();
    if (!binInfo.isDir() || canonicalBin.isEmpty() || !pathWithin(canonicalRoot, canonicalBin)) {
        if (error) {
            *error = QStringLiteral("Extracted runtime bin directory is missing or invalid");
        }
        return false;
    }

    for (const QString& application : internalApplications) {
        const QFileInfo executable(QDir(canonicalBin).filePath(application));
        const QString canonicalExecutable = executable.canonicalFilePath();
        if (!executable.exists() || executable.isSymLink() || !executable.isFile() || !executable.isExecutable() ||
            canonicalExecutable.isEmpty() || !pathWithin(canonicalBin, canonicalExecutable)) {
            if (error) {
                *error = QStringLiteral("Runtime application is missing, unsafe or non-executable: %1").arg(application);
            }
            return false;
        }
    }
    return true;
}

QString UserRuntimeManager::activeRuntimeRoot() const {
    const QFileInfo current(paths_.currentRuntimeLink());
    if (!current.isSymLink()) {
        return {};
    }
    const QString canonicalApps = QFileInfo(paths_.appsRoot()).canonicalFilePath();
    const QString canonicalCurrent = current.canonicalFilePath();
    if (canonicalApps.isEmpty() || canonicalCurrent.isEmpty() || !pathWithin(canonicalApps, canonicalCurrent)) {
        return {};
    }
    return canonicalCurrent;
}

bool UserRuntimeManager::activateAtomically(const QString& runtimeRoot, QString* error) const {
    const QString appsRoot = QFileInfo(paths_.appsRoot()).canonicalFilePath();
    const QString canonicalRuntime = QFileInfo(runtimeRoot).canonicalFilePath();
    if (appsRoot.isEmpty() || canonicalRuntime.isEmpty() || !pathWithin(appsRoot, canonicalRuntime)) {
        if (error) {
            *error = QStringLiteral("Refusing to activate a runtime outside the GenESyS apps root");
        }
        return false;
    }

    const QFileInfo currentInfo(paths_.currentRuntimeLink());
    if (currentInfo.exists() && !currentInfo.isSymLink()) {
        if (error) {
            *error = QStringLiteral("Refusing to replace a non-symlink current runtime entry");
        }
        return false;
    }

#ifdef Q_OS_UNIX
    const QString dataRoot = QFileInfo(paths_.dataRoot()).canonicalFilePath();
    if (dataRoot.isEmpty()) {
        if (error) {
            *error = QStringLiteral("User data root is unavailable for atomic activation");
        }
        return false;
    }
    const QString relativeTarget = QDir(dataRoot).relativeFilePath(canonicalRuntime);
    if (relativeTarget.startsWith(QStringLiteral("../")) || relativeTarget == QStringLiteral("..")) {
        if (error) {
            *error = QStringLiteral("Runtime activation target escaped the user data root");
        }
        return false;
    }

    const QString temporaryLink = paths_.currentRuntimeLink() +
        QStringLiteral(".tmp.%1").arg(QCoreApplication::applicationPid());
    QFile::remove(temporaryLink);
    const QByteArray targetBytes = QFile::encodeName(relativeTarget);
    const QByteArray temporaryBytes = QFile::encodeName(temporaryLink);
    if (::symlink(targetBytes.constData(), temporaryBytes.constData()) != 0) {
        if (error) {
            *error = QStringLiteral("Unable to create temporary current runtime symlink (errno %1)").arg(errno);
        }
        return false;
    }

    const QByteArray currentBytes = QFile::encodeName(paths_.currentRuntimeLink());
    if (::rename(temporaryBytes.constData(), currentBytes.constData()) != 0) {
        const int savedErrno = errno;
        QFile::remove(temporaryLink);
        if (error) {
            *error = QStringLiteral("Unable to atomically activate runtime (errno %1)").arg(savedErrno);
        }
        return false;
    }
    return true;
#else
    Q_UNUSED(runtimeRoot)
    if (error) {
        *error = QStringLiteral("Atomic user-runtime activation is currently implemented for Unix platforms only");
    }
    return false;
#endif
}

RuntimeInstallResult UserRuntimeManager::installAndActivate(const QString& verifiedBundlePath,
                                                            const UpdateManifest& manifest,
                                                            const int maxUserVersions) {
    RuntimeInstallResult result;
    if (!hasUpdateLock()) {
        result.error = QStringLiteral("Runtime installation requires the per-user update lock");
        return result;
    }
    if (maxUserVersions < 1) {
        result.error = QStringLiteral("max_user_versions must be at least 1");
        return result;
    }

    const QString appsRoot = paths_.appsRoot();
    if (appsRoot.isEmpty() || !QDir().mkpath(appsRoot)) {
        result.error = QStringLiteral("Unable to create the user runtime apps directory");
        return result;
    }

    result.previousRuntimeRoot = activeRuntimeRoot();
    const QString versionName = manifest.version.toString();
    const QString finalRoot = QDir(appsRoot).filePath(versionName);
    const QString partialRoot = finalRoot + QStringLiteral(".partial");
    QDir(partialRoot).removeRecursively();

    if (QFileInfo::exists(finalRoot)) {
        QString validationError;
        if (!validateRuntime(finalRoot, manifest, &validationError)) {
            result.error = QStringLiteral("A conflicting runtime directory already exists: %1").arg(validationError);
            return result;
        }
    } else {
        if (!QDir().mkpath(partialRoot)) {
            result.error = QStringLiteral("Unable to create partial runtime directory");
            return result;
        }
        const ExtractionResult extraction = extractor_.extract(verifiedBundlePath, partialRoot);
        if (!extraction.ok) {
            QDir(partialRoot).removeRecursively();
            result.error = extraction.error;
            log(QStringLiteral("runtime-extraction-failed"), result.error);
            return result;
        }

        QString validationError;
        if (!validateRuntime(partialRoot, manifest, &validationError)) {
            QDir(partialRoot).removeRecursively();
            result.error = validationError;
            log(QStringLiteral("runtime-validation-failed"), result.error);
            return result;
        }

        QDir apps(appsRoot);
        if (!apps.rename(versionName + QStringLiteral(".partial"), versionName)) {
            QDir(partialRoot).removeRecursively();
            result.error = QStringLiteral("Unable to atomically promote partial runtime directory");
            return result;
        }
    }

    QString activationError;
    if (!activateAtomically(finalRoot, &activationError)) {
        result.error = activationError;
        log(QStringLiteral("runtime-activation-failed"), result.error);
        return result;
    }

    result.ok = true;
    result.runtimeRoot = QFileInfo(finalRoot).canonicalFilePath();
    log(QStringLiteral("runtime-activated"), QStringLiteral("version=%1 root=%2").arg(versionName, result.runtimeRoot));
    cleanupOldVersions(maxUserVersions, result.runtimeRoot, result.previousRuntimeRoot);
    return result;
}

void UserRuntimeManager::cleanupOldVersions(const int maxUserVersions,
                                            const QString& activeRuntime,
                                            const QString& rollbackRuntime) const {
    QDir apps(paths_.appsRoot());
    if (!apps.exists()) {
        return;
    }

    struct Candidate {
        Version version;
        QString path;
    };
    QVector<Candidate> candidates;
    const QFileInfoList entries = apps.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& info : entries) {
        if (info.fileName().endsWith(QStringLiteral(".partial"))) {
            continue;
        }
        const Version version = Version::parse(info.fileName());
        if (version.isValid()) {
            candidates.push_back({version, info.canonicalFilePath()});
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& lhs, const Candidate& rhs) {
        return lhs.version > rhs.version;
    });

    QSet<QString> preserved;
    preserved.insert(QDir::cleanPath(activeRuntime));
    if (!rollbackRuntime.isEmpty()) {
        preserved.insert(QDir::cleanPath(rollbackRuntime));
    }
    const int preservedCount = static_cast<int>(preserved.size());
    const int desired = std::max(maxUserVersions, preservedCount);
    int retained = preservedCount;
    for (const Candidate& candidate : candidates) {
        const QString path = QDir::cleanPath(candidate.path);
        if (preserved.contains(path)) {
            continue;
        }
        if (retained < desired) {
            preserved.insert(path);
            ++retained;
            continue;
        }
        if (!QDir(path).removeRecursively()) {
            log(QStringLiteral("runtime-retention-warning"), QStringLiteral("unable to remove old runtime %1").arg(path));
        } else {
            log(QStringLiteral("runtime-removed"), path);
        }
    }
}

void UserRuntimeManager::log(const QString& event, const QString& detail) const {
    if (logger_) {
        logger_->log(event, detail);
    }
}

} // namespace genesys::launcher
