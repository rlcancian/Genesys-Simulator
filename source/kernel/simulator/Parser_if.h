/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Parser_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 23 de Agosto de 2018, 15:57
 */

#ifndef PARSER_IF_H
#define PARSER_IF_H

#include <string>
#include "../statistics/Sampler_if.h"

class genesyspp_driver;

/*!
 * \brief Interface for evaluating model expressions and stochastic functions.
 *
 * Parser implementations are used by model components/data definitions to evaluate
 * arithmetic/logical expressions, integrating deterministic symbols and stochastic
 * calls backed by a \c Sampler_if instance.
 */
class Parser_if {
public:
	/*!
	 * \brief parse
	 * \param expression
	 * \return
	 */
	/*! \brief Evaluates an expression and returns its numeric value; may throw on syntax/semantic errors. */
	virtual double parse(const std::string expression) = 0; // may throw exception
	/*!
	 * \brief parse
	 * \param expression
	 * \param success
	 * \param errorMessage
	 * \return
	 */
    /*! \brief Evaluates an expression without throwing and reports status via output parameters. */
    virtual double parse(const std::string expression, bool& success, std::string& errorMessage) = 0; // does not throw exceptions
	/*!
	 * \brief getErrorMessage
	 * \return
	 */
    /*! \brief Returns the last error message produced by the parser. */
    virtual std::string getErrorMessage() = 0; // to get error message in the case of thrown exception
	/*!
	 * \brief setSampler
	 * \param sampler
	 */
	/*! \brief Sets the random sampler used by stochastic language functions. */
	virtual void setSampler(Sampler_if* sampler) = 0;
	/*!
	 * \brief getSampler
	 * \return
	 */
	/*! \brief Returns the sampler currently associated with the parser. */
	virtual Sampler_if* getSampler() const = 0;
	// ...? // TODO: Implement a method to get the TOKENS parsed. Example: If parsing "nq(queue1) < var1" return a list containing something like "fNQ tLPAR eQUEUE tRPAR tLESS eVARIABLE"
	/*!
	 * \brief getParser
	 * \return
	 */
	/*! \brief Exposes the internal parser driver for advanced integration scenarios. */
	virtual genesyspp_driver getParser() const = 0;
};

#endif /* PARSER_IF_H */

