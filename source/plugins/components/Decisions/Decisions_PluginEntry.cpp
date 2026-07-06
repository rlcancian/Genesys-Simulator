#ifdef PLUGINCONNECT_DYNAMIC

#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Decisions/PickUp.h"
#include "plugins/components/Decisions/DropOff.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Decisions/Search.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Decide::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &PickUp::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &DropOff::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_3() { return &Remove::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &Search::GetPluginInformation; }

#endif
