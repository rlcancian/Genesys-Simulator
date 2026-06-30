#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Assign::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Create::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &Dispose::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
