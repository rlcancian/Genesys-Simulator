#ifdef PLUGINCONNECT_DYNAMIC

#include <vector>
#include "./Sequence.h"
#include "./Storage.h"

// The rest of the plugins are not added because they must be statically linked
// in order to be accessed by other components.

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return {
      &Sequence::GetPluginInformation,
      &Storage::GetPluginInformation,
  };
}

#endif
