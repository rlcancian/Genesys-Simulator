/*
 * File:   GroProgramRuntime.h
 * Author: rlcancian
 *
 * Created on 17 de Abril de 2022
 */

#ifndef GROPROGRAMRUNTIME_H
#define GROPROGRAMRUNTIME_H

#include "plugins/data/BiochemicalSimulation/GroProgramIr.h"

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
	// Counts "VAR := signal(kdiff, kdeg);" assignments executed so far in
	// this pass, in source order. The Nth assignment yields handle N within
	// one program. BacteriaColony separately rejects that ordinal being
	// declared by a different program scope in the same replication.
	unsigned int signalDeclarationOrdinal = 0;
	// Highest signal channel handle already established in an EARLIER pass
	// (e.g. a global "signal(...)" declaration executed by the colony-wide
	// prelude before this bacterium-scoped pass began). A named program
	// that only *consumes* a global handle (get_signal/emit_signal/
	// absorb_signal) without itself containing a "signal(...)" declaration
	// has signalDeclarationOrdinal == 0 in its own fresh pass, so handle
	// validation must also accept anything already known from history.
	// The caller (BacteriaColony) is responsible for setting this from its
	// own persistent channel bookkeeping; it defaults to 0 (no prior
	// history) for callers that run self-contained IR with no such colony.
	unsigned int knownSignalChannelCount = 0;
	// Stable source scope for signal() declarations. All passes of one named
	// program share this identity; global prelude declarations use "<global>".
	std::string signalDeclarationScope;
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

	enum class MotionMutationType {
		Run,
		Tumble
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
		// Channel 0 is the colony's single default/legacy field (backward
		// compatible with pre-multi-channel behavior). Channel N >= 1 is the
		// Nth distinct "signal(...)" declaration encountered in program
		// order, matching GroProgramCompiler/GroProgramRuntime's ordinal
		// channel-handle assignment (see executeCommands' Assignment case).
		unsigned int channel = 0;
	};

	struct MotionMutation {
		MotionMutationType type = MotionMutationType::Run;
		double value = 0.0;
		double previousDirection = 0.0;
		double resultingDirection = 0.0;
		double previousSpeed = 0.0;
		double resultingSpeed = 0.0;
	};

		struct ColonyMutation {
			enum class Type {
				Reset,
				SpawnSeed,
			SetChemostatMode,
			AddBarrier,
			MapToCells,
			SetSignalAt,
			SetSignalRect,
			SetSignalGridWidth,
			SetSignalGridHeight,
			GetSignalMatrix,
			DumpSignalField,
			EnsureSignalChannel
		};

			Type type = Type::Reset;
			std::vector<std::string> arguments;
			std::vector<double> numericArguments;
			std::string expressionText;
			std::string signalDeclarationScope;
			std::size_t previewRows = 0;
			std::size_t previewColumns = 0;
		};

	struct ExecutionResult {
		bool succeeded = true;
		std::string errorMessage = "";
		unsigned int executedCommands = 0;
		std::vector<PopulationMutation> populationMutations;
		std::vector<SignalMutation> signalMutations;
		std::vector<MotionMutation> motionMutations;
		std::vector<ColonyMutation> colonyMutations;
		std::vector<double> mappedCellValues;
		std::string mappedCellExpression;
		std::vector<double> signalMatrixValues;
		unsigned int signalMatrixWidth = 0;
		unsigned int signalMatrixHeight = 0;
		std::string signalFieldDump;
		std::vector<std::string> messages;
		std::map<std::string, double> assignedVariables;
		std::vector<std::string> unsupportedCommands;
		std::vector<std::string> skippedRawStatements;
	};

public:
	/*! \brief Executes supported commands and reports unsupported commands. */
	ExecutionResult execute(const GroProgramIr& ir, GroProgramRuntimeState& state) const;
	/*! Reject structurally conditional signal declarations across all IR scopes. */
	static bool validateSignalDeclarationPlacement(const GroProgramIr& ir, std::string& errorMessage);
	/*! \brief Evaluates one scalar Gro expression against one runtime state. */
	static bool evaluateExpression(const std::string& expressionText,
	                               const GroProgramRuntimeState& state,
	                               double& value,
	                               std::string& errorMessage);
};

#endif /* GROPROGRAMRUNTIME_H */
