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

FitterDummyImpl::FitterDummyImpl() {
}

FitterDummyImpl::FitterDummyImpl(const FitterDummyImpl& orig) {
}

FitterDummyImpl::~FitterDummyImpl() {
}

bool FitterDummyImpl::isNormalDistributed(double confidencelevel) {

}

void FitterDummyImpl::fitUniform(double *sqrerror, double *min, double *max) {

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

}

void FitterDummyImpl::setDataFilename(std::string dataFilename) {

}

std::string FitterDummyImpl::getDataFilename() {

}
