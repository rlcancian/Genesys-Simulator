/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationExperiment.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 4 de maio de 2021, 15:10
 */

#ifndef SIMULATIONEXPERIMENT_H
#define SIMULATIONEXPERIMENT_H

/**
 * @brief Represents a higher-level simulation experiment definition.
 *
 * Historically, GenESyS started with experiment control concentrated in
 * ModelSimulation, mainly through replication count and replication length.
 *
 * SimulationExperiment was introduced later as part of an unfinished effort to
 * move richer experiment-design concepts into the simulation kernel, including
 * experiment sets, scenarios and Process Analyzer-like workflows.
 *
 * This class is still under development and should currently be treated as a
 * placeholder for future experiment-layer behavior.
 */
class SimulationExperiment {
public:
	SimulationExperiment();
	virtual ~SimulationExperiment() = default;
private:

};

#endif /* SIMULATIONEXPERIMENT_H */

