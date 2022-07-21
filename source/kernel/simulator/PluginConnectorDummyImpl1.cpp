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
#include "../../plugins/components/CppForG.h"
#include "../../plugins/components/Create.h"
#include "../../plugins/components/Decide.h"
#include "../../plugins/components/Delay.h"
#include "../../plugins/components/Dispose.h"
#include "../../plugins/components/DropOff.h"
#include "../../plugins/components/DummyComponent.h"
#include "../../plugins/components/Enter.h"
#include "../../plugins/components/Exit.h"
#include "../../plugins/components/Wait.h"
#include "../../plugins/components/Leave.h"
#include "../../plugins/components/Match.h"
#include "../../plugins/components/MarkovChain.h"
#include "../../plugins/components/PickUp.h"
#include "../../plugins/components/Seize.h"
//#include "../../plugins/components/Read.h"
#include "../../plugins/components/Release.h"
#include "../../plugins/components/Remove.h"
#include "../../plugins/components/PickStation.h"
#include "../../plugins/components/Process.h"
#include "../../plugins/components/Record.h"
#include "../../plugins/components/Route.h"
#include "../../plugins/components/Start.h"
#include "../../plugins/components/Search.h"
#include "../../plugins/components/Signal.h"
#include "../../plugins/components/Stop.h"
#include "../../plugins/components/Store.h"
#include "../../plugins/components/Separate.h"
#include "../../plugins/components/Submodel.h"
#include "../../plugins/components/Unstore.h"
#include "../../plugins/components/Write.h"
#include "../../plugins/components/LSODE.h"
#include "../../plugins/components/FSM.h"


// Model data definitions
#include "../../plugins/data/CppCode.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../plugins/data/Failure.h"
#include "../../plugins/data/File.h"
#include "../../plugins/data/Label.h"
#include "../../plugins/data/Schedule.h"
#include "../../plugins/data/Sequence.h"
#include "../../plugins/data/Set.h"
#include "../../plugins/data/SignalData.h"
#include "../../plugins/data/Station.h"
#include "../../plugins/data/Storage.h"
#include "../../plugins/data/Variable.h"
#include "../../plugins/data/FSMState.h"
#include "../../plugins/data/FSMTransition.h"
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

