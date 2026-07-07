#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Decisions/PickUp.h"
#include "plugins/components/Decisions/DropOff.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Decisions/Search.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    return {
        &Decide::GetPluginInformation,
        &PickUp::GetPluginInformation,
        &DropOff::GetPluginInformation,
        &Remove::GetPluginInformation,
        &Search::GetPluginInformation
    };
}

#endif // PLUGINCONNECT_DYNAMIC
