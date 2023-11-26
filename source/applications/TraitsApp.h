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


// TERMINAL USER INTERFACES
#include "terminal/GenesysShell/GenesysShell.h"
//
// EXAMPLES
//
// book
#include "terminal/examples/book/Book_Cap02_Example01.h"
// smarts
#include "terminal/examples/smarts/Smart_AssignWriteSeizes.h"
#include "terminal/examples/smarts/Smart_BatchSeparate.h"
#include "terminal/examples/smarts/Smart_Clone.h"
#include "terminal/examples/smarts/Smart_CppForG.h"
#include "terminal/examples/smarts/Smart_Delay.h"
#include "terminal/examples/smarts/Smart_Dummy.h"
#include "terminal/examples/smarts/Smart_Failures.h"
#include "terminal/examples/smarts/Smart_FiniteStateMachine.h"
#include "terminal/examples/smarts/Smart_HoldSearchRemove.h"
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
#include "terminal/examples/smarts/Smart_SimulationControlResponse.h"
#include "terminal/examples/smarts/Smart_WaitScanCondition.h"
#include "terminal/examples/smarts/Smart_WaitSignal.h"

// Arena Smarts from UFSC/CTC/INE/INE5425 students 2022-2
#include "terminal/examples/arenaSmarts/Smart_AddingResource.h"
#include "terminal/examples/arenaSmarts/Smart_AlternatingEntityCreation.h"
#include "terminal/examples/arenaSmarts/Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime.h"
#include "terminal/examples/arenaSmarts/Smart_ArrivalsEntityTypeVsAttribute.h"
#include "terminal/examples/arenaSmarts/Smart_AssignExample.h"
#include "terminal/examples/arenaSmarts/Smart_AutomaticStatisticsCollection.h"
#include "terminal/examples/arenaSmarts/Smart_BasicModeling.h"
#include "terminal/examples/arenaSmarts/Smart_BatchAndSeparate.h"
#include "terminal/examples/arenaSmarts/Smart_ContinuousFlowEntities.h"
#include "terminal/examples/arenaSmarts/Smart_Create.h"
#include "terminal/examples/arenaSmarts/Smart_DecideNWayByChance.h"
#include "terminal/examples/arenaSmarts/Smart_DefiningAttributesAsStrings.h"
#include "terminal/examples/arenaSmarts/Smart_DefiningControlLogic.h"
#include "terminal/examples/arenaSmarts/Smart_DefiningResourceCapacity.h"
#include "terminal/examples/arenaSmarts/Smart_DelayBasedOnReplication.h"
#include "terminal/examples/arenaSmarts/Smart_EntitiesProcessedByPriority.h"
#include "terminal/examples/arenaSmarts/Smart_EvaluatingConditionsBeforeEnteringQueue.h"
#include "terminal/examples/arenaSmarts/Smart_Expression.h"
#include "terminal/examples/arenaSmarts/Smart_InventoryAndHoldingCosts.h"
#include "terminal/examples/arenaSmarts/Smart_MaxArrivalsField.h"
#include "terminal/examples/arenaSmarts/Smart_ModelRunUntil1000Parts.h"
#include "terminal/examples/arenaSmarts/Smart_ModuleDisplayVariables.h"
#include "terminal/examples/arenaSmarts/Smart_ParallelProcessingOfEntities.h"
#include "terminal/examples/arenaSmarts/Smart_PlacingEntitiesInQueueSets.h"
#include "terminal/examples/arenaSmarts/Smart_PriorityExample.h"
#include "terminal/examples/arenaSmarts/Smart_ProcessArena.h"
#include "terminal/examples/arenaSmarts/Smart_Record.h"
#include "terminal/examples/arenaSmarts/Smart_ResourceCosting.h"
#include "terminal/examples/arenaSmarts/Smart_ResourceScheduleCosting.h"
#include "terminal/examples/arenaSmarts/Smart_ResourceSets.h"
#include "terminal/examples/arenaSmarts/Smart_SeizingMultipleSimultaneosly.h"
#include "terminal/examples/arenaSmarts/Smart_SelectingRouteBasedOnProbability.h"
#include "terminal/examples/arenaSmarts/Smart_SelectingShorterQueue.h"
#include "terminal/examples/arenaSmarts/Smart_SynchronizingParallelEntities.h"
#include "terminal/examples/arenaSmarts/Smart_ValueAdded.h"
#include "terminal/examples/arenaSmarts/Smart_WaitForSignal.h"

// arena examples
#include "terminal/examples/arenaExamples/AirportSecurityExample.h"
#include "terminal/examples/arenaExamples/AirportSecurityExampleExtended.h"
#include "terminal/examples/arenaExamples/Example_Basic_Order_Shipping.h"
#include "terminal/examples/arenaExamples/Example_PublicTransport.h"

// teachng
#include "terminal/examples/teaching/AnElectronicAssemblyAndTestSystem.h"
#include "terminal/examples/teaching/OperatingSystem02.h"
#include "terminal/examples/teaching/OperatingSystem03.h"


