#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Synchronization/Match.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/Synchronization/Wait.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation();

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    std::vector<StaticGetPluginInformation> plugins = {
        &Match::GetPluginInformation,
        &Signal::GetPluginInformation,
        &Wait::GetPluginInformation
    };
    auto dataPlugins = GetAllDataPluginInformation();
    plugins.insert(plugins.end(), dataPlugins.begin(), dataPlugins.end());
    return plugins;
}

#endif // PLUGINCONNECT_DYNAMIC
