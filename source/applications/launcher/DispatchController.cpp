#include "DispatchController.h"

namespace genesys::launcher {

DispatchController::DispatchController(const RuntimeSelector& selector,
                                       IProcessLauncher& processLauncher)
    : selector_(selector), processLauncher_(processLauncher) {}

int DispatchController::dispatch(const QString& application,
                                 const QStringList& arguments,
                                 const LauncherConfig& config,
                                 QString* error) const {
    const RuntimeSelection selection = selector_.select(application, config);
    if (!selection.ok) {
        if (error) {
            *error = selection.reason;
        }
        return 127;
    }
    return processLauncher_.launchReplacingProcess(selection, arguments, error);
}

} // namespace genesys::launcher
