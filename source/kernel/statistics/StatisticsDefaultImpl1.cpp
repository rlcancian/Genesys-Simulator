/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StatisticsDefaultImpl1.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 1 de Agosto de 2018, 21:03
 */

#include <complex>
#include <cmath>
#include <limits>

#include "StatisticsDefaultImpl1.h"
#include "../TraitsKernel.h"
//#include "Integrator_if.h"
//#include "ProbDistribDefaultImpl1.h"

//using namespace GenesysKernel;

StatisticsDefaultImpl1::StatisticsDefaultImpl1() {
	// Create and own the default collector implementation used by online statistics.
	_collector = new TraitsKernel<Model>::StatisticsCollector_CollectorImplementation();
	_ownsCollector = true;
	this->_bindCollectorHandlers(_collector);
	this->_resetStateForCurrentCollector();
}

StatisticsDefaultImpl1::StatisticsDefaultImpl1(Collector_if* collector) {
	// Bind to an external collector without taking ownership of its lifetime.
	_collector = collector;
	_ownsCollector = false;
	this->_bindCollectorHandlers(_collector);
	this->_resetStateForCurrentCollector();
}

StatisticsDefaultImpl1::~StatisticsDefaultImpl1() {
	// Release collector only when this statistics object owns the allocated instance.
	if (_ownsCollector && _collector != nullptr) {
		delete _collector;
	}
	_collector = nullptr;
}

void StatisticsDefaultImpl1::_bindCollectorHandlers(Collector_if* collector) {
	// Reconnect add/clear callbacks so collector events keep statistics in sync.
	if (collector != nullptr) {
		collector->setAddValueHandler(setCollectorAddValueHandler(&StatisticsDefaultImpl1::collectorAddHandler, this));
		collector->setClearHandler(setCollectorClearHandler(&StatisticsDefaultImpl1::collectorClearHandler, this));
	}
}

void StatisticsDefaultImpl1::_resetStateForCurrentCollector() {
	// Reset online accumulators and invalidate metrics if historical values are not reconstructable.
	this->initStatistics();
	if (_collector != nullptr && _collector->numElements() > 0) {
		_onlineStateValid = false;
		_elems = _collector->numElements();
	}
}

void StatisticsDefaultImpl1::collectorAddHandler(double newValue, double newWeight) {
	// Ignore incremental updates while bound to prefilled collector state that cannot be reconstructed.
	if (!_onlineStateValid || _collector == nullptr) {
		return;
	}
	_elems = _collector->numElements();
	// Initialize first-sample bounds explicitly and update bounds for subsequent samples.
	if (_elems == 1) {
		_min = newValue;
		_max = newValue;
	} else {
		if (newValue < _min) {
			_min = newValue;
		}
		if (newValue > _max) {
			_max = newValue;
		}
	}

	// alternative 1
	// equally
	//_sumData += newValue;
	//double oldAverage = _average;
	//_average = _sumData / _elems;
	//_sumDataSquare += (newValue-oldAverage)*(newValue-_average);
	//_unweightedvariance = _sumDataSquare/_elems;
	//if (_elems>1) {
	//	_unbiasedVariance =_sumDataSquare/(_elems-1);
	//}
	// Considering weight
	_sumWeight += newWeight;
	_sumWeightSquare += newWeight*newWeight;
	double oldAverage = _average;
	_average = oldAverage + (newWeight/_sumWeight)*(newValue-oldAverage);
	_sumData += newWeight*(newValue-oldAverage)*(newValue-_average);
	// Keep variance caches coherent and return NaN for undefined finite-sample configurations.
	const double varianceDenominator = _sumWeight - 1.0;
	// Protect weighted-unbiased denominator calculation against zero accumulated weight.
	const double unbiasedDenominator = (_sumWeight == 0.0) ? 0.0 : (_sumWeight - _sumWeightSquare / _sumWeight);
	_variance = (_elems < 2 || varianceDenominator <= 0.0) ? std::numeric_limits<double>::quiet_NaN() : _sumData / varianceDenominator;
	_unbiasedVariance = (_elems < 2 || unbiasedDenominator <= 0.0) ? std::numeric_limits<double>::quiet_NaN() : _sumData / unbiasedDenominator;

	// alternative 2 (numerical instability)
	//_average = _average + (newValue - _average)/_elems;  // or bellow
	//_average = (_average * (elems - 1) + newValue) / elems;  // this approach propagates the numeric error
	//_variance = (_variance * (elems - 1) + pow(newValue - _average, 2)) / elems;  // this approach propagates the numeric error

	// Propagate undefined dispersion and confidence settings as NaN instead of fragile default zeros.
	_stddeviation = (_elems < 3 || std::isnan(_variance)) ? std::numeric_limits<double>::quiet_NaN() : sqrt(_variance);
	_variationCoef = (_elems == 0 || std::isnan(_stddeviation) || _average == 0.0) ? std::numeric_limits<double>::quiet_NaN() : (_stddeviation / _average);
	_halfWidth = (_elems == 0 || std::isnan(_stddeviation) || !std::isfinite(_criticalTn_1)) ? std::numeric_limits<double>::quiet_NaN() : _criticalTn_1 * (_stddeviation / std::sqrt(_elems));
}

