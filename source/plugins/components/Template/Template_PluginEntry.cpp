#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Template/DummyComponent.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return { &DummyComponent::GetPluginInformation };
}

#endif // PLUGINCONNECT_DYNAMIC
