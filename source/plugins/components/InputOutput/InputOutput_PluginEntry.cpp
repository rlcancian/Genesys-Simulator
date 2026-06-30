#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/InputOutput/Record.h"
#include "plugins/components/InputOutput/Write.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Record::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Write::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
