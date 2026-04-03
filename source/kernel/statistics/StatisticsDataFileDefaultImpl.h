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

/*!
 * \brief Default implementation of \c StatisticsDatafile_if for file-backed samples.
 *
 * Computes descriptive/inferential statistics from a \c CollectorDatafile_if and
 * caches results to avoid recomputation until source data changes.
 */
class StatisticsDatafileDefaultImpl1 : public StatisticsDatafile_if {
public:
	/*! \brief Creates statistics object with default collector implementation. */
	StatisticsDatafileDefaultImpl1();
	/*! \brief Releases owned collector/sorter resources when configured as owner. */
	virtual ~StatisticsDatafileDefaultImpl1();
public:
	/*! \brief Returns the collector currently used as data source. */
	virtual Collector_if* getCollector()  const override;
	/*! \brief Sets a collector to be used as data source for calculations. */
	virtual void setCollector(Collector_if* collector) override;
public:
	/*! \brief Returns number of observations available in the collector. */
	virtual unsigned int numElements() override;
	/*! \brief Returns minimum observed value. */
	virtual double min() override;
	/*! \brief Returns maximum observed value. */
	virtual double max() override;
	/*! \brief Returns arithmetic mean. */
	virtual double average() override;
	/*! \brief Returns sample variance. */
	virtual double variance() override;
	/*! \brief Returns sample standard deviation. */
	virtual double stddeviation() override;
	/*! \brief Returns coefficient of variation. */
	virtual double variationCoef() override;
	/*! \brief Returns current confidence interval half-width. */
	virtual double halfWidthConfidenceInterval() override;
	/*! \brief Returns configured confidence level. */
	virtual double confidenceLevel() override;
	/*! \brief Estimates sample size needed for a desired half-width. */
	virtual unsigned int newSampleSize(double halfWidth) override;
	/*! \brief Sets confidence level used in inferential calculations. */
	virtual void setConfidenceLevel(double confidencelevel) override;
public:
	/*! \brief Returns sample mode. */
	virtual double mode() override;
	/*! \brief Returns sample median. */
	virtual double mediane() override;
	/*! \brief Returns requested quartile. */
	virtual double quartil(unsigned short num) override;
	/*! \brief Returns requested decile. */
	virtual double decil(unsigned short num) override;
	/*! \brief Returns requested percentile. */
	virtual double centil(unsigned short num) override;
	/*! \brief Sets number of histogram classes. */
	virtual void setHistogramNumClasses(unsigned short num) override;
	/*! \brief Returns current number of histogram classes. */
	virtual unsigned short histogramNumClasses() override;
	/*! \brief Returns lower limit of histogram class \p classNum. */
	virtual double histogramClassLowerLimit(unsigned short classNum) override;
	/*! \brief Returns absolute frequency of histogram class \p classNum. */
	virtual unsigned int histogramClassFrequency(unsigned short classNum) override;
private:
	/*! \brief Sorts collector file if required by order-based metrics. */
	void _sortFile();
	/*! \brief Checks whether source data changed since last cached computation. */
	bool _hasNewValue();
	/*! \brief Returns normal-distribution critical value for confidence level. */
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
