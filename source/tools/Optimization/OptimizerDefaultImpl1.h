/*
 * File:   OptimizerDefaultImpl1.h
 * Author: Genesys Team
 *
 * Created on 12 de Abril de 2026
 */

#ifndef OPTIMIZERDEFAULTIMPL1_H
#define OPTIMIZERDEFAULTIMPL1_H

#include "Optimizer_if.h"
#include <string>

/**
 * @brief Initial concrete scaffold for an OptQuest-like optimizer.
 *
 * @details
 * This class intentionally provides only the public API declaration for the
 * first default implementation.
 *
 * Current status:
 * - interface contract declared;
 * - behavior/algorithm implementation intentionally deferred.
 *
 * Planned behavior:
 * - discover controls/responses from an associated model,
 * - evaluate objective and constraint expressions through parser context,
 * - execute iterative search preserving top feasible solutions.
 */
class OptimizerDefaultImpl1 : public Optimizer_if {
public:
	OptimizerDefaultImpl1();
	virtual ~OptimizerDefaultImpl1();

public:
	virtual void setModel(Model* model) override;
	virtual bool setModelFilename(const std::string& filename) override;
	virtual Model* getModel() const override;
	virtual std::string getModelFilename() const override;

	virtual List<SimulationControl*>* getAvailableControls() const override;
	virtual List<SimulationResponse*>* getAvailableResponses() const override;
	virtual void setSelectedControls(List<SimulationControl*>* controls) override;
	virtual void setSelectedResponses(List<SimulationResponse*>* responses) override;
	virtual List<SimulationControl*>* getSelectedControls() const override;
	virtual List<SimulationResponse*>* getSelectedResponses() const override;

	virtual void addObjective(const ObjectiveDefinition& objective) override;
	virtual void clearObjectives() override;
	virtual List<ObjectiveDefinition>* getObjectives() const override;

	virtual void addConstraint(const ConstraintDefinition& constraint) override;
	virtual void clearConstraints() override;
	virtual List<ConstraintDefinition>* getConstraints() const override;

	virtual void setSettings(const OptimizationSettings& settings) override;
	virtual OptimizationSettings getSettings() const override;
	virtual bool checkReady(std::string* message = nullptr) const override;

	virtual bool start() override;
	virtual bool step() override;
	virtual bool pause() override;
	virtual bool resume() override;
	virtual bool stop() override;

	virtual ExecutionState getExecutionState() const override;
	virtual unsigned int getCurrentIteration() const override;
	virtual unsigned int getTotalSimulations() const override;
	virtual List<SolutionSummary>* getBestSolutions() const override;
	virtual const SolutionSummary* getCurrentBestSolution() const override;
	virtual void resetResults() override;

private:
	void _clearRuntimeState();
	void _refreshAvailableElementsFromModel();
	void _copyControlList(List<SimulationControl*>* source, List<SimulationControl*>* target);
	void _copyResponseList(List<SimulationResponse*>* source, List<SimulationResponse*>* target);
	bool _hasEnabledObjective() const;
	void _updateReadyState();

private:
	Model* _model = nullptr;
	std::string _modelFilename;
	List<SimulationControl*>* _availableControls = nullptr;
	List<SimulationResponse*>* _availableResponses = nullptr;
	List<SimulationControl*>* _selectedControls = nullptr;
	List<SimulationResponse*>* _selectedResponses = nullptr;
	List<ObjectiveDefinition>* _objectives = nullptr;
	List<ConstraintDefinition>* _constraints = nullptr;
	List<SolutionSummary>* _bestSolutions = nullptr;
	OptimizationSettings _settings;
	ExecutionState _executionState = ExecutionState::NOT_READY;
	unsigned int _currentIteration = 0;
	unsigned int _totalSimulations = 0;
	std::string _lastErrorMessage;
	const SolutionSummary* _currentBestSolution = nullptr;
};

#endif /* OPTIMIZERDEFAULTIMPL1_H */