void StatisticsDefaultImpl1::collectorClearHandler() {
	// A collector clear event restores a reconstructable online state from an empty sample.
	this->initStatistics();
	_onlineStateValid = true;
}

void StatisticsDefaultImpl1::initStatistics() {
	// Reset accumulators to the project-established empty-state baseline.
	_elems = 0;
	_min = 0.0;
	_max = 0.0;
	_sumData = 0.0;
	_sumWeight = 0.0;
	_sumWeightSquare = 0.0;
	_sumDataSquare = 0.0;
	_average = 0.0;
	_variance = 0.0;
	_unweightedvariance = 0.0;
	_unbiasedVariance = 0.0;
	_stddeviation = 0.0;
	_variationCoef = 0.0;
	_halfWidth = 0.0;
	_onlineStateValid = true;
}

unsigned int StatisticsDefaultImpl1::numElements() {
	return this->getCollector()->numElements();
}

double StatisticsDefaultImpl1::min() {
	// Keep contract-compatible zero baseline for empty or invalid state.
	if (!_onlineStateValid || _elems == 0) {
		return 0.0;
	}
	return _min;
}

double StatisticsDefaultImpl1::max() {
	// Keep contract-compatible zero baseline for empty or invalid state.
	if (!_onlineStateValid || _elems == 0) {
		return 0.0;
	}
	return _max;
}

double StatisticsDefaultImpl1::average() {
	// Keep contract-compatible zero baseline for empty or invalid state.
	if (!_onlineStateValid || _elems == 0) {
		return 0.0;
	}
	return _average;
}

double StatisticsDefaultImpl1::variance() {
	// Keep contract-compatible zero baseline when reconstruction is unavailable.
	if (!_onlineStateValid) {
		return 0.0;
	}
	return _variance;
}

double StatisticsDefaultImpl1::stddeviation() {
	// Keep contract-compatible zero baseline when reconstruction is unavailable.
	if (!_onlineStateValid) {
		return 0.0;
	}
	return _stddeviation;
}

double StatisticsDefaultImpl1::variationCoef() {
	// Keep contract-compatible zero baseline when reconstruction is unavailable.
	if (!_onlineStateValid) {
		return 0.0;
	}
	return _variationCoef;
}

double StatisticsDefaultImpl1::halfWidthConfidenceInterval() {
	// Keep contract-compatible zero baseline when reconstruction is unavailable.
	if (!_onlineStateValid) {
		return 0.0;
	}
	return _halfWidth;
}

void StatisticsDefaultImpl1::setConfidenceLevel(double confidencelevel) {
	// Accept only supported confidence levels and invalidate inferential constants otherwise.
	if (confidencelevel <= 0.0 || confidencelevel >= 1.0) {
		_confidenceLevel = std::numeric_limits<double>::quiet_NaN();
		_criticalTn_1 = std::numeric_limits<double>::quiet_NaN();
	} else {
		auto found = _supportedConfidenceLevels.find(confidencelevel);
		if (found != _supportedConfidenceLevels.end()) {
			_confidenceLevel = confidencelevel;
			_criticalTn_1 = found->second;
		} else {
			_confidenceLevel = std::numeric_limits<double>::quiet_NaN();
			_criticalTn_1 = std::numeric_limits<double>::quiet_NaN();
		}
	}
	// Recompute cached half-width consistently with the current confidence policy.
	_halfWidth = (!_onlineStateValid || _elems == 0 || std::isnan(_stddeviation) || !std::isfinite(_criticalTn_1)) ? std::numeric_limits<double>::quiet_NaN() : _criticalTn_1 * (_stddeviation / std::sqrt(_elems));

}

double StatisticsDefaultImpl1::confidenceLevel() {
	return _confidenceLevel;
}

unsigned int StatisticsDefaultImpl1::newSampleSize(double halfWidth) {
	// Compute required sample size from current dispersion and reject undefined input scenarios.
	if (!_onlineStateValid || halfWidth <= 0.0 || !std::isfinite(_criticalTn_1) || std::isnan(_stddeviation) || std::isinf(_stddeviation)) {
		return 0;
	}
	const double ratio = _criticalTn_1 * _stddeviation / halfWidth;
	if (!std::isfinite(ratio) || ratio < 0.0) {
		return 0;
	}
	return static_cast<unsigned int>(std::ceil(ratio * ratio));
}

Collector_if* StatisticsDefaultImpl1::getCollector() const {
	return this->_collector;
}

void StatisticsDefaultImpl1::setCollector(Collector_if* collector) {
	// Transfer collector binding safely, preserving ownership semantics and callback connections.
	if (_collector == collector) {
		return;
	}
	Collector_if* oldCollector = _collector;
	const bool oldCollectorOwned = _ownsCollector;
	this->_collector = collector;
	_ownsCollector = false;
	this->_bindCollectorHandlers(_collector);
	this->_resetStateForCurrentCollector();
	// Delete previous collector only when this object owned it and the instance actually changed.
	if (oldCollectorOwned && oldCollector != nullptr) {
		delete oldCollector;
	}
}
