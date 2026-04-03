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

class ExperimentManagerDefaultImpl1 : public ExperimentManager_if {
public:
	ExperimentManagerDefaultImpl1();
	virtual ~ExperimentManagerDefaultImpl1() = default;
public:
	virtual List<SimulationScenario*>* getScenarios() const override;
	//virtual List<PropertyBase*>* getControls() const;
	//virtual List<PropertyBase*>* getResponses() const;
	//virtual List<PropertyBase*>* extractControlsFromModel(std::string modelFilename) const;
	//virtual List<PropertyBase*>* extractResponsesFromModel(std::string modelFilename) const;
	virtual void startSimulationOfScenario(SimulationScenario* scenario) override;
	virtual void startExperiment() override;
	virtual void stopExperiment() override;
	virtual void addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) override;
private:
	//List<PropertyBase*>* _controls = new List<PropertyBase*>();
};

#endif /* PROCESSANALYSERDEFAULTIMPL1_H */

