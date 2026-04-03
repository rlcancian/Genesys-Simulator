/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CollectorDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 1 de Agosto de 2018, 20:43
 */

#ifndef COLLECTORDEFAULTIMPL1_H
#define COLLECTORDEFAULTIMPL1_H

#include "Collector_if.h"
//namespace GenesysKernel {

/*!
 * \brief Default in-memory implementation of \c Collector_if.
 *
 * Maintains minimal aggregate state (last value and count) and supports
 * optional callbacks for add/clear events.
 */
class CollectorDefaultImpl1 : public Collector_if {
public:
	/*! \brief Creates an empty collector. */
	CollectorDefaultImpl1();
	virtual ~CollectorDefaultImpl1() = default;
public:
	/*! \brief Clears all collected data and resets counters. */
	virtual void clear() override;
	/*! \brief Adds a new observation to the collector. */
	virtual void addValue(double value, double weight=1) override;
	/*! \brief Returns the latest observed value. */
	virtual double getLastValue() override;
	/*! \brief Returns total number of observed values. */
	virtual unsigned long numElements() override;
public:
	/*! \brief Registers callback notified after each \c addValue call. */
	virtual void setAddValueHandler(CollectorAddValueHandler addValueHandler) override;
	/*! \brief Registers callback notified when \c clear is invoked. */
	virtual void setClearHandler(CollectorClearHandler clearHandler) override;
private:
	double _lastValue;
	unsigned long _numElements = 0;
	CollectorAddValueHandler _addValueHandler = nullptr;
	CollectorClearHandler _clearHandler = nullptr;
};
//namespace\\}
#endif /* COLLECTORDEFAULTIMPL1_H */
