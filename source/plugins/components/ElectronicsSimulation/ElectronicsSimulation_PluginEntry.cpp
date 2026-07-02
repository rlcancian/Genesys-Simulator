#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/ElectronicsSimulation/SPICECircuit.h"
#include "plugins/components/ElectronicsSimulation/SPICENode.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &SPICECircuit::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &SPICENode::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
