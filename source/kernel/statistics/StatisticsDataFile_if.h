/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   StatisticsDataFile.h
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Novembro de 2018, 01:16
 */

#ifndef STATISTICSDATAFILE_IF_H
#define STATISTICSDATAFILE_IF_H

#include "CollectorDatafile_if.h"
#include "Statistics_if.h"

/*!
 * \brief Extension of \c Statistics_if for metrics that require full data access.
 *
 * This interface targets collector implementations backed by data files, enabling
 * order-based measures (median, quantiles) and histogram-related operations.
 */
class StatisticsDatafile_if : public Statistics_if {
	//public:
	//    virtual CollectorDatafile_if* getCollector() = 0;
	//    virtual void setCollector(Collector_if* collector) = 0;
public:
	virtual ~StatisticsDatafile_if() = default;
	/*!
	 * \brief mode
	 * \return
	 */
	/*! \brief Returns the mode of the sample stored in file. */
	virtual double mode() = 0;
	/*!
	 * \brief mediane
	 * \return
	 */
	/*! \brief Returns the sample median. */
	virtual double mediane() = 0;
	/*!
	 * \brief quartil
	 * \param num
	 * \return
	 */
	/*! \brief Returns the requested quartile (1 to 3). */
	virtual double quartil(unsigned short num) = 0;
	/*!
	 * \brief decil
	 * \param num
	 * \return
	 */
	/*! \brief Returns the requested decile (1 to 9). */
	virtual double decil(unsigned short num) = 0;
	/*!
	 * \brief centil
	 * \param num
	 * \return
	 */
	/*! \brief Returns the requested percentile (1 to 99). */
	virtual double centil(unsigned short num) = 0;
	/*!
	 * \brief setHistogramNumClasses
	 * \param num
	 */
	/*! \brief Sets the number of classes used in the histogram. */
	virtual void setHistogramNumClasses(unsigned short num) = 0;
	/*!
	 * \brief histogramNumClasses
	 * \return
	 */
	/*! \brief Returns the configured number of histogram classes. */
	virtual unsigned short histogramNumClasses() = 0;
	/*!
	 * \brief histogramClassLowerLimit
	 * \param classNum
	 * \return
	 */
	/*! \brief Returns the lower bound of the histogram class at index \p classNum. */
	virtual double histogramClassLowerLimit(unsigned short classNum) = 0;
	/*!
	 * \brief histogramClassFrequency
	 * \param classNum
	 * \return
	 */
	/*! \brief Returns the absolute frequency of the requested histogram class. */
	virtual unsigned int histogramClassFrequency(unsigned short classNum) = 0;

};

#endif /* STATISTICSDATAFILE_IF_H */
