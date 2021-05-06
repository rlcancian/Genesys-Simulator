/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HypothesisTester_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 23 de Agosto de 2018, 19:04
 */

#ifndef HYPOTHESISTESTER_IF_H
#define HYPOTHESISTESTER_IF_H

#include <string>

/**
 * Interface for parametric hypothesis tests based on a datafile. 
 */
class HypothesisTester_if {
public:

	enum class H1Comparition: int {
		DIFFERENT = 1,
		LESS_THAN = 2,
		GREATER_THAN = 3
	};
public:
	/* \todo: all "test" methods should return double p-value!! */
	virtual double testAverage(double confidencelevel, H1Comparition comp, std::string secondPopulationDataFilename = "") = 0;
	virtual double testProportion(double confidencelevel, H1Comparition comp, std::string secondPopulationDataFilename = "") = 0;
	virtual double testVariance(double confidencelevel, H1Comparition comp, std::string secondPopulationDataFilename = "") = 0;
	virtual void setDataFilename(std::string dataFilename) = 0;
	virtual std::string getDataFilename() = 0;
};

#endif /* HYPOTHESISTESTER_IF_H */

