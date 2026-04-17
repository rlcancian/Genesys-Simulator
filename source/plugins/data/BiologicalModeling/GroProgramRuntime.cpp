/*
 * File:   GroProgramRuntime.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"

GroProgramRuntime::ExecutionResult GroProgramRuntime::execute(const GroProgramIr& ir, GroProgramRuntimeState& state) const {
	ExecutionResult result;

	if (state.simulationStep <= 0.0) {
		result.succeeded = false;
		result.errorMessage = "GroProgramRuntime simulation step must be greater than zero. ";
		return result;
	}

	for (const GroProgramIr::Command& command : ir.commands) {
		if (!command.isFunctionCall()) {
			result.skippedRawStatements.push_back(command.sourceText);
			continue;
		}

		if (command.functionName == "tick") {
			if (!command.arguments.empty()) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime tick command does not accept arguments. ";
				return result;
			}
			state.colonyTime += state.simulationStep;
			++state.tickCount;
			++result.executedCommands;
			continue;
		}

		result.unsupportedCommands.push_back(command.sourceText);
	}

	return result;
}
