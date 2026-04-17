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

#include "PluginConnectorDummyImpl1.h"
#include "../kernel/simulator/Plugin.h"

// Model Components

#include "components/Access.h"
#include "components/Assign.h"
#include "components/Batch.h"
#include "components/Buffer.h"
#include "components/CellularAutomataComp.h"
#include "components/Clone.h"
#include "components/CppForG.h"
#include "components/Create.h"
#include "components/Decide.h"
#include "components/ModalModelDefault.h"
#include "components/Delay.h"
#include "components/DiffEquations.h"
#include "components/Dispose.h"
#include "components/DropOff.h"
#include "components/DummyComponent.h"
#include "components/Enter.h"
#include "components/Exit.h"
#include "components/Leave.h"
#include "components/Match.h"
#include "components/MarkovChain.h"
//#include "../../plugins/components/Octave.h"
#include "components/PickStation.h"
#include "components/PickUp.h"
#include "components/Seize.h"
#include "components/ModalModelFSM.h"
#include "components/ModalModelPetriNet.h"
//#include "../../plugins/components/Read.h"
#include "components/Release.h"
#include "components/Remove.h"
#include "components/Process.h"
#include "components/Record.h"
#include "components/Route.h"
#include "components/RSimulator.h"
#include "components/Start.h"
#include "components/Search.h"
#include "components/Signal.h"
#include "components/SPICECircuit.h"
#include "components/SPICENode.h"
#include "components/Stop.h"
#include "components/Store.h"
#include "components/Separate.h"
#include "components/Submodel.h"
#include "components/Unstore.h"
#include "components/Wait.h"
#include "components/Write.h"
#include "components/LSODE.h"
#include "components/OLD_ODEelement.h"
#include "components/bacteria/BacteriaColony.h"
#include "components/network/DefaultNode.h"
#include "components/network/PetriPlace.h"


// Model data definitions
#include "data/BioSimulatorRunner.h"
#include "data/BioNetwork.h"
#include "data/BioParameter.h"
#include "data/BioReaction.h"
#include "data/BioSpecies.h"
#include "data/CppCompiler.h"
#include "data/DummyElement.h"
#include "data/EntityGroup.h"
#include "data/Failure.h"
#include "data/File.h"
#include "data/Formula.h"
#include "data/GroProgram.h"
#include "data/Label.h"
#include "data/Schedule.h"
#include "data/Sequence.h"
#include "data/Set.h"
#include "data/SignalData.h"
#include "data/SPICERunner.h"
#include "data/RSimulatorRunner.h"
#include "data/Station.h"
#include "data/Storage.h"
#include "data/Variable.h"
//#include "../../plugins/data/Expression.h"
//#include "../../plugins/data/Conveyor.h"
//#include "../../plugins/data/Segment.h"

#include "../kernel/util/Util.h"
#include "components/network/FSMState.h"

//namespace GenesysKernel {

PluginConnectorDummyImpl1::PluginConnectorDummyImpl1() {
}

Plugin* PluginConnectorDummyImpl1::check(const std::string dynamicLibraryFilename) {
	// Dummy check creates metadata from the built-in plugin table without connecting a library.
	return connect(dynamicLibraryFilename);
}

bool PluginConnectorDummyImpl1::disconnect(const std::string dynamicLibraryFilename) {
	// @TODO: To implement!
	//dynamicLibraryFilename = ""; // just to use ut
	return true;
}

bool PluginConnectorDummyImpl1::disconnect(Plugin* plugin) {
	// @TODO: To implement!
	return true;
}

