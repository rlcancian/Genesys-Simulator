/*
 * File:   Optimizer_if.h
 * Author: Genesys Team
 *
 * Created on 12 de Abril de 2026
 */

#ifndef OPTIMIZER_IF_H
#define OPTIMIZER_IF_H

#include <cstddef>
#include <string>
#include "../kernel/util/List.h"

class Model;
class SimulationControl;
class SimulationResponse;

/**
 * @brief Interface for simulation-based optimization engines (OptQuest-like).
 *
 * This contract defines how an optimization tool can be attached to a model,
 * configured, and executed while managing:
 * - decision variables (selected SimulationControl objects),
 * - observed KPIs (selected SimulationResponse objects),
 * - objectives (minimize/maximize expressions), and
 * - constraints (boolean expressions evaluated as false==0 and true!=0).
 *
 * Typical flow:
 * 1) Associate a model (from file or direct instance).
 * 2) Select controls and responses participating in the search.
 * 3) Register optimization objectives and feasibility constraints.
 * 4) Configure search parameters (iterations, retained best solutions, etc.).
 * 5) Run lifecycle commands (start/step/pause/resume/stop).
 * 6) Query current status, best solutions and progress metrics.
 */
class Optimizer_if {
public:
	virtual ~Optimizer_if() = default;

	/**
	 * @brief Direction used by an optimization objective.
	 */
	enum class ObjectiveSense {
		MINIMIZE,
		MAXIMIZE
	};

	/**
	 * @brief Execution state of the optimization process.
	 */
	enum class ExecutionState {
		NOT_READY,
		READY,
		RUNNING,
		PAUSED,
		STOPPED,
		FINISHED,
		ERROR
	};

	/**
	 * @brief Configurable objective definition.
	 *
	 * @details
	 * The expression is parsed by the model/parser context and can reference
	 * selected responses, controls, constants or user-defined expressions.
	 */
	struct ObjectiveDefinition {
		std::string name;
		ObjectiveSense sense;
		std::string expression;
		double weight = 1.0;
		bool enabled = true;
	};

	/**
	 * @brief Feasibility restriction represented as an expression.
	 *
	 * @details
	 * A constraint is considered satisfied when expression != 0 and violated when
	 * expression == 0.
	 */
	struct ConstraintDefinition {
		std::string name;
		std::string expression;
		bool enabled = true;
	};

	/**
	 * @brief Generic optimization settings independent of a specific algorithm.
	 */
	struct OptimizationSettings {
		unsigned int maxIterations = 100;
		unsigned int maxSimulations = 0;
		unsigned int replicationsPerSolution = 1;
		unsigned int bestSolutionsToKeep = 10;
		unsigned int randomSeed = 0;
		double improvementTolerance = 0.0;
		double timeLimitSeconds = 0.0;
	};

	/**
	 * @brief Captures one candidate or best-known optimization solution.
	 *
	 * @details
	 * A solution stores the controls/responses snapshot produced by one
	 * simulation scenario and whether all constraints were satisfied.
	 */
	struct SolutionSummary {
		unsigned int iteration = 0;
		unsigned int simulationNumber = 0;
		bool feasible = false;
		double aggregatedObjectiveValue = 0.0;
		std::string description;
	};

	/**
	 * @brief Associates an already instantiated model with this optimizer.
	 */
	virtual void setModel(Model* model) = 0;

	/**
	 * @brief Loads/associates a model from a file supported by the simulator.
	 *
	 * @param filename Model file path.
	 * @return true when the model file was successfully loaded and linked.
	 */
	virtual bool setModelFilename(const std::string& filename) = 0;

	/**
	 * @brief Returns the model currently associated with the optimizer.
	 */
	virtual Model* getModel() const = 0;

	/**
	 * @brief Returns the source filename previously configured for model loading.
	 */
	virtual std::string getModelFilename() const = 0;

	/**
	 * @brief Retrieves available model controls discoverable from Model.
	 */
	virtual List<SimulationControl*>* getAvailableControls() const = 0;

	/**
	 * @brief Retrieves available model responses discoverable from Model.
	 */
	virtual List<SimulationResponse*>* getAvailableResponses() const = 0;

	/**
	 * @brief Replaces the set of controls selected for optimization.
	 */
	virtual void setSelectedControls(List<SimulationControl*>* controls) = 0;

	/**
	 * @brief Replaces the set of responses selected for optimization.
	 */
	virtual void setSelectedResponses(List<SimulationResponse*>* responses) = 0;

	/**
	 * @brief Returns currently selected controls used as decision variables.
	 */
	virtual List<SimulationControl*>* getSelectedControls() const = 0;

	/**
	 * @brief Returns currently selected responses monitored by optimization.
	 */
	virtual List<SimulationResponse*>* getSelectedResponses() const = 0;

	/**
	 * @brief Adds one objective to the optimization model.
	 */
	virtual void addObjective(const ObjectiveDefinition& objective) = 0;

	/**
	 * @brief Clears all configured objectives.
	 */
	virtual void clearObjectives() = 0;

	/**
	 * @brief Returns all configured optimization objectives.
	 */
	virtual List<ObjectiveDefinition>* getObjectives() const = 0;

	/**
	 * @brief Adds one feasibility constraint expression.
	 */
	virtual void addConstraint(const ConstraintDefinition& constraint) = 0;

	/**
	 * @brief Clears all configured constraints.
	 */
	virtual void clearConstraints() = 0;

	/**
	 * @brief Returns all configured feasibility constraints.
	 */
	virtual List<ConstraintDefinition>* getConstraints() const = 0;

	/**
	 * @brief Applies algorithm settings such as iteration and retention limits.
	 */
	virtual void setSettings(const OptimizationSettings& settings) = 0;

	/**
	 * @brief Returns currently active optimization settings.
	 */
	virtual OptimizationSettings getSettings() const = 0;

	/**
	 * @brief Validates readiness before running optimization.
	 *
	 * @param message Receives validation details when non-null.
	 * @return true when optimizer has enough data/configuration to start.
	 */
	virtual bool checkReady(std::string* message = nullptr) const = 0;

	/**
	 * @brief Starts a new optimization run.
	 * @return true on successful transition to RUNNING state.
	 */
	virtual bool start() = 0;

	/**
	 * @brief Executes one optimization iteration (or one scenario decision step).
	 * @return true when a step was executed.
	 */
	virtual bool step() = 0;

	/**
	 * @brief Pauses an active optimization execution.
	 */
	virtual bool pause() = 0;

	/**
	 * @brief Resumes optimization execution after a pause.
	 */
	virtual bool resume() = 0;

	/**
	 * @brief Stops optimization execution and finalizes current run.
	 */
	virtual bool stop() = 0;

	/**
	 * @brief Returns current lifecycle state of the optimizer.
	 */
	virtual ExecutionState getExecutionState() const = 0;

	/**
	 * @brief Returns the number of optimization iterations already performed.
	 */
	virtual unsigned int getCurrentIteration() const = 0;

	/**
	 * @brief Returns total number of simulated scenarios evaluated so far.
	 */
	virtual unsigned int getTotalSimulations() const = 0;

	/**
	 * @brief Returns all feasible solutions retained as "best" by the algorithm.
	 */
	virtual List<SolutionSummary>* getBestSolutions() const = 0;

	/**
	 * @brief Returns the best-known feasible solution, or nullptr when absent.
	 */
	virtual const SolutionSummary* getCurrentBestSolution() const = 0;

	/**
	 * @brief Clears results from previous runs while preserving configuration.
	 */
	virtual void resetResults() = 0;
};

#endif /* OPTIMIZER_IF_H */
