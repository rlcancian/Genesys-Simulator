/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CollectorDatafileDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 1 de Agosto de 2018, 20:58
 */

#ifndef COLLECTORDATAFILEDEFAULTIMPL1_H
#define COLLECTORDATAFILEDEFAULTIMPL1_H

#include <string>

#include "CollectorDatafile_if.h"

/*!
 * \brief Default file-backed implementation of \c CollectorDatafile_if.
 *
 * Stores collected values in an associated data file and keeps lightweight
 * counters/cache for last value and number of elements.
 */
class CollectorDatafileDefaultImpl1 : public CollectorDatafile_if {
public:
	/*! \brief Creates an empty file-backed collector instance. */
	CollectorDatafileDefaultImpl1();
	virtual ~CollectorDatafileDefaultImpl1() = default;
public: // inherited from Collector_if
	/*! \brief Clears collector state and resets file cursor/content as implemented. */
	virtual void clear() override;
	/*! \brief Appends a value (and optional weight) to the underlying storage. */
	virtual void addValue(double value, double weight=1) override;
	/*! \brief Returns the most recently collected value. */
	virtual double getLastValue() override;
	/*! \brief Returns the number of collected elements. */
	virtual unsigned long numElements() override;
public:
	/*! \brief Returns the value stored at ordinal position \p num. */
	virtual double getValue(unsigned int num) override;
	/*! \brief Returns the next value from current file cursor and advances cursor. */
	virtual double getNextValue() override;
	/*! \brief Repositions internal cursor to the first stored value. */
	virtual void seekFirstValue() override;
	/*! \brief Returns the filename currently used as data backing store. */
	virtual std::string getDataFilename() override;
	/*! \brief Sets the filename used as data backing store. */
	virtual void setDataFilename(std::string filename) override;
public:
	/*! \brief Registers callback invoked whenever a value is added. */
	virtual void setAddValueHandler(CollectorAddValueHandler addValueHandler) override;
	/*! \brief Registers callback invoked whenever the collector is cleared. */
	virtual void setClearHandler(CollectorClearHandler clearHandler) override;
private:
	std::string _filename;
	double _lastValue;
	unsigned int _numElements;
};

#endif /* COLLECTORDATAFILEDEFAULTIMPL1_H */
