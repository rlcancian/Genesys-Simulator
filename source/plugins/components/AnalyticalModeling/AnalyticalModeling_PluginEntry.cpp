#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/AnalyticalModeling/MarkovChain.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &MarkovChain::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
