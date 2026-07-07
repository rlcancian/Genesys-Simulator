#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Continuous/DiffEquations.h"
#include "plugins/components/Continuous/LSODE.h"
#include "plugins/components/Continuous/OLD_ODEelement.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &DiffEquations::GetPluginInformation,
        &LSODE::GetPluginInformation,
        &OLD_ODEelement::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
