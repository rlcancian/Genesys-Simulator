#pragma once

#include "LauncherConfig.h"
#include "ProcessLauncher.h"
#include "RuntimeSelector.h"

#include <QString>
#include <QStringList>

namespace genesys::launcher {

class DispatchController {
public:
    DispatchController(const RuntimeSelector& selector,
                       IProcessLauncher& processLauncher);

    [[nodiscard]] int dispatch(const QString& application,
                               const QStringList& arguments,
                               const LauncherConfig& config,
                               QString* error = nullptr) const;

private:
    const RuntimeSelector& selector_;
    IProcessLauncher& processLauncher_;
};

} // namespace genesys::launcher
