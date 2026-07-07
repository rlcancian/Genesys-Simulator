#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/MaterialHandling/Access.h"
#include "plugins/components/MaterialHandling/Enter.h"
#include "plugins/components/MaterialHandling/Exit.h"
#include "plugins/components/MaterialHandling/Leave.h"
#include "plugins/components/MaterialHandling/PickStation.h"
#include "plugins/components/MaterialHandling/Route.h"
#include "plugins/components/MaterialHandling/Start.h"
#include "plugins/components/MaterialHandling/Stop.h"
#include "plugins/components/MaterialHandling/Store.h"
#include "plugins/components/MaterialHandling/Unstore.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation();

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    std::vector<StaticGetPluginInformation> plugins = {
        &Access::GetPluginInformation,
        &Enter::GetPluginInformation,
        &Exit::GetPluginInformation,
        &Leave::GetPluginInformation,
        &PickStation::GetPluginInformation,
        &Route::GetPluginInformation,
        &Start::GetPluginInformation,
        &Stop::GetPluginInformation,
        &Store::GetPluginInformation,
        &Unstore::GetPluginInformation
    };
    auto dataPlugins = GetAllDataPluginInformation();
    plugins.insert(plugins.end(), dataPlugins.begin(), dataPlugins.end());
    return plugins;
}

#endif // PLUGINCONNECT_DYNAMIC
