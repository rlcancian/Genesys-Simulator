#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Template/DummyComponent.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &DummyComponent::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
