/*
 * File:   OptimizerDefaultImpl1.cpp
 * Author: Genesys Team
 *
 * Created on 12 de Abril de 2026
 */

#include "OptimizerDefaultImpl1.h"

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelSimulation.h"

OptimizerDefaultImpl1::OptimizerDefaultImpl1() {
	_availableControls = new List<SimulationControl*>();
	_availableResponses = new List<SimulationResponse*>();
	_selectedControls = new List<SimulationControl*>();
	_selectedResponses = new List<SimulationResponse*>();
	_objectives = new List<ObjectiveDefinition>();
	_constraints = new List<ConstraintDefinition>();
	_bestSolutions = new List<SolutionSummary>();
	_updateReadyState();
}

OptimizerDefaultImpl1::~OptimizerDefaultImpl1() {
	delete _availableControls;
	delete _availableResponses;
	delete _selectedControls;
	delete _selectedResponses;
	delete _objectives;
	delete _constraints;
	delete _bestSolutions;
	if (_currentBestSolution != nullptr) {
		delete _currentBestSolution;
	}
}

void OptimizerDefaultImpl1::setModel(Model* model) {
	_model = model;
	_lastErrorMessage.clear();
	_refreshAvailableElementsFromModel();
	_updateReadyState();
}

bool OptimizerDefaultImpl1::setModelFilename(const std::string& filename) {
	// Limitation: Model loading from file requires an existing Simulator instance 
	// which is not directly accessible here. We support only an already instantiated 
	// model via setModel(...) for now.
	_modelFilename = filename;
	_lastErrorMessage = "Loading from file is currently unsupported. Use setModel() instead.";
	_updateReadyState();
	return false;
}

Model* OptimizerDefaultImpl1::getModel() const {
	return _model;
}

std::string OptimizerDefaultImpl1::getModelFilename() const {
	return _modelFilename;
}

List<SimulationControl*>* OptimizerDefaultImpl1::getAvailableControls() const {
	return _availableControls;
}

List<SimulationResponse*>* OptimizerDefaultImpl1::getAvailableResponses() const {
	return _availableResponses;
}

void OptimizerDefaultImpl1::setSelectedControls(List<SimulationControl*>* controls) {
	_copyControlList(controls, _selectedControls);
	_updateReadyState();
}

void OptimizerDefaultImpl1::setSelectedResponses(List<SimulationResponse*>* responses) {
	_copyResponseList(responses, _selectedResponses);
	_updateReadyState();
}

List<SimulationControl*>* OptimizerDefaultImpl1::getSelectedControls() const {
	return _selectedControls;
}

List<SimulationResponse*>* OptimizerDefaultImpl1::getSelectedResponses() const {
	return _selectedResponses;
}

void OptimizerDefaultImpl1::addObjective(const ObjectiveDefinition& objective) {
	_objectives->insert(objective);
	_updateReadyState();
}

void OptimizerDefaultImpl1::clearObjectives() {
	_objectives->clear();
	_updateReadyState();
}

List<Optimizer_if::ObjectiveDefinition>* OptimizerDefaultImpl1::getObjectives() const {
	return _objectives;
}

void OptimizerDefaultImpl1::addConstraint(const ConstraintDefinition& constraint) {
	_constraints->insert(constraint);
}

void OptimizerDefaultImpl1::clearConstraints() {
	_constraints->clear();
}

List<Optimizer_if::ConstraintDefinition>* OptimizerDefaultImpl1::getConstraints() const {
	return _constraints;
}

void OptimizerDefaultImpl1::setSettings(const OptimizationSettings& settings) {
	_settings = settings;
	_updateReadyState();
}

Optimizer_if::OptimizationSettings OptimizerDefaultImpl1::getSettings() const {
	return _settings;
}

