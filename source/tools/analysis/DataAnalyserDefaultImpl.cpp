#include "DataAnalyserDefaultImpl.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {

std::size_t defaultHistogramClassCount(std::size_t count) {
	if (count == 0) {
		return 0;
	}
	return static_cast<std::size_t>(std::ceil(1.0 + 3.322 * std::log10(static_cast<double>(count))));
}

double percentile(const std::vector<double>& sorted, double probability) {
	if (sorted.empty()) {
		return 0.0;
	}
	if (sorted.size() == 1) {
		return sorted.front();
	}

	const double p = std::max(0.0, std::min(1.0, probability));
	const double position = p * static_cast<double>(sorted.size() - 1U);
	const std::size_t lower = static_cast<std::size_t>(std::floor(position));
	const std::size_t upper = static_cast<std::size_t>(std::ceil(position));
	const double fraction = position - static_cast<double>(lower);
	return sorted[lower] * (1.0 - fraction) + sorted[upper] * fraction;
}

} // namespace

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
	_dataset = dataset;

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
	_dataset = dataset;
	_dataFilename.clear();
	return true;
}

DataSetSummary DataAnalyserDefaultImpl::summary() const {
	DataSetSummary result;
	result.usable = _dataset.isUsable();
	result.count = _dataset.count();
	result.min = _dataset.min();
	result.max = _dataset.max();
	result.mean = _dataset.mean();
	result.variance = _dataset.variance();
	result.stddev = _dataset.stddev();
	result.hasNegativeData = _dataset.hasNegativeData();
	return result;
}

DataSetHistogram DataAnalyserDefaultImpl::histogram(std::size_t classCount) const {
	DataSetHistogram result;
	if (!_dataset.isUsable() || _dataset.count() == 0) {
		return result;
	}

	result.usable = true;
	result.count = _dataset.count();
	result.min = _dataset.min();
	result.max = _dataset.max();

	if (result.min == result.max) {
		result.bins.push_back(HistogramBin{result.min, result.max, result.count, 1.0});
		return result;
	}

	if (classCount == 0) {
		classCount = defaultHistogramClassCount(result.count);
	}
	if (classCount == 0) {
		result.usable = false;
		return result;
	}

	result.classWidth = (result.max - result.min) / static_cast<double>(classCount);
	result.bins.reserve(classCount);
	for (std::size_t i = 0; i < classCount; ++i) {
		const double lower = result.min + static_cast<double>(i) * result.classWidth;
		const double upper = (i + 1U == classCount) ? result.max : lower + result.classWidth;
		result.bins.push_back(HistogramBin{lower, upper, 0, 0.0});
	}

	for (double value : _dataset.data()) {
		std::size_t index = static_cast<std::size_t>((value - result.min) / result.classWidth);
		if (index >= classCount) {
			index = classCount - 1U;
		}
		++result.bins[index].frequency;
	}

	for (HistogramBin& bin : result.bins) {
		bin.relativeFrequency = static_cast<double>(bin.frequency) / static_cast<double>(result.count);
	}
	return result;
}

DataSetBoxPlot DataAnalyserDefaultImpl::boxplot() const {
	DataSetBoxPlot result;
	const std::vector<double>& sorted = _dataset.sortedData();
	if (!_dataset.isUsable() || sorted.empty()) {
		return result;
	}

	result.usable = true;
	result.count = sorted.size();
	result.min = sorted.front();
	result.max = sorted.back();
	result.firstQuartile = percentile(sorted, 0.25);
	result.median = percentile(sorted, 0.50);
	result.thirdQuartile = percentile(sorted, 0.75);
	result.interquartileRange = result.thirdQuartile - result.firstQuartile;
	result.lowerFence = result.firstQuartile - 1.5 * result.interquartileRange;
	result.upperFence = result.thirdQuartile + 1.5 * result.interquartileRange;
	result.lowerWhisker = result.min;
	result.upperWhisker = result.max;

	for (double value : sorted) {
		if (value < result.lowerFence || value > result.upperFence) {
			result.outliers.push_back(value);
		}
	}

	const auto lowerIt = std::find_if(sorted.begin(), sorted.end(), [&result](double value) {
		return value >= result.lowerFence;
	});
	if (lowerIt != sorted.end()) {
		result.lowerWhisker = *lowerIt;
	}

	const auto upperIt = std::find_if(sorted.rbegin(), sorted.rend(), [&result](double value) {
		return value <= result.upperFence;
	});
	if (upperIt != sorted.rend()) {
		result.upperWhisker = *upperIt;
	}

	return result;
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
