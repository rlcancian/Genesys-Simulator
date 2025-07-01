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

// Model Components

#include "../../plugins/components/Access.h"
#include "../../plugins/components/Assign.h"
#include "../../plugins/components/Batch.h"
#include "../../plugins/components/Buffer.h"
#include "../../plugins/components/Clone.h"
#include "../../plugins/components/CppForG.h"
#include "../../plugins/components/Create.h"
#include "../../plugins/components/Decide.h"
#include "../../plugins/components/Delay.h"
#include "../../plugins/components/DiffEquations.h"
#include "../../plugins/components/Dispose.h"
#include "../../plugins/components/DropOff.h"
#include "../../plugins/components/DummyComponent.h"
#include "../../plugins/components/Enter.h"
#include "../../plugins/components/Exit.h"
#include "../../plugins/components/Leave.h"
#include "../../plugins/components/Match.h"
#include "../../plugins/components/MarkovChain.h"
//#include "../../plugins/components/Octave.h"
#include "../../plugins/components/PickStation.h"
#include "../../plugins/components/PickUp.h"
#include "../../plugins/components/Seize.h"
//#include "../../plugins/components/Read.h"
#include "../../plugins/components/Release.h"
#include "../../plugins/components/Remove.h"
#include "../../plugins/components/Process.h"
#include "../../plugins/components/Record.h"
#include "../../plugins/components/Route.h"
#include "../../plugins/components/Start.h"
#include "../../plugins/components/Search.h"
#include "../../plugins/components/Signal.h"
#include "../../plugins/components/SPICECircuit.h"
#include "../../plugins/components/SPICENode.h"
#include "../../plugins/components/Stop.h"
#include "../../plugins/components/Store.h"
#include "../../plugins/components/Separate.h"
#include "../../plugins/components/Submodel.h"
#include "../../plugins/components/Unstore.h"
#include "../../plugins/components/Wait.h"
#include "../../plugins/components/Write.h"
#include "../../plugins/components/LSODE.h"


// Model data definitions
#include "../../plugins/data/CppCompiler.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../plugins/data/Failure.h"
#include "../../plugins/data/File.h"
#include "../../plugins/data/Formula.h"
#include "../../plugins/data/Label.h"
#include "../../plugins/data/Schedule.h"
#include "../../plugins/data/Sequence.h"
#include "../../plugins/data/Set.h"
#include "../../plugins/data/SignalData.h"
#include "../../plugins/data/SPICERunner.h"
#include "../../plugins/data/Station.h"
#include "../../plugins/data/Storage.h"
#include "../../plugins/data/Variable.h"
//#include "../../plugins/data/Expression.h"
//#include "../../plugins/data/Conveyor.h"
//#include "../../plugins/data/Segment.h"

#include "../util/Util.h"

//namespace GenesysKernel {

PluginConnectorDummyImpl1::PluginConnectorDummyImpl1() {
}

