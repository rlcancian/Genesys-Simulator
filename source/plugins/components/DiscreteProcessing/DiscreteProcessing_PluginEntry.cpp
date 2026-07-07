#ifdef PLUGINCONNECT_DYNAMIC

#include <vector>

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/DiscreteProcessing/Process.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Buffer.h"
#include "plugins/components/DiscreteProcessing/Clone.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation();

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    std::vector<StaticGetPluginInformation> plugins = {
        &Process::GetPluginInformation,
        &Seize::GetPluginInformation,
        &Release::GetPluginInformation,
        &Buffer::GetPluginInformation,
        &Clone::GetPluginInformation,
        &Delay::GetPluginInformation
    };
    auto dataPlugins = GetAllDataPluginInformation();
    plugins.insert(plugins.end(), dataPlugins.begin(), dataPlugins.end());
    return plugins;
}

#endif // PLUGINCONNECT_DYNAMIC
