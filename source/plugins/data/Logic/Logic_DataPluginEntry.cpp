#ifdef PLUGINCONNECT_DYNAMIC

#include <vector>
#include "./Formula.h"
#include "./Label.h"
#include "./Set.h"
#include "./Variable.h"

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return {
      &Formula::GetPluginInformation,
      &Label::GetPluginInformation,
      &Set::GetPluginInformation,
      &Variable::GetPluginInformation,
  };
}

#endif
