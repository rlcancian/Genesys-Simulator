#pragma once

#include "RuntimeSelector.h"

#include <QString>
#include <QStringList>

namespace genesys::launcher {

class LauncherLogger;

class IProcessLauncher {
public:
    virtual ~IProcessLauncher() = default;

    [[nodiscard]] virtual int launchReplacingProcess(const RuntimeSelection& selection,
                                                     const QStringList& arguments,
                                                     QString* error = nullptr) = 0;
};

class ProcessLauncher final : public IProcessLauncher {
public:
    explicit ProcessLauncher(LauncherLogger* logger = nullptr);

    [[nodiscard]] int launchReplacingProcess(const RuntimeSelection& selection,
                                             const QStringList& arguments,
                                             QString* error = nullptr) override;

private:
    LauncherLogger* logger_ = nullptr;
};

} // namespace genesys::launcher
