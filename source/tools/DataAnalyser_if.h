/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   DataAnalyser.h
 * Author: rlcancian
 *
 * Created on 17 de maio de 2022, 11:06
 */

#ifndef DATAANALYSERIF_H
#define DATAANALYSERIF_H

#include <string>
#include "Fitter_if.h"
#include "HypothesisTester_if.h"
#include "../kernel/statistics/Sampler_if.h"
#include "../kernel/simulator/ExperimetManager_if.h"

class DataAnalyser_if {
public:
	virtual ~DataAnalyser_if() = default;
	virtual bool loadDataSet(std::string datafilename) = 0;
	virtual bool saveDataSet(std::string datasetname) = 0;
	virtual void newDataSet(std::string datasetname, std::string datafilename) = 0;
	virtual Fitter_if* fitter() = 0;
	virtual Sampler_if* sampler() = 0;
	virtual ExperimentManager_if* experimenter() = 0;
	virtual HypothesisTester_if* tester() = 0;
};


#endif /* DATAANALYSERIF_H */
