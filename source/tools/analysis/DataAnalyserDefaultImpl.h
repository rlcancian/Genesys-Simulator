#ifndef DATAANALYSERDEFAULTIMPL_H
#define DATAANALYSERDEFAULTIMPL_H

#include "DataAnalyser_if.h"
#include "TraitsAnalysis.h"

#include <string>

/**
 * @brief Default implementation of DataAnalyser_if.
 *
 * Current scope:
 * - Dataset lifecycle orchestration.
 * - Fitter access (initial implementation phase).
 *
 * Planned scope:
 * - Sampler integration.
 * - ExperimentManager integration.
 * - HypothesisTester integration.
 *
 * Architectural notes:
 * - Dependencies are injected externally.
 * - This class does not own injected services.
 * - Null services are temporarily allowed during phased implementation.
 */
class DataAnalyserDefaultImpl : public DataAnalyser_if {
public:
	explicit DataAnalyserDefaultImpl(
		Fitter_if* fitter = nullptr,
		Sampler_if* sampler = nullptr,
		ExperimentManager_if* experimenter = nullptr,
		HypothesisTester_if* tester = nullptr
	);

	virtual ~DataAnalyserDefaultImpl() = default;

public:
	virtual bool loadDataSet(std::string datafilename) override;
	virtual bool saveDataSet(std::string datasetname) override;
	virtual void newDataSet(std::string datasetname, std::string datafilename) override;

	virtual Fitter_if* fitter() override;
	virtual Sampler_if* sampler() override;
	virtual ExperimentManager_if* experimenter() override;
	virtual HypothesisTester_if* tester() override;

private:
	TraitsAnalysis<Fitter_if>::Implementation _defaultFitter;
	TraitsAnalysis<HypothesisTester_if>::Implementation _defaultTester;

	// Injected dependencies (non-owning)
	Fitter_if* _fitter = nullptr;
	Sampler_if* _sampler = nullptr;
	ExperimentManager_if* _experimenter = nullptr;
	HypothesisTester_if* _tester = nullptr;

	// Dataset state
	std::string _datasetName = "";
	std::string _dataFilename = "";
};

#endif /* DATAANALYSERDEFAULTIMPL_H */
