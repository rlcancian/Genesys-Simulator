#ifdef PLUGINCONNECT_DYNAMIC

#include "plugins/components/DiscreteProcessing/Process.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Buffer.h"
#include "plugins/components/DiscreteProcessing/Clone.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "kernel/simulator/PluginInformation.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Process::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Seize::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &Release::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_3() { return &Buffer::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &Clone::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_5() { return &Delay::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
