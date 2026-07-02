#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &CppForG::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &RSimulator::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
