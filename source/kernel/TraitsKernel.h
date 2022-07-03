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


#ifndef TRAITSKERNEL_H
#define TRAITSKERNEL_H


#include "simulator/ModelPersistenceDefaultImpl1.h"
#include "simulator/SimulationReporterDefaultImpl1.h"
#include "simulator/Counter.h"
#include "simulator/ModelCheckerDefaultImpl1.h"
#include "simulator/PluginConnectorDummyImpl1.h"
#include "simulator/ParserDefaultImpl2.h"
#include "simulator/SimulationReporter_if.h"

#include "statistics/CollectorDatafileDefaultImpl1.h"
#include "statistics/CollectorDefaultImpl1.h"
#include "statistics/StatisticsDefaultImpl1.h"
#include "statistics/StatisticsDataFileDefaultImpl.h"
#include "statistics/SamplerDefaultImpl1.h"

#include "network/NetworkIPv4Imp1.h"
#include "network/NetworkSchedulerStatic.h"

//namespace GenesysKernel {

template <typename T>
struct TraitsKernel {
};

/*
 *  Simulation and Simulation Parts
 */

template <> struct TraitsKernel<Simulator> {
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L9_mostDetailed;
};

template <> struct TraitsKernel<SimulationReporter_if> {
	typedef SimulationReporterDefaultImpl1 Implementation;
	typedef Counter CounterImplementation;
	static constexpr bool showSimulationResponses = false;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L2_results;
};

template <> struct TraitsKernel<PluginConnector_if> {
	typedef PluginConnectorDummyImpl1 Implementation;
	//typedef PluginConnectorDynamicLibraryLoader Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L4_warning;
};

template <> struct TraitsKernel<Parser_if> {
	typedef ParserDefaultImpl2 Implementation;
};

/*
 *  Model and Model Parts
 */

template <> struct TraitsKernel<Model> {
	typedef StatisticsDefaultImpl1 StatisticsCollector_StatisticsImplementation;
	typedef CollectorDefaultImpl1 StatisticsCollector_CollectorImplementation;
	static constexpr bool automaticallyCreatesModelDatas = true; //@TODO: Need to be supported by every component
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L5_event;
};

template <> struct TraitsKernel<ModelComponent> {
	static constexpr bool reportStatistics = true;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L2_results;
};

template <> struct TraitsKernel<ModelDataDefinition> {
	static constexpr bool reportStatistics = true;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L2_results;
};

template <> struct TraitsKernel<ModelChecker_if> {
	typedef ModelCheckerDefaultImpl1 Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L2_results;
};

template <> struct TraitsKernel<ModelPersistence_if> {
	typedef ModelPersistenceDefaultImpl1 Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L2_results;
};

/*
 *  Statistics
 */
template <> struct TraitsKernel<Statistics_if> {
	typedef double DataType; // TODO: not used yet. Change all classes that collect statistics to this type (so classes that deal with erros and bit limits can be assigned to it
	static constexpr double SignificanceLevel = 0.05;
};

template <> struct TraitsKernel<StatisticsDatafile_if> {
	typedef StatisticsDatafileDefaultImpl1 Implementation;
	typedef CollectorDatafileDefaultImpl1 CollectorImplementation;
	static constexpr double SignificanceLevel = 0.05;
};

template <> struct TraitsKernel<Sampler_if> {
	typedef SamplerDefaultImpl1 Implementation;
	typedef SamplerDefaultImpl1::DefaultImpl1RNG_Parameters Parameters;
};

template <> struct TraitsKernel<Collector_if> {
	typedef CollectorDatafileDefaultImpl1 Implementation;
	typedef double DataType; // TODO: not used yet. Change all classes that collect statistics to this type (so classes that deal with erros and bit limits can be assigned to it
};
/*
 *  Network
 */
template <> struct TraitsKernel<Network_if> {
	typedef NetworkIPv4Imp1 Implementation;
	typedef NetworkIPv4Imp1::Socket_Data Socket_Data;
};

template <> struct TraitsKernel<NetworkScheduler_if> {
	typedef NetworkSchedulerStatic Implementation;
	typedef NetworkSchedulerStatic::Scheduler_Info Scheduler_Info;
};
//namespace\\}

#endif /* TRAITSKERNEL_H */