List<std::string>* PluginConnectorDummyImpl1::find() {
    List<std::string>* filenames = new List<std::string>();
    filenames->insert("assign.so");
    filenames->insert("buffer.so");
    filenames->insert("create.so");
    filenames->insert("dispose.so");
    filenames->insert("dummy.so");
    filenames->insert("dummyelement.so");
    filenames->insert("entitygroup.so");
    filenames->insert("failure.so");
    filenames->insert("formula.so");
    filenames->insert("groprogram.so");
    filenames->insert("label.so");
    filenames->insert("queue.so");
    filenames->insert("resource.so");
    filenames->insert("variable.so");
    filenames->insert("batch.so");
    filenames->insert("bacteriacolony.so");
    filenames->insert("bionetwork.so");
    filenames->insert("bioparameter.so");
    filenames->insert("bioreaction.so");
    filenames->insert("biospecies.so");
    filenames->insert("biosimulatorrunner.so");
    filenames->insert("cellularautomata.so");
    filenames->insert("clone.so");
    filenames->insert("ModalModelDefault.so");
    filenames->insert("decide.so");
    filenames->insert("delay.so");
    filenames->insert("dropoff.so");
    filenames->insert("hold.so");
    filenames->insert("match.so");
    filenames->insert("process.so");
    filenames->insert("pickup.so");
    filenames->insert("remove.so");
    filenames->insert("record.so");
    filenames->insert("release.so");
    filenames->insert("storage.so");
    filenames->insert("separate.so");
    filenames->insert("submodel.so");
    filenames->insert("seize.so");
    filenames->insert("search.so");
    filenames->insert("signal.so");
    filenames->insert("store.so");
    filenames->insert("unstore.so");
    filenames->insert("storage.so");
    filenames->insert("set.so");
    filenames->insert("schedule.so");
    filenames->insert("signaldata.so");
    filenames->insert("diffequations.so");
    filenames->insert("lsode.so");
    //filenames->insert("finiteelement.so");
    filenames->insert("old_odeelement.so");
    filenames->insert("modalmodelfsm.so");
    filenames->insert("fsmstate.so");
    filenames->insert("modalmodelpetrinet.so");
    //filenames->insert("finitevolume.so");
    filenames->insert("cppcompiler.so");
    filenames->insert("cppforg.so");
    filenames->insert("spicecircuit.so");
    filenames->insert("spicenode.so");
    filenames->insert("spicerunner.so");
    filenames->insert("rsimulatorrunner.so");
    //filenames->insert("octave.so");
    filenames->insert("file.so");
    //filenames->insert("read.so");
    filenames->insert("write.so");
    filenames->insert("access.so");
    filenames->insert("defaultnode.so");
    filenames->insert("enter.so");
    filenames->insert("exit.so");
    filenames->insert("leave.so");
    filenames->insert("pickstation.so");
    filenames->insert("petriplace.so");
    filenames->insert("route.so");
    filenames->insert("rsimulator.so");
    filenames->insert("start.so");
    filenames->insert("stop.so");
    filenames->insert("station.so");
    filenames->insert("sequence.so");
    /*
    if (fn == "cellularautomata.so");"modalmodelfsm.so");
    filenames->insert(("modalmodelpetrinet.so"
    filenames->insert("efsmData.so");
    filenames->insert("efsm.so");
    filenames->insert("fsm_state.so");
    filenames->insert("fsm_transition.so");
    filenames->insert("fsm_modalmodel.so");
    */
    filenames->insert("markovchain.so");
    filenames->insert("resistor.so");
    filenames->insert("vsource.so");
    filenames->insert("vpulse.so");
    filenames->insert("vsine.so");
    filenames->insert("capacitor.so");
    filenames->insert("diode.so");
    filenames->insert("pmos.so");
    filenames->insert("nmos.so");
    filenames->insert("not.so");
    filenames->insert("nor.so");
    filenames->insert("nand.so");
    filenames->insert("and.so");
    filenames->insert("or.so");
    filenames->insert("xor.so");
    filenames->insert("xnor.so");
    return filenames;
}


