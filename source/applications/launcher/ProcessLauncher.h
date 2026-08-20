#pragma once

#include "RuntimeSelector.h"

#include <QString>
#include <QStringList>

namespace genesys::launcher {

class LauncherLogger;

class ProcessLauncher {
public:
    explicit ProcessLauncher(LauncherLogger* logger = nullptr);

    [[nodiscard]] int launchReplacingProcess(const RuntimeSelection& selection,
                                             const QStringList& arguments,
                                             QString* error = nullptr) const;

private:
    LauncherLogger* logger_ = nullptr;
};

} // namespace genesys::launcher
