/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   FitterDummyImpl.h
 * Author: cancian
 *
 * Created on 23 de Agosto de 2018, 15:36
 */

#ifndef FITTERDUMMYIMPL_H
#define FITTERDUMMYIMPL_H

#include "Fitter_if.h"

/**
 * @brief Placeholder implementation of Fitter_if.
 *
 * Purpose:
 * - Keep binary/source compatibility while a functional fitter implementation
 *   is not fully available.
 *
 * Implementation status:
 * - Dummy/stub component; not a functional fitting engine.
 *
 * Planned evolution:
 * - To be replaced as default by FitterDefaultImpl when fitting logic is
 *   consolidated.
 */
class FitterDummyImpl : public Fitter_if {
public:
	FitterDummyImpl();
	FitterDummyImpl(const FitterDummyImpl& orig);
	~FitterDummyImpl();
public:
	bool isNormalDistributed(double confidencelevel);
	void fitUniform(double *sqrerror, double *min, double *max);
	void fitTriangular(double *sqrerror, double *min, double *mo, double *max);
	void fitNormal(double *sqrerror, double *avg, double *stddev);
	void fitExpo(double *sqrerror, double *avg1);
	void fitErlang(double *sqrerror, double *avg, double *m);
	void fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit);
	void fitWeibull(double *sqrerror, double *alpha, double *scale);
	void fitAll(double *sqrerror, std::string *name);
public:
	void setDataFilename(std::string dataFilename);
	std::string getDataFilename();
private:
	std::string _dataFilename = "";
};

#endif /* FITTERDUMMYIMPL_H */
