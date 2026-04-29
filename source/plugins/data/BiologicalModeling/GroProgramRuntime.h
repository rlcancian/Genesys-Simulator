/*
 * File:   GroProgramRuntime.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMRUNTIME_H
#define GROPROGRAMRUNTIME_H

#include "plugins/data/BiologicalModeling/GroProgramIr.h"

#include <map>
#include <string>
#include <vector>

/*!
 * \brief Mutable state used by the plugin-side Gro runtime helper.
 */
struct GroProgramRuntimeState {
	double colonyTime = 0.0;
	double simulationStep = 1.0;
	unsigned int populationSize = 1;
	unsigned int tickCount = 0;
	std::map<std::string, double> contextVariables;
	std::map<std::string, double> variables;
};

/*!
 * \brief Executes the supported Gro IR commands without simulator binding.
 *
 * This helper is intentionally independent from BacteriaColony and the GenESyS
 * event scheduler. It now supports persistent scalar variables, arithmetic
 * expressions, and basic `if/else` control flow while still keeping the runtime
 * local to the biological plugins.
 */
class GroProgramRuntime {
public:
	enum class PopulationMutationType {
		Grow,
		Divide,
		Die,
		SetPopulation
	};

	enum class SignalMutationType {
		Emit,
		Consume,
		Set
	};

	struct PopulationMutation {
		PopulationMutationType type = PopulationMutationType::Grow;
		unsigned int value = 0;
		unsigned int previousPopulationSize = 0;
		unsigned int resultingPopulationSize = 0;
	};

	struct SignalMutation {
		SignalMutationType type = SignalMutationType::Emit;
		double value = 0.0;
	};

	struct ExecutionResult {
		bool succeeded = true;
		std::string errorMessage = "";
		unsigned int executedCommands = 0;
		std::vector<PopulationMutation> populationMutations;
		std::vector<SignalMutation> signalMutations;
		std::map<std::string, double> assignedVariables;
		std::vector<std::string> unsupportedCommands;
		std::vector<std::string> skippedRawStatements;
	};

public:
	/*! \brief Executes supported commands and reports unsupported commands. */
	ExecutionResult execute(const GroProgramIr& ir, GroProgramRuntimeState& state) const;
};

#endif /* GROPROGRAMRUNTIME_H */