Plugin* PluginConnectorDummyImpl1::check(const std::string dynamicLibraryFilename) {
	// @TODO: not implemented yet!
	//dynamicLibraryFilename = ""; // just to use it
	return nullptr; /**@ @TODO:To implement */
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

//Plugin* PluginConnectorDummyImpl1::_connectModelDefinitions(const std::string dynamicLibraryFilename){
//    StaticGetPluginInformation GetInfo = nullptr;
//}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectBasic(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
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
    else if (fn == "entitygroup.so")
        GetInfo = &EntityGroup::GetPluginInformation;
    else if (fn == "failure.so")
        GetInfo = &Failure::GetPluginInformation;
    else if (fn == "formula.so")
        GetInfo = &Formula::GetPluginInformation;
    else if (fn == "label.so")
        GetInfo = &Label::GetPluginInformation;
    else if (fn == "queue.so")
        GetInfo = &Queue::GetPluginInformation;
    else if (fn == "resource.so")
        GetInfo = &Resource::GetPluginInformation;
    else if (fn == "variable.so")
        GetInfo = &Variable::GetPluginInformation;
    return GetInfo;
}
StaticGetPluginInformation PluginConnectorDummyImpl1::_connectDiscrete(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "batch.so")
        GetInfo = &Batch::GetPluginInformation;
    else if (fn == "clone.so")
        GetInfo = &Clone::GetPluginInformation;
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
    //else if (fn == "submodel.so")
    //    GetInfo = &Submodel::GetPluginInformation;
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
    return GetInfo;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectContinuos(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "diffequations.so")
        GetInfo = &DiffEquations::GetPluginInformation;
    else if (fn == "lsode.so")
        GetInfo = &LSODE::GetPluginInformation;
    //else if (fn == "finiteelement.so")
    //    GetInfo = &LSODE::GetPluginInformation;
    //else if (fn == "finitevolume.so")
    //    GetInfo = &LSODE::GetPluginInformation;
    return GetInfo;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectIntegrations(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "cppcompiler.so")
        GetInfo = &CppCompiler::GetPluginInformation;
    else if (fn == "cppforg.so")
        GetInfo = &CppForG::GetPluginInformation;
    else if (fn == "spicecircuit.so")
        GetInfo = &SPICECircuit::GetPluginInformation;
    else if (fn == "spicenode.so")
        GetInfo = &SPICENode::GetPluginInformation;
    else if (fn == "spicerunner.so")
        GetInfo = &SPICERunner::GetPluginInformation;
    //else if (fn == "octave.so")
    //	GetInfo = &Octave::GetPluginInformation;
    return GetInfo;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectInputOutput(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "file.so")
        GetInfo = &File::GetPluginInformation;
    //else if (fn == "read.so")
    //    GetInfo = &Read::GetPluginInformation;
    else if (fn == "write.so")
        GetInfo = &Write::GetPluginInformation;
    return GetInfo;
}



StaticGetPluginInformation PluginConnectorDummyImpl1::_connectTransfer(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "access.so")
        GetInfo = &Access::GetPluginInformation;
    else if (fn == "enter.so")
        GetInfo = &Enter::GetPluginInformation;
    else if (fn == "exit.so")
        GetInfo = &Exit::GetPluginInformation;
    else if (fn == "leave.so")
        GetInfo = &Leave::GetPluginInformation;
    else if (fn == "pickstation.so")
        GetInfo = &PickStation::GetPluginInformation;
    else if (fn == "route.so")
        GetInfo = &Route::GetPluginInformation;
    else if (fn == "start.so")
        GetInfo = &Start::GetPluginInformation;
    else if (fn == "stop.so")
        GetInfo = &Stop::GetPluginInformation;
    else if (fn == "station.so")
        GetInfo = &Station::GetPluginInformation;
    else if (fn == "sequence.so")
        GetInfo = &Sequence::GetPluginInformation;
    return GetInfo;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectNetwork(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    //if (fn == "cellularautomata.so")
    //    GetInfo = &CellularAutomataComp::GetPluginInformation;
    //else
    /*
    if (fn == "efsmData.so")
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
    if (fn == "markovchain.so")
        GetInfo = &MarkovChain::GetPluginInformation;

    // boolean networks

    // petri nets

    // finite elements

    return GetInfo;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectElectronicDomain(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;
    if (fn == "resistor.so")
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
    return GetInfo;
}


StaticGetPluginInformation PluginConnectorDummyImpl1::_connectBiochemicalDomain(const std::string fn){
    StaticGetPluginInformation GetInfo = nullptr;

    return GetInfo;
}

Plugin* PluginConnectorDummyImpl1::connect(const std::string dynamicLibraryFilename) {
	std::string fn = Util::FilenameFromFullFilename(dynamicLibraryFilename);
	StaticGetPluginInformation GetInfo = nullptr;
    // @TODO: Dummy connections basically does nothing but give access to PluginInformation already compiled

    GetInfo = this->_connectBasic(fn);
    if (GetInfo==nullptr) {
        GetInfo = this->_connectDiscrete(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectContinuos(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectTransfer(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectIntegrations(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectInputOutput(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectNetwork(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectElectronicDomain(fn);
    }
    if (GetInfo==nullptr) {
        GetInfo = this->_connectBiochemicalDomain(fn);
    }

    Plugin* pluginResult = nullptr;
    if (GetInfo != nullptr) {
		pluginResult = new Plugin(GetInfo);
    }
	return pluginResult;
}



//namespace\\}
