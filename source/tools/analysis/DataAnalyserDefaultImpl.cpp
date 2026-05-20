#include "DataAnalyserDefaultImpl.h"

DataAnalyserDefaultImpl::DataAnalyserDefaultImpl(
	Fitter_if* fitter,
	Sampler_if* sampler,
	ExperimentManager_if* experimenter,
	HypothesisTester_if* tester
) :
	_fitter(fitter),
	_sampler(sampler),
	_experimenter(experimenter),
	_tester(tester) {
}

bool DataAnalyserDefaultImpl::loadDataSet(std::string datafilename) {
	_dataFilename = datafilename;
    _fitter->setDataFilename(datafilename);

	return true;
}

bool DataAnalyserDefaultImpl::saveDataSet(std::string datasetname) {
	// TODO: Implement dataset persistence
    throw std::runtime_error("DataAnalyserDefaultImpl::saveDataSet not implemented");
}

void DataAnalyserDefaultImpl::newDataSet(std::string datasetname, std::string datafilename) {
	// TODO: Implement dataset creation lifecycle
    throw std::runtime_error("DataAnalyserDefaultImpl::newDataSet not implemented");
}

Fitter_if* DataAnalyserDefaultImpl::fitter() {
	return _fitter;
}

Sampler_if* DataAnalyserDefaultImpl::sampler() {
	return _sampler;
}

ExperimentManager_if* DataAnalyserDefaultImpl::experimenter() {
	return _experimenter;
}

HypothesisTester_if* DataAnalyserDefaultImpl::tester() {
	return _tester;
}