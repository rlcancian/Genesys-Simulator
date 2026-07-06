/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PluginConnectorDummyImpl1.cpp
 * Author: rlcancian
 *
 * Created on 9 de Setembro de 2019, 19:24
 */

#include <vector>
#include <functional>

#include "PluginConnectorDummyImpl1.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/PluginInformation.h"

/* Model Components */

// AnalyticalModeling
#include "plugins/components/AnalyticalModeling/MarkovChain.h"

// BiochemicalSimulation
#include "plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "plugins/components/BiochemicalSimulation/BioRunnerCommand.h"
#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/BiochemicalSimulation/BioSteadyState.h"
#include "plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.h"
#include "plugins/components/BiochemicalSimulation/GeneticExpressionStep.h"
#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"

// Continuous
#include "plugins/components/Continuous/DiffEquations.h"
#include "plugins/components/Continuous/LSODE.h"
#include "plugins/components/Continuous/OLD_ODEelement.h"

// Decisions
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/Decisions/PickUp.h"
#include "plugins/components/Decisions/DropOff.h"
#include "plugins/components/Decisions/Remove.h"
#include "plugins/components/Decisions/Search.h"

// DiscreteProcessing
#include "plugins/components/DiscreteProcessing/Process.h"
#include "plugins/components/DiscreteProcessing/Seize.h"
#include "plugins/components/DiscreteProcessing/Release.h"
#include "plugins/components/DiscreteProcessing/Buffer.h"
#include "plugins/components/DiscreteProcessing/Clone.h"
#include "plugins/components/DiscreteProcessing/Delay.h"

// ElectronicsSimulation
#include "plugins/components/ElectronicsSimulation/SPICECircuit.h"
#include "plugins/components/ElectronicsSimulation/SPICENode.h"

// ExternalIntegration
#include "plugins/components/ExternalIntegration/CppForG.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"

// Grouping
#include "plugins/components/Grouping/Batch.h"
#include "plugins/components/Grouping/Separate.h"

// InputOutput
#include "plugins/components/InputOutput/Record.h"
#include "plugins/components/InputOutput/Write.h"

// Logic
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"

// MaterialHandling
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

// ModalModel
#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "plugins/components/ModalModel/DefaultNode.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "plugins/components/ModalModel/ModalModelPetriNet.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/components/ModalModel/Submodel.h"

// Synchronization
#include "plugins/components/Synchronization/Match.h"
#include "plugins/components/Synchronization/Signal.h"
#include "plugins/components/Synchronization/Wait.h"

// Template
#include "plugins/components/Template/DummyComponent.h"


/* Model data definitions */
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"
#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"

#include "plugins/data/Logic/Formula.h"
#include "plugins/data/Logic/Variable.h"
#include "plugins/data/Logic/Label.h"
#include "plugins/data/Logic/Set.h"

#include "plugins/data/Template/DummyElement.h"

#include "plugins/data/Grouping/EntityGroup.h"

#include "plugins/data/InputOutput/File.h"

#include "plugins/data/DiscreteProcessing/Schedule.h"
#include "plugins/data/DiscreteProcessing/Failure.h"

#include "plugins/components/ModalModel/FSMState.h"

#include "plugins/data/Synchronization/SignalData.h"

#include "plugins/data/ExternalIntegration/CppCompiler.h"
#include "plugins/data/ExternalIntegration/SPICERunner.h"
#include "plugins/data/ExternalIntegration/RSimulatorRunner.h"
#include "plugins/data/ExternalIntegration/OctaveRunner.h"

#include "plugins/data/MaterialHandling/Station.h"
#include "plugins/data/MaterialHandling/Storage.h"
#include "plugins/data/MaterialHandling/Sequence.h"

#include "kernel/util/Util.h"

//namespace GenesysKernel {

PluginConnectorDummyImpl1::PluginConnectorDummyImpl1() {}

List<Plugin*>* PluginConnectorDummyImpl1::check(const std::string dynamicLibraryFilename) {
	// Dummy check creates metadata from the built-in plugin table without connecting a library.
	return connect(dynamicLibraryFilename);
}

bool PluginConnectorDummyImpl1::disconnect(const std::string dynamicLibraryFilename) {
	return true;
}

bool PluginConnectorDummyImpl1::disconnect(Plugin* plugin) {
	return true;
}

List<std::string>* PluginConnectorDummyImpl1::find() {
    auto filenames = new List<std::string>();

    filenames->insert("libplugin_AnalyticalModeling.so");
    filenames->insert("libplugin_BiochemicalSimulation.so");
    filenames->insert("libplugin_Continuous.so");
    filenames->insert("libplugin_Decisions.so");
    filenames->insert("libplugin_DiscreteProcessing.so");
    filenames->insert("libplugin_ElectronicsSimulation.so");
    filenames->insert("libplugin_BiochemicalSimulation.so");
    filenames->insert("libplugin_ExternalIntegration.so");
    filenames->insert("libplugin_Grouping.so");
    filenames->insert("libplugin_InputOutput.so");
    filenames->insert("libplugin_Logic.so");
    filenames->insert("libplugin_MaterialHandling.so");
    filenames->insert("libplugin_ModalModel.so");
    filenames->insert("libplugin_Synchronization.so");
    filenames->insert("libplugin_Template.so");
    
    return filenames;
}


