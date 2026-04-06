/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   StatisticsDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 1 de Agosto de 2018, 21:03
 */

#ifndef STATISTICSDEFAULTIMPL1_H
#define STATISTICSDEFAULTIMPL1_H

#include "Statistics_if.h"
#include "Collector_if.h"
//namespace GenesysKernel {

/*!
 * \brief Default online statistics implementation over a \c Collector_if source.
 *
 * Updates core metrics incrementally through collector callbacks, avoiding
 * complete rescans for every query.
 */
class StatisticsDefaultImpl1 : public Statistics_if {
public:
	/*! \brief Creates statistics object using configured default collector implementation. */
	StatisticsDefaultImpl1(); //!< When constructor is invoked without a Collector, it is taken from Traits<Statistics_if>::CollectorImplementation configuration
	/*! \brief Creates statistics object bound to an external collector. */
	StatisticsDefaultImpl1(Collector_if* collector);
	virtual ~StatisticsDefaultImpl1() = default;
public:
	/*! \brief Returns current collector data source. */
	virtual Collector_if* getCollector() const override;
	/*! \brief Sets collector source and reconnects internal callbacks. */
	virtual void setCollector(Collector_if* collector) override;
public:
	/*! \brief Returns number of observed samples. */
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
	/*! \brief Returns confidence interval half-width. */
	virtual double halfWidthConfidenceInterval() override;
	/*! \brief Returns configured confidence level. */
	virtual double confidenceLevel() override;
	/*! \brief Estimates sample size required for desired half-width. */
	virtual unsigned int newSampleSize(double halfWidth) override;
	/*! \brief Sets confidence level used in inferential formulas. */
	virtual void setConfidenceLevel(double confidencelevel) override;
private:
	/*! \brief Callback executed after a new value is appended to collector. */
	void collectorAddHandler(double newValue, double newWeight);
	/*! \brief Callback executed when collector is cleared. */
	void collectorClearHandler();
	/*! \brief Initializes/zeros all internal accumulators and cached metrics. */
	void initStatistics();
private:
	Collector_if* _collector;
	unsigned long _elems;
	double _sumData;
	double _sumDataSquare;
	double _sumWeight;
	double _sumWeightSquare;
	double _min;
	double _max;
	double _average;
	double _variance;
	double _unweightedvariance;
	double _unbiasedVariance;
	double _stddeviation;
	double _variationCoef;
	double _confidenceLevel = 0.95;
	double _criticalTn_1 = 1.96;
	double _halfWidth;
};
//namespace\\}
#endif /* STATISTICSDEFAULTIMPL1_H */
