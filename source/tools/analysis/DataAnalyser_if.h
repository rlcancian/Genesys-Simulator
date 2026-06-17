/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/*
 * File:   DataAnalyser.h
 * Author: rlcancian
 *
 * Created on 17 de maio de 2022, 11:06
 */

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

struct HistogramBin {
	double lowerLimit = 0.0;
	double upperLimit = 0.0;
	std::size_t frequency = 0;
	double relativeFrequency = 0.0;
};

struct DataSetHistogram {
	bool usable = false;
	std::size_t count = 0;
	double min = 0.0;
	double max = 0.0;
	double classWidth = 0.0;
	std::vector<HistogramBin> bins;
};

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
 * @brief High-level façade for applied statistical analysis over datasets.
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
 * - A consolidated default concrete implementation is still pending.
 */
class DataAnalyser_if {
public:
	virtual ~DataAnalyser_if() = default;
	virtual bool loadDataSet(std::string datafilename) = 0;
	virtual bool loadDataSet(const std::vector<double>& data) = 0;
	virtual DataSetSummary summary() const = 0;
	virtual DataSetHistogram histogram(std::size_t classCount = 0) const = 0;
	virtual DataSetBoxPlot boxplot() const = 0;
	virtual bool saveDataSet(std::string datasetname) = 0;
	virtual void newDataSet(std::string datasetname, std::string datafilename) = 0;
	virtual Fitter_if* fitter() = 0;
	virtual Sampler_if* sampler() = 0;
	virtual ExperimentManager_if* experimenter() = 0;
	virtual HypothesisTester_if* tester() = 0;
};


#endif /* DATAANALYSERIF_H */
