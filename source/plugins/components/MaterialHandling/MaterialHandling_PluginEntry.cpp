#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/MaterialHandling/Access.h"
#include "plugins/components/MaterialHandling/Enter.h"
#include "plugins/components/MaterialHandling/Exit.h"
#include "plugins/components/MaterialHandling/Leave.h"
#include "plugins/components/MaterialHandling/PickableStationItem.h"
#include "plugins/components/MaterialHandling/PickStation.h"
#include "plugins/components/MaterialHandling/Route.h"
#include "plugins/components/MaterialHandling/Start.h"
#include "plugins/components/MaterialHandling/Stop.h"
#include "plugins/components/MaterialHandling/Store.h"
#include "plugins/components/MaterialHandling/Unstore.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &Access::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &Enter::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &Exit::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_3() { return &Leave::GetPluginInformation; }
// extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &PickableStationItem::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &PickStation::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_5() { return &Route::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_6() { return &Start::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_7() { return &Stop::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_8() { return &Store::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_9() { return &Unstore::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
