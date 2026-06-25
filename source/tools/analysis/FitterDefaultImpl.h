#ifndef FITTERDEFAULTIMPL_H
#define FITTERDEFAULTIMPL_H

#include "DatasetLoader.h"
#include "Fitter_if.h"

#include <functional>
#include <string>
#include <vector>

/**
 * @brief Default kernel-independent distribution fitter.
 *
 * The implementation estimates candidate parameters directly from the current
 * DatasetLoader snapshot and ranks candidates by EDF/CDF squared error. It
 * preserves the legacy pointer-output API while also exposing FitSummary.
 */
class FitterDefaultImpl : public Fitter_if {
public:
	/** @brief Creates an empty fitter with no configured dataset. */
	FitterDefaultImpl() = default;
	/** @brief Destroys the fitter without owning external resources. */
	virtual ~FitterDefaultImpl() = default;

	/** @brief Applies the legacy EDF/CDF SSE heuristic for normal adherence. */
	virtual bool isNormalDistributed(double confidencelevel) override;
	/** @brief Fits a uniform distribution using sample minimum and maximum. */
	virtual void fitUniform(double* sqrerror, double* min, double* max) override;
	/** @brief Fits a triangular distribution from min, mean-derived mode and max. */
	virtual void fitTriangular(double* sqrerror, double* min, double* mo, double* max) override;
	/** @brief Fits a normal distribution using sample mean and standard deviation. */
	virtual void fitNormal(double* sqrerror, double* avg, double* stddev) override;
	/** @brief Fits an exponential distribution using the sample mean. */
	virtual void fitExpo(double* sqrerror, double* avg1) override;
	/** @brief Fits an Erlang distribution using moment-based phase estimation. */
	virtual void fitErlang(double* sqrerror, double* avg, double* m) override;
	/** @brief Fits a scaled beta distribution by moment estimates. */
	virtual void fitBeta(double* sqrerror, double* alpha, double* beta, double* infLimit, double* supLimit) override;
	/** @brief Fits a Weibull distribution by coefficient-of-variation inversion. */
	virtual void fitWeibull(double* sqrerror, double* alpha, double* scale) override;
	/** @brief Writes the best fit name and squared error through legacy outputs. */
	virtual void fitAll(double* sqrerror, std::string* name) override;
	/** @brief Returns all candidate fits ordered by squared error. */
	virtual FitSummary fitAllSummary() override;
	/** @brief Loads and records a file-backed dataset for fitting. */
	virtual void setDataFilename(std::string dataFilename) override;
	/** @brief Configures an in-memory dataset for fitting. */
	virtual bool setData(const std::vector<double>& data) override;
	/** @brief Configures an in-memory dataset while preserving source filename. */
	virtual bool setData(const std::vector<double>& data, std::string dataFilename) override;
	/** @brief Returns the current dataset source filename, when known. */
	virtual std::string getDataFilename() override;

private:
	/** @brief Returns whether the current dataset is ready for fitting. */
	bool _hasUsableDataset() const;
	/**
	 * @brief Computes the EDF/CDF squared error using Hazen plotting positions.
	 */
	void _setSse(double* sqrerror, const std::function<double(double)>& cdf);
	/** @brief Evaluates a beta PDF while guarding invalid boundary values. */
	static double _betaPdfSafe(double x, double alpha, double beta);
	/** @brief Computes a scaled beta CDF by numerical integration. */
	double _betaCdfScaled(double x, double alpha, double beta, double infLimit, double supLimit) const;
	/** @brief Computes a Weibull CDF for fitted parameters. */
	double _weibullCdf(double x, double alpha, double scale) const;
	/** @brief Estimates scaled beta shape and range parameters from moments. */
	bool _estimateScaledBetaMoments(double* alpha, double* beta, double* infLimit, double* supLimit);
	/** @brief Estimates Weibull shape from a coefficient of variation. */
	bool _estimateWeibullShapeFromCv(double cv, double* shape) const;

private:
	std::string _dataFilename;
	DatasetLoader _dataset;
};

#endif /* FITTERDEFAULTIMPL_H */
