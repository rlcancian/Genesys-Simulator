/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ProcessAnalyserDefaultImpl1.h
 * Author: rlcancian
 *
 * Created on 20 de Maio de 2019, 20:45
 */

#ifndef PROCESSANALYSERDEFAULTIMPL1_H
#define PROCESSANALYSERDEFAULTIMPL1_H

#include "ExperimetManager_if.h"
#include "SimulationScenario.h"
#include "SimulationResponse.h"
#include "SimulationControl.h"

class ExperimentManagerDefaultImpl1 : public ExperimentManager_if {
public:
    ExperimentManagerDefaultImpl1();
    virtual ~ExperimentManagerDefaultImpl1();
public:
    virtual List<SimulationScenario*>* getScenarios() const;
    virtual List<SimulationControl*>* getControls() const;
    virtual List<SimulationResponse*>* getResponses() const;
    virtual List<SimulationControl*>* extractControlsFromModel(std::string modelFilename);
    virtual List<SimulationResponse*>* extractResponsesFromModel(std::string modelFilename);
    virtual void startSimulationOfScenario(SimulationScenario* scenario);
    virtual void startExperiment();
    virtual void stopExperiment();
    virtual void addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener);
    virtual Simulator * simulator() const;
private:
    List<SimulationControl*>* _controls;
    List<SimulationResponse*>* _responses;
    List<SimulationScenario*>* _scenarios;
    std::map<std::string, Model*> _loaded_models;
    Simulator * _simulator;
};

#endif /* PROCESSANALYSERDEFAULTIMPL1_H */

