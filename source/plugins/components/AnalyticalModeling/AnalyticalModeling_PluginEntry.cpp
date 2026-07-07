#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/AnalyticalModeling/MarkovChain.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return { &MarkovChain::GetPluginInformation };
}

#endif // PLUGINCONNECT_DYNAMIC
