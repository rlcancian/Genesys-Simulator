#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &Assign::GetPluginInformation,
        &Create::GetPluginInformation,
        &Dispose::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
