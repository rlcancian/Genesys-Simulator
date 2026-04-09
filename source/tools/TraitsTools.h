/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/*
 * File:   TraitsTools.h
 * Author: rlcancian
 *
 * Created on 3 de maio de 2022, 18:57
 */

#ifndef TRAITSTOOLS_H
#define TRAITSTOOLS_H

// TOOLS
#include "SolverDefaultImpl1.h"
#include "HypothesisTesterDefaultImpl1.h"
#include "HypothesisTester_if.h"
#include "FitterDummyImpl.h"
#include "Fitter_if.h"

/**
 * @brief Registry-like traits to bind abstractions to concrete tools classes.
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
struct TraitsTools {
};

/*!
 *  Configure the Solver to be used
 */
template <> struct TraitsTools<Solver_if> {
	typedef SolverDefaultImpl1 Implementation;
	static constexpr double Precision = 1e-5;
	static constexpr unsigned int MaxSteps = 1e2;
};

/*!
 *  Configure the Hypothesis Tester to be used
 */
template <> struct TraitsTools<HypothesisTester_if> {
	typedef HypothesisTesterDefaultImpl1 Implementation;
	static constexpr unsigned int ConfidenceLevel = 95;
};

/*!
 *  Configure the Fitter to be used
 */
template <> struct TraitsTools<Fitter_if> {
	typedef FitterDummyImpl Implementation;
};

#endif /* TRAITSTOOLS_H */
