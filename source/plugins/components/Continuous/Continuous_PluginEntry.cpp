#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Continuous/DiffEquations.h"
#include "plugins/components/Continuous/LSODE.h"
#include "plugins/components/Continuous/OLD_ODEelement.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &DiffEquations::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &LSODE::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &OLD_ODEelement::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
