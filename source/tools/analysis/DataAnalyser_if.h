#ifndef DATAANALYSERIF_H
#define DATAANALYSERIF_H

#include <cstddef>
#include <string>
#include <vector>
#include "Fitter_if.h"
#include "HypothesisTester_if.h"

class Sampler_if;
class ExperimentManager_if;

struct DataSetSummary {
	bool usable = false;
	std::size_t count = 0;
	double min = 0.0;
	double max = 0.0;
	double mean = 0.0;
	double variance = 0.0;
	double stddev = 0.0;
	bool hasNegativeData = false;
};

/**
 * @brief One numeric histogram class produced by DataAnalyser_if::histogram().
 */
struct HistogramBin {
	double lowerLimit = 0.0;
	double upperLimit = 0.0;
	std::size_t frequency = 0;
	double relativeFrequency = 0.0;
};

/**
 * @brief Numeric histogram without plotting concerns.
 */
struct DataSetHistogram {
	bool usable = false;
	std::size_t count = 0;
	double min = 0.0;
	double max = 0.0;
	double classWidth = 0.0;
	std::vector<HistogramBin> bins;
};

/**
 * @brief Five-number summary plus 1.5-IQR fences and outliers.
 */
struct DataSetBoxPlot {
	bool usable = false;
	std::size_t count = 0;
	double min = 0.0;
	double firstQuartile = 0.0;
	double median = 0.0;
	double thirdQuartile = 0.0;
	double max = 0.0;
	double interquartileRange = 0.0;
	double lowerFence = 0.0;
	double upperFence = 0.0;
	double lowerWhisker = 0.0;
	double upperWhisker = 0.0;
	std::vector<double> outliers;
};

/**
 * @brief High-level facade for applied statistical analysis over datasets.
 *
 * Purpose:
 * - Coordinate dataset lifecycle and analysis services in a single entry point.
 *
 * Architectural role:
 * - Facade that orchestrates fitting, sampling, experiment execution and
 *   hypothesis testing.
 *
 * Ownership/lifetime:
 * - The returned pointers from fitter(), sampler(), experimenter() and tester()
 *   are non-owning views. Concrete implementations must document whether those
 *   services are internally owned singletons, members, or externally injected
 *   objects.
 *
 * Implementation status:
 * - Interface contract is stable and legacy-compatible.
 * - DataAnalyserDefaultImpl is the consolidated default implementation.
 */
class DataAnalyser_if {
public:
	/** @brief Destroys the facade interface. */
	virtual ~DataAnalyser_if() = default;
	/** @brief Loads a dataset from a numeric file. */
	virtual bool loadDataSet(std::string datafilename) = 0;
	/** @brief Loads a dataset directly from memory. */
	virtual bool loadDataSet(const std::vector<double>& data) = 0;
	/** @brief Returns the current dataset in input order. */
	virtual const std::vector<double>& data() const = 0;
	/** @brief Returns the current dataset in sorted order. */
	virtual const std::vector<double>& sortedData() const = 0;
	/** @brief Returns the current dataset's descriptive summary. */
	virtual DataSetSummary summary() const = 0;
	/** @brief Returns the current dataset's numeric histogram. */
	virtual DataSetHistogram histogram(std::size_t classCount = 0) const = 0;
	/** @brief Returns the current dataset's boxplot statistics. */
	virtual DataSetBoxPlot boxplot() const = 0;
	/** @brief Persists the current dataset when supported by an implementation. */
	virtual bool saveDataSet(std::string datasetname) = 0;
	/** @brief Starts or registers a new dataset when supported by an implementation. */
	virtual void newDataSet(std::string datasetname, std::string datafilename) = 0;
	/** @brief Returns the active distribution fitter service. */
	virtual Fitter_if* fitter() = 0;
	/** @brief Returns the active sampler service. */
	virtual Sampler_if* sampler() = 0;
	/** @brief Returns the active experiment manager service. */
	virtual ExperimentManager_if* experimenter() = 0;
	/** @brief Returns the active hypothesis tester service. */
	virtual HypothesisTester_if* tester() = 0;
};


#endif /* DATAANALYSERIF_H */
