/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ExperimentManager_if (old ProcessAnalyser_if.h)
 * Author: rafael.luiz.cancian
 *
 * Created on 10 de Outubro de 2018, 14:26
 */

#ifndef EXPERIMENTMANAGER_IF_H
#define EXPERIMENTMANAGER_IF_H

#include "../util/List.h"
#include "SimulationScenario.h"
#include "PropertyGenesys.h"
#include "TraceManager.h"

/*!
 * \brief Interface that coordinates execution of scenario-based simulation experiments.
 *
 * An experiment manager owns a set of scenarios, starts/stops scenario runs and
 * exposes progress tracing hooks so higher-level tools can monitor experiment status.
 */
class ExperimentManager_if {
public:
	virtual ~ExperimentManager_if() = default;
	/*! \brief Returns the list of scenarios that compose the experiment. */
	virtual List<SimulationScenario*>* getScenarios() const = 0;
	//virtual List<PropertyBase*>* getControls() const = 0;
	//virtual List<PropertyBase*>* getResponses() const = 0;
	//virtual List<PropertyBase*>* extractControlsFromModel(std::string modelFilename) const = 0;
	//virtual List<PropertyBase*>* extractResponsesFromModel(std::string modelFilename) const = 0;
	/*! \brief Starts simulation for a specific scenario. */
	virtual void startSimulationOfScenario(SimulationScenario* scenario) = 0;
	/*! \brief Starts full experiment execution (all enabled scenarios). */
	virtual void startExperiment() = 0;
	/*! \brief Requests interruption of the currently running experiment. */
	virtual void stopExperiment() = 0;
	/*! \brief Registers a callback to receive simulation progress traces for the experiment. */
	virtual void addTraceSimulationHandler(traceSimulationProcessListener traceSimulationProcessListener) = 0;
};

#endif /* EXPERIMENTMANAGER_IF_H */
