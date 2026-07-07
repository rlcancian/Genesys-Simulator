// source/plugins/components/ElectronicsSimulation/ElectronicsSimulation_PluginEntry.cpp
#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ElectronicsSimulation/SPICECircuit.h"
#include "plugins/components/ElectronicsSimulation/SPICENode.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &SPICECircuit::GetPluginInformation,
        &SPICENode::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
