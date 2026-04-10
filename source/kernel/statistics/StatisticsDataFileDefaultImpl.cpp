/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StatisticsDataFileDummyImpl.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Novembro de 2018, 01:24
 */

#include "StatisticsDataFileDefaultImpl.h"
#include "../TraitsKernel.h"
#include <math.h>

StatisticsDatafileDefaultImpl1::StatisticsDatafileDefaultImpl1() {
	_collector = new TraitsKernel<StatisticsDatafile_if>::CollectorImplementation();
	_collectorSorted = new TraitsKernel<StatisticsDatafile_if>::CollectorImplementation();
	_z.insert(std::make_pair(0.50, 0));
	_z.insert(std::make_pair(0.80, 1.282));
	_z.insert(std::make_pair(0.90, 1.645));
	_z.insert(std::make_pair(0.95, 1.96));
	_z.insert(std::make_pair(0.975, 2.06));
	_z.insert(std::make_pair(0.98, 2.326));
	_z.insert(std::make_pair(0.99, 2.576));
	_z.insert(std::make_pair(0.995, 2.807));
}

StatisticsDatafileDefaultImpl1::~StatisticsDatafileDefaultImpl1() {
	if (_ownsCollector) {
		delete _collector;
	}
	if (_ownsCollectorSorted) {
		delete _collectorSorted;
	}
	if (_ownsSort) {
		delete sort;
	}
	_collector = nullptr;
	_collectorSorted = nullptr;
	sort = nullptr;
}

Collector_if* StatisticsDatafileDefaultImpl1::getCollector() const {
	return this->_collector;
}

void StatisticsDatafileDefaultImpl1::setCollector(Collector_if* collector) {
	if (_collector == collector) {
		return;
	}
	if (_ownsCollector) {
		delete _collector;
	}
	_collector = static_cast<CollectorDatafile_if*> (collector);
	_ownsCollector = false;
}

bool StatisticsDatafileDefaultImpl1::_hasNewValue() {
	if (_numElements < _collector->numElements()) {
		_numElements = _collector->numElements();
		_maxCalculated = false;
		_minCalculated = false;
		_averageCalculated = false;
		_varianceCalculated = false;
		_modeCalculated = false;
		_medianeCalculated = false;
		_stddeviationCalculated = false;
		_variationCoefCalculated = false;
		_halfWidthConfidenceIntervalCalculated = false;
		_newSampleSizeCalculated = false;
		_quartilCalculated = false;
		_decilCalculated = false;
		_centilCalculated = false;
		_histogramNumClassesCalculated = false;
		_histogramClassLowerLimitCalculated = false;
		_histogramClassFrequencyCalculated = false;
		_proportionCalculed = false;
		_fileSorted = false;
		return true;
	} else {
		return false;
	}

}

unsigned int StatisticsDatafileDefaultImpl1::numElements() {
	// Synchronize cached size with the collector before exposing the count.
	_hasNewValue();
	return _numElements;
}

double StatisticsDatafileDefaultImpl1::min() {
	if (_hasNewValue() || !_minCalculated) {
		for (unsigned long i = 0; i < _collector->numElements(); i++) {
			_min = (_collector->getValue(i) < _min) ? _collector->getValue(i) : _min;
		}
		_minCalculated = true;
	}
	return _min;
}

double StatisticsDatafileDefaultImpl1::max() {
	if (_hasNewValue() || !_maxCalculated) {
		for (unsigned long i = 0; i < _collector->numElements(); i++) {
			_max = (_collector->getValue(i) > _max) ? _collector->getValue(i) : _max;
		}
		_maxCalculated = true;
	}
	return _max;
}

double StatisticsDatafileDefaultImpl1::average() {
	if (_hasNewValue() || !_averageCalculated) {
		valueType sumElements = 0;
		for (unsigned long i = 0; i < _collector->numElements(); i++) {
			sumElements += _collector->getValue(i);
		}
		_average = sumElements / (valueType) _collector->numElements();
		_averageCalculated = true;
	}
	return _average;
}

