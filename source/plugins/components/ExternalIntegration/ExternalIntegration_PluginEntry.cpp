#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation();

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    std::vector<StaticGetPluginInformation> plugins = {
        &CppForG::GetPluginInformation,
        &RSimulator::GetPluginInformation
    };
    auto dataPlugins = GetAllDataPluginInformation();
    plugins.insert(plugins.end(), dataPlugins.begin(), dataPlugins.end());
    return plugins;
}

#endif // PLUGINCONNECT_DYNAMIC
