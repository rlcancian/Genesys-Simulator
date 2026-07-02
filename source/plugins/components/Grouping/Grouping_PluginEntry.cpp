#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Grouping/Batch.h"
#include "plugins/components/Grouping/Separate.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Batch::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Separate::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
