#ifdef PLUGINCONNECT_DYNAMIC

#include <vector>
#include "./SignalData.h"

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return {
      &SignalData::GetPluginInformation,
  };
}

#endif
