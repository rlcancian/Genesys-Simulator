#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Grouping/Batch.h"
#include "plugins/components/Grouping/Separate.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &Batch::GetPluginInformation,
        &Separate::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
