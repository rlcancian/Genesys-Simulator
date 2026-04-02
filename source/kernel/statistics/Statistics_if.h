/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Statistics_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 14 de Agosto de 2018, 13:47
 */

#ifndef STATISTICS_IF_H
#define STATISTICS_IF_H

#include <string>
#include "Collector_if.h"

/*!
 * Interface for statisct synthesis of a stochastic variable collected by a Collector_if. The statistics generated may be updated based only on the previous statistics and the single newest added value or they may be updated based on a datafile, depending on the Collector implementation.
 */
class Statistics_if {
public:
	/*! \brief Returns the data collector associated with these statistics. */
	virtual Collector_if* getCollector() const = 0;
	/*! \brief Sets the collector used as source for statistical calculations. */
	virtual void setCollector(Collector_if* collector) = 0;
public:
	/*!
	 * \brief numElements
	 * \return
	 */
	/*! \brief Returns the number of observations considered in calculations. */
	virtual unsigned int numElements() = 0;
	/*!
	 * \brief min
	 * \return
	 */
	/*! \brief Returns the minimum observed value. */
	virtual double min() = 0;
	/*!
	 * \brief max
	 * \return
	 */
	/*! \brief Returns the maximum observed value. */
	virtual double max() = 0;
	/*!
	 * \brief average
	 * \return
	 */
	/*! \brief Returns the sample mean of observations. */
	virtual double average() = 0;
	/*!
	 * \brief variance
	 * \return
	 */
	/*! \brief Returns the sample variance of observations. */
	virtual double variance() = 0;
	/*!
	 * \brief stddeviation
	 * \return
	 */
	/*! \brief Returns the sample standard deviation. */
	virtual double stddeviation() = 0;
	/*!
	 * \brief variationCoef
	 * \return
	 */
	/*! \brief Returns the coefficient of variation (standard deviation / mean). */
	virtual double variationCoef() = 0;
	/*!
	 * \brief halfWidthConfidenceInterval
	 * \return
	 */
	/*! \brief Returns the half-width of the current confidence interval. */
	virtual double halfWidthConfidenceInterval() = 0;
	/*!
	 * \brief confidenceLevel
	 * \return
	 */
	/*! \brief Returns the confidence level used for interval estimates. */
	virtual double confidenceLevel() = 0;
	/*!
	 * \brief newSampleSize
	 * \param halfWidth
	 * \return
	 */
	/*! \brief Estimates the sample size needed to reach a target half-width. */
	virtual unsigned int newSampleSize(double halfWidth) = 0;
	/*!
	 * \brief setConfidenceLevel
	 * \param confidencelevel
	 */
	/*! \brief Sets the confidence level used in inferential calculations. */
	virtual void setConfidenceLevel(double confidencelevel) = 0;
};

#endif /* STATISTICS_IF_H */

