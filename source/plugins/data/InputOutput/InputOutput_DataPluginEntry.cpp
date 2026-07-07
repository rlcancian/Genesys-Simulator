#ifdef PLUGINCONNECT_DYNAMIC

#include "./File.h"

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return { &File::GetPluginInformation };
}

#endif
