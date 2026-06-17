#include "DataAnalyserDefaultImpl.h"
#include "DatasetLoader.h"

#include <stdexcept>

DataAnalyserDefaultImpl::DataAnalyserDefaultImpl(
	Fitter_if* fitter,
	Sampler_if* sampler,
	ExperimentManager_if* experimenter,
	HypothesisTester_if* tester
) :
	_fitter(fitter != nullptr ? fitter : &_defaultFitter),
	_sampler(sampler),
	_experimenter(experimenter),
	_tester(tester != nullptr ? tester : &_defaultTester) {
}

bool DataAnalyserDefaultImpl::loadDataSet(std::string datafilename) {
	DatasetLoader dataset;
	if (!dataset.loadFromFile(datafilename, ',') && !dataset.loadFromFile(datafilename, ' ')) {
		return false;
	}

	_dataFilename = datafilename;
	if (_fitter != nullptr) {
		_fitter->setDataFilename(datafilename);
	}

	return true;
}

bool DataAnalyserDefaultImpl::loadDataSet(const std::vector<double>& data) {
	DatasetLoader dataset;
	if (!dataset.loadFromVector(data)) {
		return false;
	}

	if (_fitter != nullptr && !_fitter->setData(data)) {
		return false;
	}
	_dataFilename.clear();
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
	if (_sampler == nullptr) {
		throw std::runtime_error("TODO: implement DataAnalyserDefaultImpl::sampler");
	}
	return _sampler;
}

ExperimentManager_if* DataAnalyserDefaultImpl::experimenter() {
	if (_experimenter == nullptr) {
		throw std::runtime_error("TODO: implement DataAnalyserDefaultImpl::experimenter");
	}
	return _experimenter;
}

HypothesisTester_if* DataAnalyserDefaultImpl::tester() {
	return _tester;
}
