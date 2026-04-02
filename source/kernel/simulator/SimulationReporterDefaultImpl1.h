/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationReporterDefaultImpl1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 8 de Agosto de 2018, 10:59
 */

#ifndef SIMULATIONREPORTERDEFAULTIMPL1_H
#define SIMULATIONREPORTERDEFAULTIMPL1_H

#include "SimulationReporter_if.h"
#include "ModelSimulation.h"
#include "Model.h"

//namespace GenesysKernel {

/*!
 * Class that implements SimulationReporter_if interface and is responsible for building and showing replication and simulation reports
 */
class SimulationReporterDefaultImpl1 : public SimulationReporter_if {
public:
	SimulationReporterDefaultImpl1(ModelSimulation* simulation, Model* model, List<ModelDataDefinition*>* statsCountersSimulation);
	virtual ~SimulationReporterDefaultImpl1() = default;
public:
	virtual void showReplicationStatistics() override;
	virtual void showSimulationStatistics() override;
	virtual void showSimulationResponses() override;
	virtual void showSimulationControls() override;
private:
	ModelSimulation* _simulation;
	Model* _model;
private:
	List<ModelDataDefinition*>* _statsCountersSimulation;
private:
	const unsigned short _w = 9;
	const unsigned short _nameW = 36;
};
//namespace\\}
#endif /* SIMULATIONREPORTERDEFAULTIMPL1_H */

