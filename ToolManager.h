/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ToolManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Maio de 2019, 09:04
 */

#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include "Sampler_if.h"
#include "Fitter_if.h"
#include "ExperimetManager_if.h"

class Simulator;

class ToolManager {
public:
	ToolManager(Simulator* _simulator);
	virtual ~ToolManager() = default;
public:
	Sampler_if* sampler() const; ///< Returns the Sampler, used to generate samples accordingly to a probability distribution
	Fitter_if* fitter() const;  ///< Returns the fitter, responsible for carrying out tests of adherence of theoretical distributions of probability with sampled data.
	ExperimentManager_if* experimentManager() const; // \todo 202104 to be moved to kernel, controlling SimulationScenarios and therefore ModelSimulations
public: // event handlers
private:
	Fitter_if* _fitter; // = new Traits<Fitter_if>::Implementation();
	Sampler_if* _sampler; // = new Traits<Sampler_if>::Implementation();
	ExperimentManager_if* _processAnalyser;
private:
	Simulator* _simulator;
};

#endif /* TOOLMANAGER_H */