#include "distributed/Benchmark.h"
#include "distributed/ParallelAndDistributedManager.h"
template <typename T>
struct TraitsApp {
};

/*!
 *  Configure the Genesys Application to be compiled and executed
 */
template <> struct TraitsApp<GenesysApplication_if> {
	//static const bool runTests = false;
	static const TraceManager::Level traceLevel = TraceManager::Level::L9_mostDetailed;

	typedef ParallelAndDistributedManager Application;

	//// TESTS @TODO: To be replaced by googletest
	//typedef Tests Application;

	//// SMART SMARTs ALPHA SORTED
	//typedef Smart_AssignWriteSeizes Application;
	//typedef Smart_BatchSeparate Application;
	//typedef Smart_Clone Application;
	//typedef Smart_CppForG Application;
	//typedef Smart_Delay Application;
	//typedef Smart_Dummy Application;
	// typedef DistributedExecutionManager DistributedExecutionManagerApp;
	// typedef Benchmark BenchmarkApp;
	//typedef Smart_Failures Application;
	//typedef Smart_FiniteStateMachine Application;
	//typedef Smart_HoldSearchRemove Application;
	//typedef Smart_ModelInfoModelSimulation Application;
	//typedef Smart_ODE Application;
	//typedef Smart_OnEvent Application;
	//typedef Smart_Parser Application;
	//typedef Smart_ParserModelFunctions Application;
	//typedef Smart_Plugin Application;
	//typedef Smart_Process Application;
	//typedef Smart_ProcessSet Application;
	//typedef Smart_RouteStation Application;
	//typedef Smart_SeizeDelayRelease Application;
	//typedef Smart_SeizeDelayReleaseNoDataDefs Application;
	//typedef Smart_SeizeDelayReleaseMany Application;
	//typedef Smart_Sequence Application;
	//typedef Smart_SynchronizingParallelEntities Application;
	//typedef Smart_SimulationControlResponse Application;
	//typedef Smart_WaitScanCondition Application;
	//typedef Smart_WaitSignal Application;

	//// ARENA SMARTS
	//typedef Smart_AddingResource Application;
	//typedef Smart_AlternatingEntityCreation Application;
	//typedef Smart_ArrivalsElementStopsEntitiesArrivingAfterASetTime Application;
	//typedef Smart_ArrivalsEntityTypeVsAttribute Application;
	//typedef Smart_AssignExample Application;
	//typedef Smart_AutomaticStatisticsCollection Application;
	//typedef Smart_BasicModeling Application;
	//typedef Smart_BatchAndSeparate Application;
	//typedef Smart_ContinuousFlowEntities Application;
	//typedef Smart_Create Application;
	//typedef Smart_DecideNWayByChance Application;
	//typedef Smart_DefiningAttributesAsStrings Application;
	//typedef Smart_DefiningControlLogic Application;
	//typedef Smart_DefiningResourceCapacity Application;
	//typedef Smart_DelayBasedOnReplication Application;
	//typedef Smart_EntitiesProcessedByPriority Application;
	//typedef Smart_EvaluatingConditionsBeforeEnteringQueue Application;
	//typedef Smart_Expression Application;
	//typedef Smart_InventoryAndHoldingCosts Application;
	//typedef Smart_MaxArrivalsField Application;
	//typedef Smart_ModelRunUntil1000Parts Application;
	//typedef Smart_ModuleDisplayVariables Application;
	//typedef Smart_ParallelProcessingOfEntities Application;
	//typedef Smart_PlacingEntitiesInQueueSets Application;
	//typedef Smart_PriorityExample Application;
	//typedef Smart_ProcessArena Application;
	//typedef Smart_Record Application;
	//typedef Smart_ResourceCosting Application;
	//typedef Smart_ResourceScheduleCosting Application;
	//typedef Smart_ResourceSets Application;
	//typedef Smart_SeizingMultipleSimultaneosly Application;
	//typedef Smart_SelectingRouteBasedOnProbability Application;
	//typedef Smart_SelectingShorterQueue Application;
	//typedef Smart_SynchronizingParallelEntities Application;
	//typedef Smart_ValueAdded Application;
	//typedef Smart_WaitForSignal Application;

	//// ARENA EXAMPLES
	//typedef AirportSecurityExample.h Application;
	//typedef AirportSecurityExampleExtended.h Application;
	//typedef Example_Basic_Order_Shipping.h Application;
	//typedef Example_PublicTransport.h Application;


	//// TEACHING
	//typedef AnElectronicAssemblyAndTestSystem Application;
	//typedef OperatingSystem01 Application;
	//typedef OperatingSystem02 Application;
	//typedef OperatingSystem03 Application;

	//// BOOK
	//typedef Book_Cap02_Example01 Application;

	//// TERMINAL APPLICATIONS
	//typedef GenesysTerminalApp Application;
};

#endif /* TRAITSAPP_H */