bool OptimizerDefaultImpl1::checkReady(std::string* message) const {
	if (_model == nullptr) {
		if (message != nullptr) *message = "Missing model instance.";
		return false;
	}
	if (!_hasEnabledObjective()) {
		if (message != nullptr) *message = "Missing enabled objective.";
		return false;
	}
	if (_settings.maxIterations == 0) {
		if (message != nullptr) *message = "maxIterations must be greater than zero.";
		return false;
	}
	if (_settings.bestSolutionsToKeep == 0) {
		if (message != nullptr) *message = "bestSolutionsToKeep must be greater than zero.";
		return false;
	}
	if (_settings.replicationsPerSolution == 0) {
		if (message != nullptr) *message = "replicationsPerSolution must be greater than zero.";
		return false;
	}
	if (_selectedControls->size() == 0) {
		if (message != nullptr) *message = "No controls selected for optimization.";
		return false;
	}
	if (message != nullptr) {
		*message = "Ready";
	}
	return true;
}

bool OptimizerDefaultImpl1::start() {
	std::string message;
	if (!checkReady(&message)) {
		_lastErrorMessage = message;
		_executionState = ExecutionState::ERROR;
		return false;
	}

	_clearRuntimeState();
	
	// Ensure the model is ready for simulation
	if (!_model->check()) {
		_lastErrorMessage = "Model check failed.";
		_executionState = ExecutionState::ERROR;
		return false;
	}

	_executionState = ExecutionState::RUNNING;
	return true;
}

bool OptimizerDefaultImpl1::step() {
	if (_executionState != ExecutionState::RUNNING) {
		return false;
	}
	
	++_currentIteration;

	// 1. Generate candidate solution (Random search within bounds could be implemented here, 
	// for now we just take the current values from the model as a naive step)
	// Optionally randomize the controls if needed.

	// 2. Execute simulation
	_model->getSimulation()->start();
	++_totalSimulations;

	// 3. Evaluate Constraints
	bool feasible = true;
	for (const ConstraintDefinition& constraint : *_constraints->list()) {
		if (constraint.enabled) {
			bool success = false;
			std::string errorMsg;
			double val = _model->parseExpression(constraint.expression, success, errorMsg);
			if (!success || val == 0.0) {
				feasible = false;
				break;
			}
		}
	}

	// 4. Evaluate Objectives
	double aggregatedObjective = 0.0;
	if (feasible) {
		for (const ObjectiveDefinition& objective : *_objectives->list()) {
			if (objective.enabled) {
				bool success = false;
				std::string errorMsg;
				double val = _model->parseExpression(objective.expression, success, errorMsg);
				if (success) {
					if (objective.sense == ObjectiveSense::MINIMIZE) {
						aggregatedObjective -= val * objective.weight;
					} else {
						aggregatedObjective += val * objective.weight;
					}
				}
			}
		}
	}

	// 5. Update Best Solutions
	if (feasible) {
		SolutionSummary newSolution;
		newSolution.iteration = _currentIteration;
		newSolution.simulationNumber = _totalSimulations;
		newSolution.feasible = feasible;
		newSolution.aggregatedObjectiveValue = aggregatedObjective;
		newSolution.description = "Iteration " + std::to_string(_currentIteration);

		_bestSolutions->insert(newSolution);
		
		// Sort best solutions (descending order based on aggregatedObjectiveValue)
		_bestSolutions->sort([](const SolutionSummary& a, const SolutionSummary& b) {
			return a.aggregatedObjectiveValue > b.aggregatedObjectiveValue;
		});

		// Trim to keep only the best solutions
		while (_bestSolutions->size() > _settings.bestSolutionsToKeep) {
			_bestSolutions->removeAt(_bestSolutions->size() - 1);
		}

		if (_bestSolutions->size() > 0) {
			// Using getAt is better, but since it's an internal pointer, we store a copy or fetch by index.
			// _currentBestSolution will be updated dynamically from getBestSolutions().
		}
	}

	if (_currentIteration >= _settings.maxIterations) {
		_executionState = ExecutionState::FINISHED;
	}
	return true;
}

