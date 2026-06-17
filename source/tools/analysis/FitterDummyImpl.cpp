/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FitterDummyImpl.cpp
 * Author: cancian
 * 
 * Created on 23 de Agosto de 2018, 15:36
 */

#include "FitterDummyImpl.h"

#include <limits>

FitterDummyImpl::FitterDummyImpl() {
}

FitterDummyImpl::FitterDummyImpl(const FitterDummyImpl& orig) {
}

FitterDummyImpl::~FitterDummyImpl() {
}

bool FitterDummyImpl::isNormalDistributed(double confidencelevel) {
	(void) confidencelevel;
	return false;
}

void FitterDummyImpl::fitUniform(double *sqrerror, double *min, double *max) {
	if (sqrerror != nullptr) { *sqrerror = std::numeric_limits<double>::infinity(); }
	if (min != nullptr) { *min = std::numeric_limits<double>::quiet_NaN(); }
	if (max != nullptr) { *max = std::numeric_limits<double>::quiet_NaN(); }
}

void FitterDummyImpl::fitTriangular(double *sqrerror, double *min, double *mo, double *max) {

}

void FitterDummyImpl::fitNormal(double *sqrerror, double *avg, double *stddev) {

}

void FitterDummyImpl::fitExpo(double *sqrerror, double *avg1) {
}

void FitterDummyImpl::fitErlang(double *sqrerror, double *avg, double *m) {

}

void FitterDummyImpl::fitBeta(double *sqrerror, double *alpha, double *beta, double *infLimit, double *supLimit) {

}

void FitterDummyImpl::fitWeibull(double *sqrerror, double *alpha, double *scale) {

}

void FitterDummyImpl::fitAll(double *sqrerror, std::string *name) {
	if (sqrerror != nullptr) { *sqrerror = std::numeric_limits<double>::infinity(); }
	if (name != nullptr) { *name = "dummy"; }
}

FitSummary FitterDummyImpl::fitAllSummary() {
	FitSummary summary;
	summary.bestFit.distributionName = "dummy";
	summary.bestFit.squaredError = std::numeric_limits<double>::infinity();
	summary.bestFit.message = "dummy fitter";
	return summary;
}

void FitterDummyImpl::setDataFilename(std::string dataFilename) {
	_dataFilename = dataFilename;
}

bool FitterDummyImpl::setData(const std::vector<double>& data) {
	(void) data;
	_dataFilename.clear();
	return false;
}

std::string FitterDummyImpl::getDataFilename() {
	return _dataFilename;
}
