/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Fitter_if.h
 * Author: cancian
 *
 * Created on 14 de Agosto de 2018, 14:05
 */

#ifndef FITTER_IF_H
#define FITTER_IF_H

#include <string>

/**
 * @brief Interface for fitting theoretical distributions to real datasets.
 *
 * Purpose:
 * - Infer parameters of candidate probability distributions from sample data.
 *
 * Architectural role:
 * - Core fitting abstraction consumed by analysis workflows.
 *
 * Mathematical meaning:
 * - Each fit* method estimates family parameters and writes an adherence/error
 *   measure in @p sqrerror (typically squared-error-like criterion).
 * - fitAll() selects the best candidate according to implementation-defined
 *   criterion.
 * - isNormalDistributed() should be interpreted as a normality adherence check
 *   under the given confidence level.
 *
 * Preconditions:
 * - Output pointers must be valid non-null addresses.
 * - Data source must be configured through setDataFilename() before fitting.
 *
 * Postconditions:
 * - Output pointers are filled with estimated values when operation succeeds.
 *
 * Failure modes:
 * - Concrete implementations may return conservative defaults and/or report
 *   failure when insufficient data or unsupported models are provided.
 *
 * Legacy compatibility:
 * - The pointer-based output API is intentionally preserved as a legacy public
 *   contract; it is not necessarily the final long-term shape.
 */
class Fitter_if {
public:
	virtual ~Fitter_if() = default;
	virtual bool isNormalDistributed(double confidencelevel) = 0;
	virtual void fitUniform(double *sqrerror, double *min, double *max) = 0;
	virtual void fitTriangular(double *sqrerror, double *min, double *mo, double *max) = 0;
	virtual void fitNormal(double *sqrerror, double *avg, double *stddev) = 0;
	virtual void fitExpo(double *sqrerror, double *avg1) = 0;
	virtual void fitErlang(double *sqrerror, double *avg, double *m) = 0;
	virtual void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit) = 0;
	virtual void fitWeibull(double *sqrerror, double *alpha, double *scale) = 0;
	virtual void fitAll(double *sqrerror, std::string *name) = 0;
public:
	/**
	 * @brief Defines the dataset source filename used by fitting operations.
	 */
	virtual void setDataFilename(std::string dataFilename) = 0;
	/**
	 * @brief Retrieves the currently configured dataset source filename.
	 */
	virtual std::string getDataFilename() = 0;
};

#endif /* FITTER_IF_H */