double StatisticsDatafileDefaultImpl1::variance() {
	if (_hasNewValue() || !_varianceCalculated) {
		valueType sumElements = 0;
		for (unsigned long i = 0; i < _collector->numElements(); i++) {
			sumElements += pow((_collector->getValue(i) - average()), 2);
		}
		_variance = sumElements / (valueType) (_collector->numElements() - 1);
		_varianceCalculated = true;
	}
	return _variance;
}

double StatisticsDatafileDefaultImpl1::stddeviation() {
	if (_hasNewValue() || !_stddeviationCalculated) {
		_stddeviation = sqrt(variance());
		_stddeviationCalculated = true;
	}
	return _stddeviation;
}

double StatisticsDatafileDefaultImpl1::variationCoef() {
	if (_hasNewValue() || !_variationCoefCalculated) {
		_variationCoef = stddeviation() / average();
		_variationCoefCalculated = true;
	}
	return _variationCoef;
}

double StatisticsDatafileDefaultImpl1::_getNormalProbability(double confidenceLevel) {
	auto search = _z.find(confidenceLevel);
	if (search != _z.end()) {
		return search->second;
	} else {
		return 0.0;
	}
}

double StatisticsDatafileDefaultImpl1::halfWidthConfidenceInterval() {
	if (_hasNewValue() || !_halfWidthConfidenceIntervalCalculated || _confidenceLevel != _lastConfidenceLevel) {
		double z = _getNormalProbability(_confidenceLevel);
		_halfWidthConfidenceInterval = z * stddeviation() / sqrt(_numElements);
		_lastConfidenceLevel = _confidenceLevel;
		_variationCoefCalculated = true;
	}
	return _halfWidthConfidenceInterval;
}

unsigned int StatisticsDatafileDefaultImpl1::newSampleSize(double halfWidth) {
	if (_hasNewValue() || !_newSampleSizeCalculated || _confidenceLevel != _lastNewSampleSizeConfidenceLevel || halfWidth != _lastNewSampleSizeHalfWidth) {
		double z = _getNormalProbability(_confidenceLevel);
		_newSampleSize = pow((z * stddeviation() / halfWidth), 2);
		_lastNewSampleSizeConfidenceLevel = _confidenceLevel;
		_lastNewSampleSizeHalfWidth = halfWidth;
		_newSampleSizeCalculated = true;
	}
	return _newSampleSize;
}

double StatisticsDatafileDefaultImpl1::confidenceLevel() {
	return _confidenceLevel;
}

void StatisticsDatafileDefaultImpl1::setConfidenceLevel(double confidencelevel) {
	_confidenceLevel = round(confidencelevel * 100.0) / 100.0;
}

double StatisticsDatafileDefaultImpl1::mode() {
	if (_hasNewValue() || !_modeCalculated) {
		if (!_fileSorted) _sortFile();
		valueType tmpModeValue = _collectorSorted->getValue(0), modeValue;
		unsigned int modeCount = 0, tmpCount = 0;
		for (unsigned long position = 1; position < _collectorSorted->numElements(); position++) {
			valueType value = _collectorSorted->getValue(position);
			if (value == tmpModeValue) {
				tmpCount++;
			} else {
				if (tmpCount > modeCount) {
					modeCount = tmpCount;
					modeValue = tmpModeValue;
					tmpModeValue = value;
					tmpCount = 1;
				} else {
					tmpCount = 1;
					tmpModeValue = value;
				}
			}
		}
		_mode = modeValue;
		_modeCalculated = true;
	}
	return _mode;
}

double StatisticsDatafileDefaultImpl1::mediane() {
	if (_hasNewValue() || !_medianeCalculated) {
		if (!_fileSorted) _sortFile();

		if (_collectorSorted->numElements() % 2 == 0) {
			valueType tmpValue = _collectorSorted->getValue((_collectorSorted->numElements() / 2) - 1);
			valueType tmpValue2 = _collectorSorted->getValue((_collectorSorted->numElements() / 2));
			_mediane = (tmpValue + tmpValue2) / 2;
		} else {
			_mediane = _collectorSorted->getValue((_collectorSorted->numElements() + 1) / 2);
		}
		_medianeCalculated = true;
	}
	return _mediane;
}

