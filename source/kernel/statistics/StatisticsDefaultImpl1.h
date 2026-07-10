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
#include <map>
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
	virtual ~StatisticsDefaultImpl1() override;
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
	/*! \brief Binds collector callbacks used by online statistics updates. */
	void _bindCollectorHandlers(Collector_if* collector);
	/*! \brief Resets state and invalidates online metrics when collector is prefilled. */
	void _resetStateForCurrentCollector();
	/*! \brief Initializes/zeros all internal accumulators and cached metrics. */
	void initStatistics();
private:
	Collector_if* _collector;
	bool _ownsCollector = false;
	bool _onlineStateValid = true;
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
	const std::map<double, double> _supportedConfidenceLevels{
		{0.50, 0.0},
		{0.80, 1.282},
		{0.90, 1.645},
		{0.95, 1.96},
		{0.975, 2.06},
		{0.98, 2.326},
		{0.99, 2.576},
		{0.995, 2.807}
	};
};
//namespace\\}
#endif /* STATISTICSDEFAULTIMPL1_H */
