#ifndef DATAANALYSERDEFAULTIMPL_H
#define DATAANALYSERDEFAULTIMPL_H

#include "DataAnalyser_if.h"
#include "DatasetLoader.h"
#include "TraitsAnalysis.h"

#include <string>
#include <vector>

/**
 * @brief Default implementation of DataAnalyser_if.
 *
 * Current scope:
 * - Dataset loading from file and memory.
 * - Descriptive summary, histogram and boxplot generation.
 * - Access to the default fitter and hypothesis tester.
 *
 * Roadmap scope:
 * - Dataset creation/persistence lifecycle.
 * - Sampler integration.
 * - ExperimentManager integration.
 *
 * Architectural notes:
 * - Default fitter/tester instances are owned by this facade.
 * - Externally injected services are non-owning and may outlive the facade.
 * - Sampler and experimenter remain optional collaborators in this phase.
 */
class DataAnalyserDefaultImpl : public DataAnalyser_if {
public:
	/** @brief Builds a facade with default or externally provided collaborators. */
	explicit DataAnalyserDefaultImpl(
		Fitter_if* fitter = nullptr,
		Sampler_if* sampler = nullptr,
		ExperimentManager_if* experimenter = nullptr,
		HypothesisTester_if* tester = nullptr
	);

	/** @brief Destroys the facade without owning injected collaborators. */
	virtual ~DataAnalyserDefaultImpl() = default;

public:
	/** @brief Loads a numeric dataset from file and forwards it to the fitter. */
	virtual bool loadDataSet(std::string datafilename) override;
	/** @brief Loads a numeric dataset from memory and forwards it to the fitter. */
	virtual bool loadDataSet(const std::vector<double>& data) override;
	/** @brief Returns the currently loaded data in input order. */
	virtual const std::vector<double>& data() const override;
	/** @brief Returns the currently loaded data in sorted order. */
	virtual const std::vector<double>& sortedData() const override;
	/** @brief Returns the current dataset's descriptive summary. */
	virtual DataSetSummary summary() const override;
	/** @brief Returns a numeric histogram for the current dataset. */
	virtual DataSetHistogram histogram(std::size_t classCount = 0) const override;
	/** @brief Returns quartiles, whiskers and outliers for the current dataset. */
	virtual DataSetBoxPlot boxplot() const override;
	/** @brief Reports unsupported dataset persistence for this delivery scope. */
	virtual bool saveDataSet(std::string datasetname) override;
	/** @brief Reports unsupported dataset creation lifecycle for this scope. */
	virtual void newDataSet(std::string datasetname, std::string datafilename) override;

	/** @brief Returns the active distribution fitter. */
	virtual Fitter_if* fitter() override;
	/** @brief Returns the injected sampler or throws for the roadmap path. */
	virtual Sampler_if* sampler() override;
	/** @brief Returns the injected experiment manager or throws for roadmap path. */
	virtual ExperimentManager_if* experimenter() override;
	/** @brief Returns the active hypothesis tester. */
	virtual HypothesisTester_if* tester() override;

private:
	TraitsAnalysis<Fitter_if>::Implementation _defaultFitter;
	TraitsAnalysis<HypothesisTester_if>::Implementation _defaultTester;

	// Active service pointers. They either reference the owned defaults above
	// or non-owning collaborators supplied by the caller.
	Fitter_if* _fitter = nullptr;
	Sampler_if* _sampler = nullptr;
	ExperimentManager_if* _experimenter = nullptr;
	HypothesisTester_if* _tester = nullptr;

	// Current validated dataset snapshot shared by summary/histogram/boxplot
	// and pushed into the fitter whenever loadDataSet succeeds.
	DatasetLoader _dataset;
	std::string _datasetName = "";
	std::string _dataFilename = "";
};

#endif /* DATAANALYSERDEFAULTIMPL_H */
