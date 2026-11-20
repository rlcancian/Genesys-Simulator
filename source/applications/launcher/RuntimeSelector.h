#pragma once

#include "LauncherConfig.h"
#include "RuntimePaths.h"
#include "Version.h"

#include <QString>
#include <QStringList>

namespace genesys::launcher {

class LauncherLogger;

enum class RuntimeSource {
    None,
    User,
    System
};

struct RuntimeSelection {
    bool ok = false;
    QString requestedApplication;
    QString resolvedApplication;
    QString primaryExecutable;
    QString fallbackExecutable;
    RuntimeSource source = RuntimeSource::None;
    QString reason;
};

class RuntimeSelector {
public:
    RuntimeSelector(const RuntimePaths& paths, LauncherLogger* logger = nullptr);

    [[nodiscard]] RuntimeSelection select(const QString& requestedApplication,
                                          const LauncherConfig& config) const;
    [[nodiscard]] Version currentUserVersion() const;

    [[nodiscard]] static QStringList applicationCandidates(const QString& requestedApplication);
    [[nodiscard]] static bool isSupportedApplicationName(const QString& application);

private:
    struct UserRuntime {
        bool valid = false;
        QString root;
        Version version;
        QStringList declaredApplications;
        QString reason;
    };

    [[nodiscard]] UserRuntime inspectCurrentRuntime() const;
    [[nodiscard]] QString findUserExecutable(const UserRuntime& runtime,
                                             const QString& requestedApplication,
                                             QString* resolvedApplication) const;
    [[nodiscard]] QString findSystemExecutable(const QString& requestedApplication,
                                               QString* resolvedApplication) const;
    void log(const QString& event, const QString& detail) const;

    RuntimePaths paths_;
    LauncherLogger* logger_ = nullptr;
};

} // namespace genesys::launcher
