/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelChecker_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 23 de Agosto de 2018, 15:48
 */

#ifndef MODELCHECKER_IF_H
#define MODELCHECKER_IF_H

#include <typeinfo>
#include <string>

//#include "Model.h"
//class Model;

/*!
 * The ModelChecker is responsable for verifying the model consistency, fixing inconsistencies wheneaver possible
 */
class ModelChecker_if {
public:
	virtual ~ModelChecker_if() = default;
	/*!
	 * \brief checkAll
	 * \return
	 */
	virtual bool checkAll() = 0; /*!< Invoques all other checks and returns true only if all of them returned true */
	/*!
	 * \brief checkConnected
	 * \return
	 */
	virtual bool checkConnected() = 0; /*!< Checks if components are consistently connected to other to form a valid process-oriented model, describing how entities proceed to the flow*/
	/*!
	 * \brief checkSymbols
	 * \return
	 */
	virtual bool checkSymbols() = 0; /*!< Checks if user-defined strings for symbols required by components, usually expressions or functions, are valid or references existing and valid elements. */
	/*!
	 * \brief checkActivationCode
	 * \return
	 */
	virtual bool checkActivationCode() = 0; /*!< Checks if the installed version has acquired a valid activation code for commercial use */
	/*!
	 * \brief checkLimits
	 * \return
	 */
	virtual bool checkLimits() = 0; /*!< Checks if the model (its components and elements) fits the limits based on the activation code */
	/*!
	 * \brief checkOrphaned
	 * \return
	 */
	virtual bool checkOrphaned() = 0; /*!< Checks if there are ModelDataDefinitions not referred by any ModelComponent, which means they are orphan ModelDataDefinitions */
};

#endif /* MODELCHECKER_IF_H */
