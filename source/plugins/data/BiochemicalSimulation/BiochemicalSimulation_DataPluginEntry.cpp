#ifdef PLUGINCONNECT_DYNAMIC

#include "./BacteriaSignalGrid.h"
#include "./BioNetwork.h"
#include "./BioParameter.h"
#include "./BioSimulatorRunner.h"
#include "./BioSpecies.h"
#include "./GeneticCircuit.h"
#include "./GeneticCircuitPart.h"
#include "./GeneticRegulation.h"
#include "./GroProgram.h"
#include "./MetabolicNetwork.h"
#include "./MetabolicReaction.h"


extern "C" std::vector<StaticGetPluginInformation> GetAllDataPluginInformation() {
    return {
      &BacteriaSignalGrid::GetPluginInformation,
      &BioNetwork::GetPluginInformation,
      &BioParameter::GetPluginInformation,
      &BioSimulatorRunner::GetPluginInformation,
      &BioSpecies::GetPluginInformation,
      &GeneticCircuit::GetPluginInformation,
      &GeneticCircuitPart::GetPluginInformation,
      &GeneticRegulation::GetPluginInformation,
      &GroProgram::GetPluginInformation,
      &MetabolicNetwork::GetPluginInformation,
      &MetabolicReaction::GetPluginInformation,
  };
}

#endif