List<Plugin*>* PluginConnectorDummyImpl1::connect(const std::string dynamicLibraryFilename) {

  static const std::unordered_map<std::string, std::vector<StaticGetPluginInformation>> pluginMap = {
      {"libplugin_AnalyticalModeling.so",   {
        &MarkovChain::GetPluginInformation
      }},
      {"libplugin_BiochemicalSimulation.so", {
        // Component
        &BacteriaColony::GetPluginInformation,
        &BioRunnerCommand::GetPluginInformation,
        &BioSimulate::GetPluginInformation,
        &BioSteadyState::GetPluginInformation,
        &GeneticCircuitSimulate::GetPluginInformation,
        &GeneticExpressionStep::GetPluginInformation,
        &MetabolicFluxBalance::GetPluginInformation,
        // Data
        &BacteriaSignalGrid::GetPluginInformation,
        &BioSimulatorRunner::GetPluginInformation,
        &BioNetwork::GetPluginInformation,
        &BioParameter::GetPluginInformation,
        &BioReaction::GetPluginInformation,
        &BioSpecies::GetPluginInformation,
        &GroProgram::GetPluginInformation,

      }},
      {"libplugin_Continuous.so",           {
        &DiffEquations::GetPluginInformation,
        &LSODE::GetPluginInformation,
        &OLD_ODEelement::GetPluginInformation,
      }},
      {"libplugin_Decisions.so",            {
        &Decide::GetPluginInformation,
        &PickUp::GetPluginInformation,
        &DropOff::GetPluginInformation,
        &Remove::GetPluginInformation,
        &Search::GetPluginInformation,
      }},
      {"libplugin_DiscreteProcessing.so", {
        &Process::GetPluginInformation,
        &Seize::GetPluginInformation,
        &Release::GetPluginInformation,
        &Buffer::GetPluginInformation,
        &Clone::GetPluginInformation,
        &Delay::GetPluginInformation,

        &Schedule::GetPluginInformation,
        &Failure::GetPluginInformation
      }}, 
      {"libplugin_ElectronicsSimulation.so",{
        &SPICECircuit::GetPluginInformation,
        &SPICENode::GetPluginInformation
      }},
      {"libplugin_ExternalIntegration.so",  {
        &CppForG::GetPluginInformation,
        &RSimulator::GetPluginInformation,

        &CppCompiler::GetPluginInformation,
        &SPICERunner::GetPluginInformation,
        &RSimulatorRunner::GetPluginInformation,
        &OctaveRunner::GetPluginInformation
      }},
      {"libplugin_Grouping.so",             {
        &Batch::GetPluginInformation,
        &Separate::GetPluginInformation,

        &EntityGroup::GetPluginInformation      
      }},
      {"libplugin_InputOutput.so",          {
        &Record::GetPluginInformation,
        &Write::GetPluginInformation,

        &File::GetPluginInformation
      }},
      {"libplugin_Logic.so",                {
        // Components
        &Assign::GetPluginInformation,
        &Create::GetPluginInformation,
        &Dispose::GetPluginInformation,
        // Data
        &Formula::GetPluginInformation,
        &Variable::GetPluginInformation,
        &Label::GetPluginInformation,
        &Set::GetPluginInformation
      }},
      {"libplugin_MaterialHandling.so",     {
        &Access::GetPluginInformation,
        &Enter::GetPluginInformation,
        &Exit::GetPluginInformation,
        &Leave::GetPluginInformation,
        &PickStation::GetPluginInformation,
        &Route::GetPluginInformation,
        &Start::GetPluginInformation,
        &Stop::GetPluginInformation,
        &Store::GetPluginInformation,
        &Unstore::GetPluginInformation,

        &Station::GetPluginInformation,
        &Storage::GetPluginInformation,
        &Sequence::GetPluginInformation
      }},
      {"libplugin_ModalModel.so",           {
        &CellularAutomataComp::GetPluginInformation,
        &DefaultNode::GetPluginInformation,
        &FSMState::GetPluginInformation,
        &ModalModelDefault::GetPluginInformation,
        &ModalModelFSM::GetPluginInformation,
        &ModalModelPetriNet::GetPluginInformation,
        &PetriPlace::GetPluginInformation,
        &Submodel::GetPluginInformation,

        &FSMState::GetPluginInformation,
      }},
      {"libplugin_Synchronization.so",      {
        &Match::GetPluginInformation,
        &Signal::GetPluginInformation,
        &Wait::GetPluginInformation,

        &SignalData::GetPluginInformation
      }},
      {"libplugin_Template.so",             {
        &DummyComponent::GetPluginInformation,
        &DummyElement::GetPluginInformation
      }},
    };

	  std::string filename  = Util::FilenameFromFullFilename(dynamicLibraryFilename);
    auto it = pluginMap.find(filename);
    if (it == pluginMap.end()) {
        return nullptr;
    }

    List<Plugin*>* pluginResult = new List<Plugin*>();
    for (const StaticGetPluginInformation& getInfo : it->second) {
        pluginResult->insert(new Plugin(getInfo));
    }

    return pluginResult;
}



//namespace\\}
