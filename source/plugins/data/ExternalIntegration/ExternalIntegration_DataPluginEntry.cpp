#ifdef PLUGINCONNECT_DYNAMIC

#include "./OctaveRunner.h"
#include "./RSimulatorRunner.h"

// The rest of the plugins are not added because they must be statically linked
// in order to be accessed by other components.

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return { &OctaveRunner::GetPluginInformation, &RSimulatorRunner::GetPluginInformation };
}

#endif
