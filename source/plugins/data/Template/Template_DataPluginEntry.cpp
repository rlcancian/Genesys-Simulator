#ifdef PLUGINCONNECT_DYNAMIC

#include <vector>
#include "./DummyElement.h"

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return {
      &DummyElement::GetPluginInformation,
  };
}

#endif
