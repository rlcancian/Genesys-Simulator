#ifdef PLUGINCONNECT_DYNAMIC

#include "kernel/simulator/PluginInformation.h"
#include "plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "plugins/components/BiochemicalSimulation/BioRunnerCommand.h"
#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/BiochemicalSimulation/BioSteadyState.h"
#include "plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.h"
#include "plugins/components/BiochemicalSimulation/GeneticExpressionStep.h"
#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"

extern "C" StaticGetPluginInformation GetPluginInformation_0() { return &BacteriaColony::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_1() { return &BioRunnerCommand::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_2() { return &BioSimulate::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_3() { return &BioSteadyState::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_4() { return &GeneticCircuitSimulate::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_5() { return &GeneticExpressionStep::GetPluginInformation; }
extern "C" StaticGetPluginInformation GetPluginInformation_6() { return &MetabolicFluxBalance::GetPluginInformation; }

#endif // PLUGINCONNECT_DYNAMIC
