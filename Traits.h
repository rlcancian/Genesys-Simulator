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

#ifndef TRAITS_H
#define TRAITS_H

// \todo: Separate into Traits.h and KernelTraits.h

// interfaces
#include "Collector_if.h"
#include "ExperimentDesign_if.h"
#include "Fitter_if.h"
#include "GenesysApplication_if.h"
#include "HypothesisTester_if.h"
#include "Integrator_if.h"
#include "Model.h"
#include "ModelChecker_if.h"
#include "ModelPersistence_if.h"
#include "Parser_if.h"
#include "PluginConnector_if.h"
#include "Prob_Distrib_if.h"
#include "ExperimetManager_if.h"
#include "Sampler_if.h"
#include "Statistics_if.h"
#include "SimulationReporter_if.h"

//  Default implementations
//statistics
#include "CollectorDefaultImpl1.h"
#include "CollectorDatafileDefaultImpl1.h"
#include "HypothesisTesterDefaultImpl1.h"
#include "IntegratorDefaultImpl1.h"
#include "ProbDistribDefaultImpl1.h"
#include "ProbDistribBoostImpl.h"
#include "SamplerDefaultImpl1.h"
#include "StatisticsDefaultImpl1.h"
//simulator and parts
#include "parserBisonFlex/ParserDefaultImpl2.h"
#include "PluginConnectorDummyImpl1.h"
//model and parts
#include "ModelCheckerDefaultImpl1.h"
#include "ModelPersistenceDefaultImpl1.h"
#include "SimulationReporterDefaultImpl1.h"
//tools
#include "ExperimentDesignDefaultImpl1.h"
#include "FitterDefaultImpl1.h"
#include "ExperimentManagerDefaultImpl1.h"


// simulationReporter issues
#include "Counter.h"

// genesys applications
#include "FullSimulationOfComplexModel.h"
//#include "BuildSimulationModel03.h"
#include "Model_CreateDelayDispose.h"
#include "Model_CreteDelayDispose2.h"
#include "Model_AssignWrite3Seizes.h"
#include "GenesysGUI.h"
#include "GenesysConsole.h"
#include "Model_SeizeDelayRelease1.h"
#include "Model_SeizeDelayReleaseMany.h"
#include "Model_StatationRouteSequence.h"
#include "Modelo_SistemaOperacional02.h"
#include "Modelo_SistemaOperacional03.h"
#include "TestEnterLeaveRoute.h"
#include "TestFunctions.h"
#include "TestSimulationControlAndSimulationResponse.h"
//#include "TestLSODE.h"
//#include "TestMarkovChain.h"
#include "TestMatricesOfAttributesAndVariables.h"

template <typename T>
struct Traits {
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};

/*
 *  KERNEL STUFF
 */

/*
 *  Genesys Application
 */

template <> struct Traits<GenesysApplication_if> {
	//typedef Model_CreateDelayDispose Application;
	//typedef Model_SeizeDelayRelease1 Application;
	//typedef Model_SeizeDelayReleaseMany Application;
	////typedef Model_AssignWrite3Seizes Application;
	//typedef Model_StatationRouteSequence Application;
	typedef Modelo_SistemaOperacional02 Application;
	//typedef Modelo_SistemaOperacional03 Application;
};

/*
 *  Simulator and Simulator Parts
 */


template <> struct Traits<PluginConnector_if> {
	typedef PluginConnectorDummyImpl1 Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L4_warning;
};

template <> struct Traits<Parser_if> {
	typedef ParserDefaultImpl2 Implementation;
};

/*
 *  Model and Model Parts
 */

template <> struct Traits<Model> {
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L4_warning;
};

template <> struct Traits<ModelPersistence_if> {
	typedef ModelPersistenceDefaultImpl1 Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};

template <> struct Traits<SimulationReporter_if> {
	typedef SimulationReporterDefaultImpl1 Implementation;
	typedef Counter CounterImplementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};

template <> struct Traits<ModelChecker_if> {
	typedef ModelCheckerDefaultImpl1 Implementation;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};

template <> struct Traits<ModelComponent> {
	typedef StatisticsDefaultImpl1 StatisticsCollector_StatisticsImplementation;
	typedef CollectorDefaultImpl1 StatisticsCollector_CollectorImplementation;
	static constexpr bool reportStatistics = true;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};
template <> struct Traits<ModelElement> {
	static constexpr bool reportStatistics = true;
	static const Util::TraceLevel traceLevel = Util::TraceLevel::L3_results;
};

/*
 Statistics
 */

template <> struct Traits<Collector_if> {
	typedef CollectorDatafileDefaultImpl1 Implementation;
};

template <> struct Traits<Statistics_if> {
	typedef StatisticsDefaultImpl1 Implementation;
	typedef CollectorDefaultImpl1 CollectorImplementation;
	static constexpr double SignificanceLevel = 0.05;
};

template <> struct Traits<Integrator_if> {
	typedef IntegratorDefaultImpl1 Implementation;
	static constexpr unsigned int MaxIterations = 1e3;
	static constexpr double Precision = 1e-9;
};

template <> struct Traits<Sampler_if> {
	typedef SamplerDefaultImpl1 Implementation;
	typedef SamplerDefaultImpl1::DefaultImpl1RNG_Parameters Parameters;
};

template <> struct Traits<ProbDistrib_if> {
	typedef ProbDistribDefaultImpl1 Implementation;
};

template <> struct Traits<Fitter_if> {
	typedef FitterDefaultImpl1 Implementation;
};

template <> struct Traits<HypothesisTester_if> {
	typedef IntegratorDefaultImpl1 IntegratorImplementation;
	typedef HypothesisTesterDefaultImpl1 Implementation;
};

/*
 * Tools
 */

template <> struct Traits<ExperimentDesign_if> {
	typedef ExperimentDesignDefaultImpl1 Implementation;
};

template <> struct Traits<ExperimentManager_if> {
	typedef ExperimentManagerDefaultImpl1 Implementation;
};

#endif /* TRAITS_H */