double StatisticsDatafileDefaultImpl1::quartil(unsigned short num) {
	if (_hasNewValue() || !_quartilCalculated || num != _lastQuartilNum) {
		if (num == 2) _quartil = mediane();
		else {
			if (!_fileSorted) _sortFile();

			valueType quartil, tmpValue, tmpValue2;

			unsigned long position = floor(num * _collectorSorted->numElements() / 4) - 1;
			tmpValue = _collectorSorted->getValue(position);
			//tmpValue2 = _collectorSorted->getValue(position + 1);

			_quartil = tmpValue; //(tmpValue + tmpValue2) / 2;
			_quartilCalculated = true;
			_lastQuartilNum = num;
		}
	}
	return _quartil;
}

double StatisticsDatafileDefaultImpl1::decil(unsigned short num) {
	if (_hasNewValue() || !_decilCalculated || num != _lastDecilNum) {
		if (num == 5) _decil = mediane();
		else {
			if (!_fileSorted) _sortFile();
			unsigned long position = ceil(num * _collectorSorted->numElements() / 10);
			_decil = _collectorSorted->getValue(position);
			_decilCalculated = true;
			_lastDecilNum = num;
		}
	}
	return _decil;
}

double StatisticsDatafileDefaultImpl1::centil(unsigned short num) {
	if (_hasNewValue() || !_centilCalculated || num != _lastCentilNum) {
		if (num == 50) _centil = mediane();
		else {
			if (!_fileSorted) _sortFile();
			unsigned long position = ceil(num * _collectorSorted->numElements() / 100);
			_centil = _collectorSorted->getValue(position);
			_centilCalculated = true;
			_lastCentilNum = num;
		}
	}
	return _centil;
}

void StatisticsDatafileDefaultImpl1::setHistogramNumClasses(unsigned short num) {
	_histogramNumClasses = num;
}

unsigned short StatisticsDatafileDefaultImpl1::histogramNumClasses() {
	if (_hasNewValue() || !_histogramNumClassesCalculated) {
		_histogramNumClasses = ceil(1 + 3.32 * log10(_numElements));
		_histogramNumClassesCalculated = true;
	}
	return _histogramNumClasses;
}

double StatisticsDatafileDefaultImpl1::histogramClassLowerLimit(unsigned short classNum) {
	if (_hasNewValue() || !_histogramClassLowerLimitCalculated || classNum != _LastClassNumHistogramClassLowerLimit) {
		if (classNum == 0)
			_histogramClassLowerLimit = min();
		else
			_histogramClassLowerLimit = min() + classNum * ((max() - min()) / histogramNumClasses());
		_histogramClassLowerLimitCalculated = true;
		_LastClassNumHistogramClassLowerLimit = classNum;
	}
	return _histogramClassLowerLimit;
}

unsigned int StatisticsDatafileDefaultImpl1::histogramClassFrequency(unsigned short classNum) {
	if (_hasNewValue() || !_histogramClassFrequencyCalculated || classNum != _lastClassNumHistogramClassFrequency) {
		if (!_fileSorted) _sortFile();

		valueType classLowerLimit = histogramClassLowerLimit(classNum);
		valueType classUpperLimit = classLowerLimit + ((max() - min()) / histogramNumClasses());
		unsigned int frequency = 0;
		valueType tmpValue;
		for (unsigned long position = 0; position < _collectorSorted->numElements(); position++) {
			tmpValue = _collectorSorted->getValue(position);

			if (tmpValue >= classLowerLimit && tmpValue < classUpperLimit) {
				frequency++;
			} else if (tmpValue >= classUpperLimit) {
				break;
			}
		}
		_histogramClassFrequency = frequency;
		_histogramClassFrequencyCalculated = true;
		_lastClassNumHistogramClassFrequency = classNum;
	}
	return _histogramClassFrequency;
}

void StatisticsDatafileDefaultImpl1::_sortFile() {

	if (!_fileSortedCreated) {
		_collectorSorted->setDataFilename(_collector->getDataFilename() + "_sorted");
		_collectorSorted->clear();
		_fileSortedCreated = true;
	}

	if (_collectorSorted->numElements() < _collector->numElements()) {
		for (unsigned long position = _collectorSorted->numElements(); position < _collector->numElements(); position++) {
			_collectorSorted->addValue(_collector->getValue(position));
		}
	}
	sort->setDataFilename(_collectorSorted->getDataFilename());
	sort->sort();
	_fileSorted = true;
}
