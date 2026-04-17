/*
 * File:   GroProgramRuntime.h
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#ifndef GROPROGRAMRUNTIME_H
#define GROPROGRAMRUNTIME_H

#include "plugins/data/BiologicalModeling/GroProgramIr.h"

#include <string>
#include <vector>

/*!
 * \brief Minimal mutable state used by the first Gro runtime helper.
 */
struct GroProgramRuntimeState {
	double colonyTime = 0.0;
	double simulationStep = 1.0;
	unsigned int tickCount = 0;
};

/*!
 * \brief Executes the first supported Gro IR commands without simulator binding.
 *
 * This helper is intentionally independent from BacteriaColony and the GenESyS
 * event scheduler. It lets later phases bind a colony to Gro commands after the
 * command semantics are tested in isolation.
 */
class GroProgramRuntime {
public:
	struct ExecutionResult {
		bool succeeded = true;
		std::string errorMessage = "";
		unsigned int executedCommands = 0;
		std::vector<std::string> unsupportedCommands;
		std::vector<std::string> skippedRawStatements;
	};

public:
	/*! \brief Executes supported commands and reports unsupported commands. */
	ExecutionResult execute(const GroProgramIr& ir, GroProgramRuntimeState& state) const;
};

#endif /* GROPROGRAMRUNTIME_H */
