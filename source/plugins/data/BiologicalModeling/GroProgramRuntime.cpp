/*
 * File:   GroProgramRuntime.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/data/BiologicalModeling/GroProgramRuntime.h"

#include <limits>
#include <stdexcept>

namespace {

bool parseUnsignedIntegerArgument(const std::string& argument, unsigned int& value) {
	if (argument.empty()) {
		return false;
	}

	std::size_t parsedCharacters = 0;
	unsigned long parsedValue = 0;
	try {
		parsedValue = std::stoul(argument, &parsedCharacters, 10);
	} catch (const std::invalid_argument&) {
		return false;
	} catch (const std::out_of_range&) {
		return false;
	}

	if (parsedCharacters != argument.size() || parsedValue > std::numeric_limits<unsigned int>::max()) {
		return false;
	}

	value = static_cast<unsigned int>(parsedValue);
	return true;
}

bool parseOptionalPositiveAmount(const GroProgramIr::Command& command, unsigned int& amount, std::string& errorMessage) {
	amount = 1;
	if (command.arguments.empty()) {
		return true;
	}
	if (command.arguments.size() != 1 || !parseUnsignedIntegerArgument(command.arguments.front(), amount) || amount == 0) {
		errorMessage = "GroProgramRuntime command " + command.functionName + " expects zero or one positive integer argument. ";
		return false;
	}
	return true;
}

bool addPopulation(unsigned int currentPopulation, unsigned int amount, unsigned int& populationSize) {
	if (amount > std::numeric_limits<unsigned int>::max() - currentPopulation) {
		return false;
	}
	populationSize = currentPopulation + amount;
	return true;
}

bool removePopulation(unsigned int currentPopulation, unsigned int amount, unsigned int& populationSize) {
	if (amount > currentPopulation) {
		return false;
	}
	populationSize = currentPopulation - amount;
	return true;
}

GroProgramRuntime::PopulationMutation makePopulationMutation(GroProgramRuntime::PopulationMutationType type,
                                                             unsigned int value,
                                                             unsigned int previousPopulationSize,
                                                             unsigned int resultingPopulationSize) {
	GroProgramRuntime::PopulationMutation mutation;
	mutation.type = type;
	mutation.value = value;
	mutation.previousPopulationSize = previousPopulationSize;
	mutation.resultingPopulationSize = resultingPopulationSize;
	return mutation;
}

}

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

		if (command.functionName == "grow") {
			unsigned int amount = 1;
			if (!parseOptionalPositiveAmount(command, amount, result.errorMessage)) {
				result.succeeded = false;
				return result;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!addPopulation(state.populationSize, amount, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime grow command would overflow population size. ";
				return result;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Grow,
			                                                            amount,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "divide") {
			if (!command.arguments.empty()) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime divide command does not accept arguments. ";
				return result;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!addPopulation(state.populationSize, state.populationSize, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime divide command would overflow population size. ";
				return result;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Divide,
			                                                            previousPopulationSize,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "die") {
			unsigned int amount = 1;
			if (!parseOptionalPositiveAmount(command, amount, result.errorMessage)) {
				result.succeeded = false;
				return result;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			if (!removePopulation(state.populationSize, amount, state.populationSize)) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime die command would remove more bacteria than available. ";
				return result;
			}
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::Die,
			                                                            amount,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		if (command.functionName == "set_population") {
			unsigned int populationSize = 0;
			if (command.arguments.size() != 1 || !parseUnsignedIntegerArgument(command.arguments.front(), populationSize) ||
			    populationSize == 0) {
				result.succeeded = false;
				result.errorMessage = "GroProgramRuntime set_population command expects one positive integer argument. ";
				return result;
			}
			const unsigned int previousPopulationSize = state.populationSize;
			state.populationSize = populationSize;
			result.populationMutations.push_back(makePopulationMutation(GroProgramRuntime::PopulationMutationType::SetPopulation,
			                                                            populationSize,
			                                                            previousPopulationSize,
			                                                            state.populationSize));
			++result.executedCommands;
			continue;
		}

		result.unsupportedCommands.push_back(command.sourceText);
	}

	return result;
}
