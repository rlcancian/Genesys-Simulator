#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "plugins/components/BiochemicalSimulation/BioRunnerCommand.h"
#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/BiochemicalSimulation/BioSteadyState.h"
#include "plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.h"
#include "plugins/components/BiochemicalSimulation/GeneticExpressionStep.h"
#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"
#include <vector>

extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation();

extern "C" std::vector<StaticGetPluginInformation> GetAllPluginInformation() {
    std::vector<StaticGetPluginInformation> plugins = {
        &BacteriaColony::GetPluginInformation,
        &BioRunnerCommand::GetPluginInformation,
        &BioSimulate::GetPluginInformation,
        &BioSteadyState::GetPluginInformation,
        &GeneticCircuitSimulate::GetPluginInformation,
        &GeneticExpressionStep::GetPluginInformation,
        &MetabolicFluxBalance::GetPluginInformation
    };
    auto dataPlugins = GetAllDataPluginInformation();
    plugins.insert(plugins.end(), dataPlugins.begin(), dataPlugins.end());
    return plugins;
}

#endif // PLUGINCONNECT_DYNAMIC
