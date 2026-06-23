#ifndef FITTERDUMMYIMPL_H
#define FITTERDUMMYIMPL_H

#include "Fitter_if.h"

/**
 * @brief Placeholder implementation of Fitter_if.
 *
 * Purpose:
 * - Keep binary/source compatibility while a functional fitter implementation
 *   is not fully available.
 *
 * Implementation status:
 * - Dummy/stub component; not a functional fitting engine.
 * - Kept only for legacy callers that still instantiate it explicitly.
 */
class FitterDummyImpl : public Fitter_if {
public:
	/** @brief Creates a legacy dummy fitter. */
	FitterDummyImpl();
	/** @brief Copies a legacy dummy fitter. */
	FitterDummyImpl(const FitterDummyImpl& orig);
	/** @brief Destroys the dummy fitter. */
	~FitterDummyImpl();
public:
	/** @brief Always reports that the data is not normally distributed. */
	bool isNormalDistributed(double confidencelevel);
	/** @brief Writes failure outputs for uniform fitting. */
	void fitUniform(double *sqrerror, double *min, double *max);
	/** @brief Writes failure outputs for triangular fitting. */
	void fitTriangular(double *sqrerror, double *min, double *mo, double *max);
	/** @brief Writes failure outputs for normal fitting. */
	void fitNormal(double *sqrerror, double *avg, double *stddev);
	/** @brief Writes failure outputs for exponential fitting. */
	void fitExpo(double *sqrerror, double *avg1);
	/** @brief Writes failure outputs for Erlang fitting. */
	void fitErlang(double *sqrerror, double *avg, double *m);
	/** @brief Writes failure outputs for beta fitting. */
	void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit);
	/** @brief Writes failure outputs for Weibull fitting. */
	void fitWeibull(double *sqrerror, double *alpha, double *scale);
	/** @brief Writes failure outputs for all-distribution fitting. */
	void fitAll(double *sqrerror, std::string *name);
	/** @brief Returns a structured failure summary. */
	FitSummary fitAllSummary();
public:
	/** @brief Records the data filename without loading a real dataset. */
	void setDataFilename(std::string dataFilename);
	/** @brief Rejects an in-memory dataset because this fitter is a stub. */
	bool setData(const std::vector<double>& data);
	/** @brief Rejects an in-memory dataset while recording the source filename. */
	bool setData(const std::vector<double>& data, std::string dataFilename);
	/** @brief Returns the filename last provided to the dummy fitter. */
	std::string getDataFilename();
private:
	std::string _dataFilename = "";
};

#endif /* FITTERDUMMYIMPL_H */
