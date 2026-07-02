#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Synchronization/Match.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/Synchronization/Wait.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Match::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Signal::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &Wait::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