Plugin* PluginConnectorDummyImpl1::connect(const std::string dynamicLibraryFilename) {
	std::string fn = getFileName(dynamicLibraryFilename);
	StaticGetPluginInformation GetInfo = nullptr;
	Plugin* pluginResult = nullptr;
	// @TODO: Dummy connections basically does nothing but give access to PluginInformation alreaady compiled

	// Model data definitions


	//if (fn == "attribute.so")
	//    GetInfo = &Attribute::GetPluginInformation;
	if (fn == "assign.so")
		GetInfo = &Assign::GetPluginInformation;
	else if (fn == "entitygroup.so")
		GetInfo = &EntityGroup::GetPluginInformation;
		//	else if (fn == "ode.so")
		//		GetInfo = &OLD_ODEelement::GetPluginInformation;
	else if (fn == "queue.so")
		GetInfo = &Queue::GetPluginInformation;
	else if (fn == "resource.so")
		GetInfo = &Resource::GetPluginInformation;
	else if (fn == "set.so")
		GetInfo = &Set::GetPluginInformation;
	else if (fn == "station.so")
		GetInfo = &Station::GetPluginInformation;
	else if (fn == "label.so")
		GetInfo = &Label::GetPluginInformation;
	else if (fn == "variable.so")
		GetInfo = &Variable::GetPluginInformation;
		// model components
	else if (fn == "create.so")
		GetInfo = &Create::GetPluginInformation;
	else if (fn == "write.so")
		GetInfo = &Write::GetPluginInformation;
	else if (fn == "decide.so")
		GetInfo = &Decide::GetPluginInformation;
	else if (fn == "delay.so")
		GetInfo = &Delay::GetPluginInformation;
	else if (fn == "dispose.so")
		GetInfo = &Dispose::GetPluginInformation;
	else if (fn == "dummy.so")
		GetInfo = &DummyComponent::GetPluginInformation;
	else if (fn == "record.so")
		GetInfo = &Record::GetPluginInformation;
	else if (fn == "release.so")
		GetInfo = &Release::GetPluginInformation;
	else if (fn == "seize.so")
		GetInfo = &Seize::GetPluginInformation;
	else if (fn == "route.so")
		GetInfo = &Route::GetPluginInformation;
	else if (fn == "process.so")
		GetInfo = &Process::GetPluginInformation;
	else if (fn == "enter.so")
		GetInfo = &Enter::GetPluginInformation;
	else if (fn == "leave.so")
		GetInfo = &Leave::GetPluginInformation;
	else if (fn == "lsode.so")
		GetInfo = &LSODE::GetPluginInformation;
	else if (fn == "markovchain.so")
		GetInfo = &MarkovChain::GetPluginInformation;
	else if (fn == "hold.so")
		GetInfo = &Wait::GetPluginInformation;
	else if (fn == "schedule.so")
		GetInfo = &Schedule::GetPluginInformation;
	else if (fn == "dropoff.so")
		GetInfo = &DropOff::GetPluginInformation;
	else if (fn == "batch.so")
		GetInfo = &Batch::GetPluginInformation;
	else if (fn == "separate.so")
		GetInfo = &Separate::GetPluginInformation;
		//else if (fn == "submodel.so")
		//    GetInfo = &Submodel::GetPluginInformation;
	else if (fn == "match.so")
		GetInfo = &Match::GetPluginInformation;
	else if (fn == "pickup.so")
		GetInfo = &PickUp::GetPluginInformation;
		//else if (fn == "read.so")
		//    GetInfo = &Read::GetPluginInformation;
	else if (fn == "remove.so")
		GetInfo = &Remove::GetPluginInformation;
	else if (fn == "search.so")
		GetInfo = &Search::GetPluginInformation;
	else if (fn == "signal.so")
		GetInfo = &Signal::GetPluginInformation;
	else if (fn == "signaldata.so")
		GetInfo = &SignalData::GetPluginInformation;
	else if (fn == "store.so")
		GetInfo = &Store::GetPluginInformation;
	else if (fn == "cppforg.so")
		GetInfo = &CppForG::GetPluginInformation;
	else if (fn == "unstore.so")
		GetInfo = &Unstore::GetPluginInformation;
		//else if (fn == "expression.so")
		//	GetInfo = &Expression::GetPluginInformation;
	else if (fn == "failure.so")
		GetInfo = &Failure::GetPluginInformation;
	else if (fn == "file.so")
		GetInfo = &File::GetPluginInformation;
	else if (fn == "storage.so")
		GetInfo = &Storage::GetPluginInformation;
	else if (fn == "pickstation.so")
		GetInfo = &PickStation::GetPluginInformation;
	else if (fn == "sequence.so")
		GetInfo = &Sequence::GetPluginInformation;
	else if (fn == "access.so")
		GetInfo = &Access::GetPluginInformation;
	else if (fn == "exit.so")
		GetInfo = &Exit::GetPluginInformation;
	else if (fn == "start.so")
		GetInfo = &Start::GetPluginInformation;
	else if (fn == "stop.so")
		GetInfo = &Stop::GetPluginInformation;
	else if (fn == "cppcode.so")
		GetInfo = &CppCode::GetPluginInformation;
	else if (fn == "fsm.so")
		GetInfo = &FSM::GetPluginInformation;
	else if (fn == "fsmstate.so")
		GetInfo = &FSMState::GetPluginInformation;
	else if (fn == "fsmtransition.so")
		GetInfo = &FSMTransition::GetPluginInformation;
	//else if (fn == "conveyour.so")
	//    GetInfo = &Conveyour::GetPluginInformation;
	//else if (fn == "segment.so")
	//    GetInfo = &Segment::GetPluginInformation;

	//else if (fn=="")

	if (GetInfo != nullptr) {
		pluginResult = new Plugin(GetInfo);
	}
	return pluginResult;
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

//namespace\\}
