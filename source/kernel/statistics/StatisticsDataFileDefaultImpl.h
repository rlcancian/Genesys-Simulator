/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StatisticsDataFileDefaultImpl.h
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Novembro de 2018, 01:24
 */

#ifndef STATISTICSDATAFILEDEFAULTIMPL_H
#define STATISTICSDATAFILEDEFAULTIMPL_H

#include "StatisticsDataFile_if.h"
#include "SorttFile.h"
#include <limits.h>
#include <map>

typedef double valueType;

class StatisticsDatafileDefaultImpl1 : public StatisticsDatafile_if {
public:
	StatisticsDatafileDefaultImpl1();
	virtual ~StatisticsDatafileDefaultImpl1();
public:
	virtual Collector_if* getCollector()  const override;
	virtual void setCollector(Collector_if* collector) override;
public:
	virtual unsigned int numElements() override;
	virtual double min() override;
	virtual double max() override;
	virtual double average() override;
	virtual double variance() override;
	virtual double stddeviation() override;
	virtual double variationCoef() override;
	virtual double halfWidthConfidenceInterval() override;
	virtual double confidenceLevel() override;
	virtual unsigned int newSampleSize(double halfWidth) override;
	virtual void setConfidenceLevel(double confidencelevel) override;
public:
	virtual double mode() override;
	virtual double mediane() override;
	virtual double quartil(unsigned short num) override;
	virtual double decil(unsigned short num) override;
	virtual double centil(unsigned short num) override;
	virtual void setHistogramNumClasses(unsigned short num) override;
	virtual unsigned short histogramNumClasses() override;
	virtual double histogramClassLowerLimit(unsigned short classNum) override;
	virtual unsigned int histogramClassFrequency(unsigned short classNum) override;
private:
	void _sortFile();
	bool _hasNewValue();
	double _getNormalProbability(double confidenceLevel);
private:
	CollectorDatafile_if* _collector;
	bool _ownsCollector = true;
	CollectorDatafile_if* _collectorSorted;
	bool _ownsCollectorSorted = true;
	//CollectorDatafile_if* _collector;
	SortFile * sort = new SortFile();
	bool _ownsSort = true;
	double _confidenceLevel = 0.95;
	unsigned long _numElements = 0;
	std::map<double, double> _z;
	bool _fileSorted = false;
	bool _fileSortedCreated = false;
	//unsigned long _numElements = 0;
	bool _numElementsCalculated = false;
	double _max = INT_MIN;
	bool _maxCalculated = false;
	double _min = INT_MAX;
	bool _minCalculated = false;
	bool _modeCalculated = false;
	double _mode = 0;
	bool _medianeCalculated = false;
	double _mediane = 0;
	double _average = 0;
	bool _averageCalculated = false;
	double _variance = 0;
	bool _varianceCalculated = false;
	double _sumSquareElements = 0;
	double _sumElements = 0;
	double _stddeviation = 0;
	bool _stddeviationCalculated = false;
	double _variationCoef = 0;
	bool _variationCoefCalculated = false;
	bool _halfWidthConfidenceIntervalCalculated = false;
	double _lastConfidenceLevel = 0;
	double _halfWidthConfidenceInterval = 0;
	bool _newSampleSizeCalculated = false;
	double _lastNewSampleSizeConfidenceLevel = 0;
	double _lastNewSampleSizeHalfWidth = 0;
	double _newSampleSize = 0;
	bool _quartilCalculated = false;
	unsigned short _lastQuartilNum = 0;
	double _quartil = 0;
	bool _decilCalculated = false;
	unsigned short _lastDecilNum = 0;
	double _decil = 0;
	bool _centilCalculated = false;
	unsigned short _lastCentilNum = 0;
	double _centil = 0;
	bool _histogramNumClassesCalculated = false;
	unsigned short _histogramNumClasses = 1;
	bool _histogramClassLowerLimitCalculated = false;
	unsigned short _LastClassNumHistogramClassLowerLimit = 0;
	double _histogramClassLowerLimit = 0;
	bool _histogramClassFrequencyCalculated = false;
	unsigned short _lastClassNumHistogramClassFrequency = 0;
	unsigned int _histogramClassFrequency = 0;
	bool _proportionCalculed = false;
	double _proportion = 0;
};

#endif /* STATISTICSDATAFILEDEFAULTIMPL_H */
