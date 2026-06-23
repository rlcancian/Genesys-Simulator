#ifndef FITTER_IF_H
#define FITTER_IF_H

#include <string>
#include <vector>

struct FittedParameter {
	std::string name;
	double value = 0.0;
};

struct FittingResult {
	std::string distributionName;
	bool success = false;
	double squaredError = 0.0;
	std::vector<FittedParameter> parameters;
	std::string message;
};

struct FitSummary {
	bool success = false;
	FittingResult bestFit;
	std::vector<FittingResult> ranking;
};

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
	 * - Data source must be configured through setDataFilename() or setData()
	 *   before fitting.
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
	/** @brief Destroys the fitter interface. */
	virtual ~Fitter_if() = default;
	/** @brief Checks normal adherence through the implementation's heuristic. */
	virtual bool isNormalDistributed(double confidencelevel) = 0;
	/** @brief Fits a uniform distribution and writes its parameters. */
	virtual void fitUniform(double *sqrerror, double *min, double *max) = 0;
	/** @brief Fits a triangular distribution and writes its parameters. */
	virtual void fitTriangular(double *sqrerror, double *min, double *mo, double *max) = 0;
	/** @brief Fits a normal distribution and writes its parameters. */
	virtual void fitNormal(double *sqrerror, double *avg, double *stddev) = 0;
	/** @brief Fits an exponential distribution and writes its parameter. */
	virtual void fitExpo(double *sqrerror, double *avg1) = 0;
	/** @brief Fits an Erlang distribution and writes its parameters. */
	virtual void fitErlang(double *sqrerror, double *avg, double *m) = 0;
	/** @brief Fits a scaled beta distribution and writes its parameters. */
	virtual void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit) = 0;
	/** @brief Fits a Weibull distribution and writes its parameters. */
	virtual void fitWeibull(double *sqrerror, double *alpha, double *scale) = 0;
	/** @brief Fits all supported candidates and writes the best legacy result. */
	virtual void fitAll(double *sqrerror, std::string *name) = 0;
	/** @brief Fits all supported candidates and returns a structured ranking. */
	virtual FitSummary fitAllSummary() = 0;
public:
	/**
	 * @brief Defines the dataset source filename used by fitting operations.
	 */
	virtual void setDataFilename(std::string dataFilename) = 0;
	/**
	 * @brief Defines the in-memory dataset used by fitting operations.
	 */
	virtual bool setData(const std::vector<double>& data) = 0;
	/**
	 * @brief Defines the in-memory dataset and records its source filename.
	 */
	virtual bool setData(const std::vector<double>& data, std::string dataFilename) {
		(void) dataFilename;
		return setData(data);
	}
	/**
	 * @brief Retrieves the currently configured dataset source filename.
	 */
	virtual std::string getDataFilename() = 0;
};

#endif /* FITTER_IF_H */