bool OptimizerDefaultImpl1::pause() {
	if (_executionState != ExecutionState::RUNNING) {
		return false;
	}
	_executionState = ExecutionState::PAUSED;
	return true;
}

bool OptimizerDefaultImpl1::resume() {
	if (_executionState != ExecutionState::PAUSED) {
		return false;
	}
	_executionState = ExecutionState::RUNNING;
	return true;
}

bool OptimizerDefaultImpl1::stop() {
	if (_executionState != ExecutionState::RUNNING && _executionState != ExecutionState::PAUSED) {
		return false;
	}
	_executionState = ExecutionState::STOPPED;
	return true;
}

Optimizer_if::ExecutionState OptimizerDefaultImpl1::getExecutionState() const {
	return _executionState;
}

unsigned int OptimizerDefaultImpl1::getCurrentIteration() const {
	return _currentIteration;
}

unsigned int OptimizerDefaultImpl1::getTotalSimulations() const {
	return _totalSimulations;
}

List<Optimizer_if::SolutionSummary>* OptimizerDefaultImpl1::getBestSolutions() const {
	return _bestSolutions;
}

const Optimizer_if::SolutionSummary* OptimizerDefaultImpl1::getCurrentBestSolution() const {
	if (_bestSolutions->size() > 0) {
		if (_currentBestSolution == nullptr) {
			const_cast<OptimizerDefaultImpl1*>(this)->_currentBestSolution = new SolutionSummary();
		}
		*(const_cast<SolutionSummary*>(_currentBestSolution)) = _bestSolutions->front();
		return _currentBestSolution;
	}
	return nullptr;
}

void OptimizerDefaultImpl1::resetResults() {
	_bestSolutions->clear();
	_currentBestSolution = nullptr;
	_currentIteration = 0;
	_totalSimulations = 0;
	_lastErrorMessage.clear();
	if (_currentBestSolution != nullptr) {
		delete _currentBestSolution;
		_currentBestSolution = nullptr;
	}
	_updateReadyState();
}

void OptimizerDefaultImpl1::_clearRuntimeState() {
	// Placeholder: best-solutions ranking storage is kept empty until algorithm phase.
	_bestSolutions->clear();
	_currentBestSolution = nullptr;
	_currentIteration = 0;
	_totalSimulations = 0;
	_lastErrorMessage.clear();
}

void OptimizerDefaultImpl1::_refreshAvailableElementsFromModel() {
	_availableControls->clear();
	_availableResponses->clear();
	if (_model == nullptr) {
		return;
	}
	_copyControlList(_model->getControls(), _availableControls);
	_copyResponseList(_model->getResponses(), _availableResponses);
}

void OptimizerDefaultImpl1::_copyControlList(List<SimulationControl*>* source, List<SimulationControl*>* target) {
	target->clear();
	if (source == nullptr) {
		return;
	}
	for (SimulationControl* control : *source->list()) {
		target->insert(control);
	}
}

void OptimizerDefaultImpl1::_copyResponseList(List<SimulationResponse*>* source, List<SimulationResponse*>* target) {
	target->clear();
	if (source == nullptr) {
		return;
	}
	for (SimulationResponse* response : *source->list()) {
		target->insert(response);
	}
}

bool OptimizerDefaultImpl1::_hasEnabledObjective() const {
	for (const ObjectiveDefinition& objective : *_objectives->list()) {
		if (objective.enabled) {
			return true;
		}
	}
	return false;
}

void OptimizerDefaultImpl1::_updateReadyState() {
	if (_executionState == ExecutionState::RUNNING ||
			_executionState == ExecutionState::PAUSED ||
			_executionState == ExecutionState::STOPPED ||
			_executionState == ExecutionState::FINISHED) {
		return;
	}

	std::string readyMessage;
	if (checkReady(&readyMessage)) {
		_executionState = ExecutionState::READY;
	} else if (_executionState != ExecutionState::ERROR) {
		_executionState = ExecutionState::NOT_READY;
	}
}
