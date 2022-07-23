/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Traits.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 19:36
 */

#ifndef TRAITSAPP_H
#define TRAITSAPP_H

#include "../kernel/util/Util.h"

// TEMPORARY TESTS
#include "../tests/Tests.h"

// TERMINAL USER INTERFACES
#include "terminal/GenesysShell/GenesysTerminalApp.h"
//
// EXAMPLES
//
// book
#include "terminal/examples/book/Book_Cap02_Example01.h"
// smarts
#include "terminal/examples/smarts/Smart_AssignWriteSeizes.h"
#include "terminal/examples/smarts/Smart_FSM.h"
#include "terminal/examples/smarts/Smart_FSM_Hierarchical.h"
#include "terminal/examples/smarts/Smart_BatchSeparate.h"
#include "terminal/examples/smarts/Smart_Delay.h"
#include "terminal/examples/smarts/Smart_Dummy.h"
#include "terminal/examples/smarts/Smart_Failures.h"
#include "terminal/examples/smarts/Smart_ModelInfoModelSimulation.h"
#include "terminal/examples/smarts/Smart_ODE.h"
#include "terminal/examples/smarts/Smart_OnEvent.h"
#include "terminal/examples/smarts/Smart_Parser.h"
#include "terminal/examples/smarts/Smart_ParserModelFunctions.h"
#include "terminal/examples/smarts/Smart_Plugin.h"
#include "terminal/examples/smarts/Smart_Process.h"
#include "terminal/examples/smarts/Smart_ProcessSet.h"
#include "terminal/examples/smarts/Smart_RouteStation.h"
#include "terminal/examples/smarts/Smart_SeizeDelayRelease.h"
#include "terminal/examples/smarts/Smart_SeizeDelayReleaseMany.h"
#include "terminal/examples/smarts/Smart_SeizeDelayReleaseNoDataDefs.h"
#include "terminal/examples/smarts/Smart_Sequence.h"
#include "terminal/examples/smarts/Smart_WaitScanCondition.h"
#include "terminal/examples/smarts/Smart_WaitSignal.h"
#include "terminal/examples/smarts/Smart_CppForG.h"
// teachng
#include "terminal/examples/teaching/AnElectronicAssemblyAndTestSystem.h"
#include "terminal/examples/teaching/OperatingSystem02.h"
#include "terminal/examples/teaching/OperatingSystem03.h"

template <typename T>
struct TraitsApp
{
};
/*!
 *  Configure the Genesys Application to be compiled and executed
 */
template <>
struct TraitsApp<GenesysApplication_if>
{
	// static const bool runTests = false;
	static const TraceManager::Level traceLevel = TraceManager::Level::L9_mostDetailed;

	//// TESTS
	// typedef Tests Application;

	//// SMART SMARTs ALPHA SORTED
	// typedef Smart_AssignWriteSeizes Application;
	// typedef Smart_BatchSeparate Application;
	// typedef Smart_CppForG Application;
	// typedef Smart_Delay Application;
	// typedef Smart_Dummy Application;
	// typedef Smart_ModelInfoModelSimulation Application;
	// typedef Smart_ODE Application;
	// typedef Smart_OnEvent Application;
	// typedef Smart_Parser Application;
	// typedef Smart_ParserModelFunctions Application;
	// typedef Smart_Plugin Application;
	// typedef Smart_Process Application;
	// typedef Smart_ProcessSet Application;
	// typedef Smart_RouteStation Application;
	// typedef Smart_SeizeDelayRelease Application;
	//  typedef Smart_Failures Application;
	// typedef Smart_FSM Application;
	typedef Smart_FSM_Hierarchical Application;
	// typedef Smart_SeizeDelayReleaseNoDataDefs Application;
	// typedef Smart_SeizeDelayReleaseMany Application;
	// typedef Smart_Sequence Application;
	// typedef Smart_WaitScanCondition Application;
	// typedef Smart_WaitSignal Application;

	//// TEACHING
	// typedef AnElectronicAssemblyAndTestSystem Application;
	// typedef OperatingSystem02 Application;
	// typedef OperatingSystem03 Application;

	//// BOOK
	// typedef Book_Cap02_Example01 Application;

	//// TERMINAL APPLICATIONS
	// typedef GenesysTerminalApp Application;
};

#endif /* TRAITSAPP_H */