Plugin* PluginConnectorDummyImpl1::connect(const std::string dynamicLibraryFilename) {
	std::string fn = Util::FilenameFromFullFilename(dynamicLibraryFilename);
	StaticGetPluginInformation GetInfo = nullptr;
    // @TODO: Dummy connections basically does nothing but give access to PluginInformation already compiled

    if (fn == "assign.so")
        GetInfo = &Assign::GetPluginInformation;
    else if (fn == "buffer.so")
        GetInfo = &Buffer::GetPluginInformation;
    else if (fn == "create.so")
        GetInfo = &Create::GetPluginInformation;
    else if (fn == "dispose.so")
        GetInfo = &Dispose::GetPluginInformation;
    else if (fn == "dummy.so")
        GetInfo = &DummyComponent::GetPluginInformation;
    else if (fn == "dummyelement.so")
        GetInfo = &DummyElement::GetPluginInformation;
    else if (fn == "entitygroup.so")
        GetInfo = &EntityGroup::GetPluginInformation;
    else if (fn == "failure.so")
        GetInfo = &Failure::GetPluginInformation;
    else if (fn == "formula.so")
        GetInfo = &Formula::GetPluginInformation;
    else if (fn == "groprogram.so")
        GetInfo = &GroProgram::GetPluginInformation;
    else if (fn == "label.so")
        GetInfo = &Label::GetPluginInformation;
    else if (fn == "queue.so")
        GetInfo = &Queue::GetPluginInformation;
    else if (fn == "resource.so")
        GetInfo = &Resource::GetPluginInformation;
    else if (fn == "variable.so")
        GetInfo = &Variable::GetPluginInformation;
    else if (fn == "batch.so")
        GetInfo = &Batch::GetPluginInformation;
    else if (fn == "bacteriacolony.so")
        GetInfo = &BacteriaColony::GetPluginInformation;
    else if (fn == "bionetwork.so")
        GetInfo = &BioNetwork::GetPluginInformation;
    else if (fn == "bioparameter.so")
        GetInfo = &BioParameter::GetPluginInformation;
    else if (fn == "bioreaction.so")
        GetInfo = &BioReaction::GetPluginInformation;
    else if (fn == "biospecies.so")
        GetInfo = &BioSpecies::GetPluginInformation;
    else if (fn == "biosimulatorrunner.so")
        GetInfo = &BioSimulatorRunner::GetPluginInformation;
    else if (fn == "cellularautomata.so")
        GetInfo = &CellularAutomataComp::GetPluginInformation;
    else if (fn == "clone.so")
        GetInfo = &Clone::GetPluginInformation;
    else if (fn == "ModalModelDefault.so")
        GetInfo = &ModalModelDefault::GetPluginInformation;
    else if (fn == "decide.so")
        GetInfo = &Decide::GetPluginInformation;
    else if (fn == "delay.so")
        GetInfo = &Delay::GetPluginInformation;
    else if (fn == "dropoff.so")
        GetInfo = &DropOff::GetPluginInformation;
    else if (fn == "hold.so")
        GetInfo = &Wait::GetPluginInformation;
    else if (fn == "match.so")
        GetInfo = &Match::GetPluginInformation;
    else if (fn == "process.so")
        GetInfo = &Process::GetPluginInformation;
    else if (fn == "pickup.so")
        GetInfo = &PickUp::GetPluginInformation;
    else if (fn == "remove.so")
        GetInfo = &Remove::GetPluginInformation;
    else if (fn == "record.so")
        GetInfo = &Record::GetPluginInformation;
    else if (fn == "release.so")
        GetInfo = &Release::GetPluginInformation;
    else if (fn == "storage.so")
        GetInfo = &Storage::GetPluginInformation;
    else if (fn == "separate.so")
        GetInfo = &Separate::GetPluginInformation;
    else if (fn == "submodel.so")
        GetInfo = &Submodel::GetPluginInformation;
    else if (fn == "seize.so")
        GetInfo = &Seize::GetPluginInformation;
    else if (fn == "search.so")
        GetInfo = &Search::GetPluginInformation;
    else if (fn == "signal.so")
        GetInfo = &Signal::GetPluginInformation;
    else if (fn == "store.so")
        GetInfo = &Store::GetPluginInformation;
    else if (fn == "unstore.so")
        GetInfo = &Unstore::GetPluginInformation;
    else if (fn == "storage.so")
        GetInfo = &Storage::GetPluginInformation;
    else if (fn == "set.so")
        GetInfo = &Set::GetPluginInformation;
    else if (fn == "schedule.so")
        GetInfo = &Schedule::GetPluginInformation;
    else if (fn == "signaldata.so")
        GetInfo = &SignalData::GetPluginInformation;
    else if (fn == "diffequations.so")
        GetInfo = &DiffEquations::GetPluginInformation;
    else if (fn == "lsode.so")
        GetInfo = &LSODE::GetPluginInformation;
    //else if (fn == "finiteelement.so")
    else if (fn == "old_odeelement.so")
        GetInfo = &OLD_ODEelement::GetPluginInformation;
    else if (fn == "modalmodelfsm.so")
        GetInfo = &ModalModelFSM::GetPluginInformation;
    else if (fn == "fsmstate.so")
        GetInfo = &FSMState::GetPluginInformation;
    else if (fn == "modalmodelpetrinet.so")
        GetInfo = &ModalModelPetriNet::GetPluginInformation;
    //    GetInfo = &LSODE::GetPluginInformation;
    //else if (fn == "finitevolume.so")
    //    GetInfo = &LSODE::GetPluginInformation;
    else if (fn == "cppcompiler.so")
        GetInfo = &CppCompiler::GetPluginInformation;
    else if (fn == "cppforg.so")
        GetInfo = &CppForG::GetPluginInformation;
    else if (fn == "spicecircuit.so")
        GetInfo = &SPICECircuit::GetPluginInformation;
    else if (fn == "spicenode.so")
        GetInfo = &SPICENode::GetPluginInformation;
    else if (fn == "spicerunner.so")
        GetInfo = &SPICERunner::GetPluginInformation;
    else if (fn == "rsimulatorrunner.so")
        GetInfo = &RSimulatorRunner::GetPluginInformation;
    //else if (fn == "octave.so")
    //	GetInfo = &Octave::GetPluginInformation;
    else if (fn == "file.so")
        GetInfo = &File::GetPluginInformation;
    //else if (fn == "read.so")
    //    GetInfo = &Read::GetPluginInformation;
    else if (fn == "write.so")
        GetInfo = &Write::GetPluginInformation;
    else if (fn == "access.so")
        GetInfo = &Access::GetPluginInformation;
    else if (fn == "defaultnode.so")
        GetInfo = &DefaultNode::GetPluginInformation;
    else if (fn == "enter.so")
        GetInfo = &Enter::GetPluginInformation;
    else if (fn == "exit.so")
        GetInfo = &Exit::GetPluginInformation;
    else if (fn == "leave.so")
        GetInfo = &Leave::GetPluginInformation;
    else if (fn == "pickstation.so")
        GetInfo = &PickStation::GetPluginInformation;
    else if (fn == "petriplace.so")
        GetInfo = &PetriPlace::GetPluginInformation;
    else if (fn == "route.so")
        GetInfo = &Route::GetPluginInformation;
    else if (fn == "rsimulator.so")
        GetInfo = &RSimulator::GetPluginInformation;
    else if (fn == "start.so")
        GetInfo = &Start::GetPluginInformation;
    else if (fn == "stop.so")
        GetInfo = &Stop::GetPluginInformation;
    else if (fn == "station.so")
        GetInfo = &Station::GetPluginInformation;
    else if (fn == "sequence.so")
        GetInfo = &Sequence::GetPluginInformation;
    /*
    if (fn == "cellularautomata.so")
        GetInfo = &CellularAutomataComp::GetPluginInformation;
    else if (fn == "efsmData.so")
        GetInfo = &ExtendedFSM::GetPluginInformation;
    else if (fn == "efsm.so")
        GetInfo = &OLD_FiniteStateMachine::GetPluginInformation;
    else if (fn == "fsm_state.so")
        GetInfo = &FSM_State::GetPluginInformation;
    else if (fn == "fsm_transition.so")
        GetInfo = &FSM_Transition::GetPluginInformation;
    else if (fn == "fsm_modalmodel.so")
        GetInfo = &FSM_ModalModel::GetPluginInformation;
    else
    */
    else if (fn == "markovchain.so")
        GetInfo = &MarkovChain::GetPluginInformation;
    else if (fn == "resistor.so")
        GetInfo = &Resistor::GetPluginInformation;
    else if (fn == "vsource.so")
        GetInfo = &Vsource::GetPluginInformation;
    else if (fn == "vpulse.so")
        GetInfo = &Vpulse::GetPluginInformation;
    else if (fn == "vsine.so")
        GetInfo = &Vsine::GetPluginInformation;
    else if (fn == "capacitor.so")
        GetInfo = &Capacitor::GetPluginInformation;
    else if (fn == "diode.so")
        GetInfo = &Diode::GetPluginInformation;
    else if (fn == "pmos.so")
        GetInfo = &PMOS::GetPluginInformation;
    else if (fn == "nmos.so")
        GetInfo = &NMOS::GetPluginInformation;
    else if (fn == "not.so")
        GetInfo = &NOT::GetPluginInformation;
    else if (fn == "nor.so")
        GetInfo = &NOR::GetPluginInformation;
    else if (fn == "nand.so")
        GetInfo = &NAND::GetPluginInformation;
    else if (fn == "and.so")
        GetInfo = &AND::GetPluginInformation;
    else if (fn == "or.so")
        GetInfo = &OR::GetPluginInformation;
    else if (fn == "xor.so")
        GetInfo = &XOR::GetPluginInformation;
    else if (fn == "xnor.so")
        GetInfo = &XNOR::GetPluginInformation;


    Plugin* pluginResult = nullptr;
    if (GetInfo != nullptr) {
		pluginResult = new Plugin(GetInfo);
    }
	return pluginResult;
}



//namespace\\}
