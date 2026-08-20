#pragma once

#include "LauncherConfig.h"
#include "UpdateChecker.h"
#include "UpdateDialog.h"
#include "UpdateDownloader.h"
#include "UpdateVerifier.h"
#include "UserRuntimeManager.h"

#include <QString>

namespace genesys::launcher {

class LauncherLogger;

struct LauncherUpdateResult {
    bool checkCompleted = false;
    bool updateAvailable = false;
    bool installed = false;
    Version installedVersion;
    QString message;
};

class LauncherController {
public:
    LauncherController(LauncherConfig config,
                       UpdateChecker& checker,
                       UpdateDownloader& downloader,
                       ISignatureVerifier* signatureVerifier,
                       UserRuntimeManager& runtimeManager,
                       IUpdateUi& ui,
                       LauncherLogger* logger = nullptr);

    [[nodiscard]] LauncherUpdateResult checkAndMaybeUpdate(const Version& currentVersion,
                                                           bool interactive,
                                                           bool forceCheck,
                                                           bool checkOnly);

private:
    LauncherUpdateResult failUpdate(QString message, bool interactive);
    void log(const QString& event, const QString& detail = {}) const;

    LauncherConfig config_;
    UpdateChecker& checker_;
    UpdateDownloader& downloader_;
    ISignatureVerifier* signatureVerifier_ = nullptr;
    UserRuntimeManager& runtimeManager_;
    IUpdateUi& ui_;
    LauncherLogger* logger_ = nullptr;
};

} // namespace genesys::launcher
