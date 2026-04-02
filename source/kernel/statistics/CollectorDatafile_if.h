/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CollectorDatafile_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Agosto de 2018, 16:45
 */

#ifndef COLLECTORDATAFILE_IF_H
#define COLLECTORDATAFILE_IF_H

#include "Collector_if.h"

/**
 * Interface for collecting values of a stochastic variable that will be stores in a datafile. 
 */
class CollectorDatafile_if : public Collector_if {
public:
	/*!
	 * \brief getValue
	 * \param rank
	 * \return
	 */
	virtual double getValue(unsigned int rank) = 0; /**< Get a value from a specific position */
	/*!
	 * \brief seekFirstValue
	 */
	virtual void seekFirstValue() = 0; /**< Set the pointer to the first value in the file */
	/*!
	 * \brief getNextValue
	 * \return
	 */
	virtual double getNextValue() = 0; /**< Get the next value in the file and advances the pointer */
	/*!
	 * \brief getDataFilename
	 * \return
	 */
	/*! \brief Returns the filename used to persist collected values. */
	virtual std::string getDataFilename() = 0;
	/*!
	 * \brief setDataFilename
	 * \param filename
	 */
	/*! \brief Sets/updates the filename used to store collected data. */
	virtual void setDataFilename(std::string filename) = 0;
};

#endif /* COLLECTORDATAFILE_IF_H */

