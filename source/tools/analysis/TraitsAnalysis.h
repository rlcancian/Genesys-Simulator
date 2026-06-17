/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/*
 * File:   TraitsAnalysis.h
 * Author: rlcancian
 *
 * Created on 3 de maio de 2022, 18:57
 */

#ifndef TRAITSANALYSIS_H
#define TRAITSANALYSIS_H

// TOOLS
#include "tools/SolverDefaultImpl1.h"
#include "HypothesisTesterDefaultImpl.h"
#include "HypothesisTester_if.h"
#include "FitterDefaultImpl.h"
#include "Fitter_if.h"

/**
 * @brief Registry-like traits to bind analysis abstractions to concrete classes.
 *
 * Architectural role:
 * - Central selection point for concrete implementations used by this package.
 *
 * Implementation status:
 * - Currently covers only a subset of tool abstractions.
 *
 * Planned evolution:
 * - Extend bindings as new stable interfaces receive concrete implementations.
 * - New abstractions without safe concrete implementations should remain
 *   documented-only for now to avoid build instability.
 */
template <typename T>
struct TraitsAnalysis {
};

class ProbabilityDistribution;

/*!
 * Configure numerical constants used by probability distribution helpers.
 */
template <> struct TraitsAnalysis<ProbabilityDistribution> {
	static constexpr unsigned int IntegrationIntervals = 8192U;
	static constexpr unsigned int MaxBracketExpansions = 128U;
	static constexpr unsigned int MaxBisectionIterations = 120U;
	static constexpr double CdfTolerance = 1e-8;
	static constexpr double QuantileTolerance = 1e-10;
	static constexpr double NormalTailBreakPoint = 0.02425;
	static constexpr double TStudentNormalApproximationDegreesFreedom = 341.0;
};

/*!
 *  Configure the Solver to be used
 */
template <> struct TraitsAnalysis<Solver_if> {
	typedef SolverDefaultImpl1 Implementation;
	static constexpr double Precision = 1e-5;
	static constexpr unsigned int MaxSteps = 1e2;
};

/*!
 *  Configure the Hypothesis Tester to be used
 */
template <> struct TraitsAnalysis<HypothesisTester_if> {
	typedef HypothesisTesterDefaultImpl Implementation;
	static constexpr unsigned int ConfidenceLevel = 95;
};

/*!
 *  Configure the Fitter to be used
 */
template <> struct TraitsAnalysis<Fitter_if> {
	typedef FitterDefaultImpl Implementation;
};

#endif /* TRAITSANALYSIS_H */
