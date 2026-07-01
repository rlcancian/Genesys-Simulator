/*
 * File:   ContinuousSystemComponent.cpp
 * Author: GenESyS
 *
 * ModelComponent that drives continuous-time simulation by delegating
 * numerical integration to an ODESolver data definition.
 */

#include "plugins/components/Continuous/ContinuousSystemComponent.h"

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/Logic/Variable.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ContinuousSystemComponent::GetPluginInformation;
}
#endif

// ---------------------------------------------------------------------------
// Static factory / framework methods
// ---------------------------------------------------------------------------

ModelDataDefinition* ContinuousSystemComponent::NewInstance(Model* model, std::string name) {
	return new ContinuousSystemComponent(model, name);
}

ModelComponent* ContinuousSystemComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	ContinuousSystemComponent* newComponent = new ContinuousSystemComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

void ContinuousSystemComponent::SaveInstance(PersistenceRecord* fields, ModelComponent* component) {
	ModelComponent::SaveInstance(fields, component);
}

bool ContinuousSystemComponent::Check(ModelComponent* component) {
	return ModelComponent::Check(component);
}

void ContinuousSystemComponent::CreateInternalData(ModelComponent* component) {
	ModelComponent::CreateInternalData(component);
}

void ContinuousSystemComponent::DispatchEvent(Event* event) {
	ModelComponent::DispatchEvent(event);
}

PluginInformation* ContinuousSystemComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(
			Util::TypeOf<ContinuousSystemComponent>(),
			&ContinuousSystemComponent::LoadInstance,
			&ContinuousSystemComponent::NewInstance);
	info->setCategory("Continuous");
	info->setDescriptionHelp("Continuous System driven by ODE Solver");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	return info;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ContinuousSystemComponent::ContinuousSystemComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<ContinuousSystemComponent>(), name) {
}

// ---------------------------------------------------------------------------
// show
// ---------------------------------------------------------------------------

std::string ContinuousSystemComponent::show() {
	return ModelComponent::show() +
	       ",odeSolver=\"" + _odeSolverName + "\"";
}

// ---------------------------------------------------------------------------
// Getters & setters
// ---------------------------------------------------------------------------

void ContinuousSystemComponent::setOdeSolver(ODESolver* solver) {
	_odeSolver = solver;
	if (solver != nullptr) {
		_odeSolverName = solver->getName();
	}
}

ODESolver* ContinuousSystemComponent::getOdeSolver() const {
	return _odeSolver;
}

void ContinuousSystemComponent::setOdeSolverName(std::string name) {
	_odeSolverName = name;
}

std::string ContinuousSystemComponent::getOdeSolverName() const {
	return _odeSolverName;
}

// ---------------------------------------------------------------------------
// _onDispatchEvent
// ---------------------------------------------------------------------------

void ContinuousSystemComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	std::cout << "[DEBUG onDispatch] chamado em simTime="
	          << _parentModel->getSimulation()->getSimulatedTime()
	          << " _odeSolver=" << _odeSolver << std::endl;

	if (_odeSolver == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "ContinuousSystemComponent \"" + getName() + "\": ODESolver not set.");
		_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
		return;
	}

	double simTime = _parentModel->getSimulation()->getSimulatedTime();
	double currentTimeBefore = _odeSolver->getCurrentTime();
	std::vector<std::string> stateVariableNames = _odeSolver->getStateVariableNames();
	std::vector<double> solverStateBefore = _odeSolver->getStateValues();
	std::string variableStateBefore;
	std::string solverStateTextBefore;
	for (unsigned int i = 0; i < stateVariableNames.size(); i++) {
		Variable* variable = dynamic_cast<Variable*>(_parentModel->getDataManager()->getDataDefinition(
				Util::TypeOf<Variable>(), stateVariableNames[i]));
		variableStateBefore += stateVariableNames[i] + "=" +
		                       std::to_string(variable != nullptr ? variable->getValue() : 0.0);
		solverStateTextBefore += stateVariableNames[i] + "=" +
		                         std::to_string(i < solverStateBefore.size() ? solverStateBefore[i] : 0.0);
		if (i + 1 < stateVariableNames.size()) {
			variableStateBefore += ", ";
			solverStateTextBefore += ", ";
		}
	}
	traceSimulation(this, TraceManager::Level::L9_mostDetailed,
	                "[ContinuousSystem] evento recebido: simTime=" + std::to_string(simTime) +
	                ", odeSolver._currentTime=" + std::to_string(currentTimeBefore) + " (antes)");
	traceSimulation(this, TraceManager::Level::L9_mostDetailed,
	                "[ContinuousSystem] variáveis antes: " + variableStateBefore);
	traceSimulation(this, TraceManager::Level::L9_mostDetailed,
	                "[ContinuousSystem] estado interno antes: " + solverStateTextBefore);

	// advance the continuous ODE from wherever it last stopped up to the current event time,
	// bridging the gap between the previous discrete event and this one
	_odeSolver->integrate(simTime);

	double currentTimeAfter = _odeSolver->getCurrentTime();
	std::vector<double> solverStateAfter = _odeSolver->getStateValues();
	std::string solverStateTextAfter;
	for (unsigned int i = 0; i < stateVariableNames.size(); i++) {
		solverStateTextAfter += stateVariableNames[i] + "=" +
		                        std::to_string(i < solverStateAfter.size() ? solverStateAfter[i] : 0.0);
		if (i + 1 < stateVariableNames.size()) {
			solverStateTextAfter += ", ";
		}
	}
	traceSimulation(this, TraceManager::Level::L9_mostDetailed,
	                "[ContinuousSystem] após integrate: _currentTime=" + std::to_string(currentTimeAfter) +
	                ", " + solverStateTextAfter);

	_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
}

// ---------------------------------------------------------------------------
// _check
// ---------------------------------------------------------------------------

bool ContinuousSystemComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createEditableDataDefinitions();

	// Try to resolve by name if the pointer is not yet set
	if (_odeSolver == nullptr && !_odeSolverName.empty()) {
		ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(
				Util::TypeOf<ODESolver>(), _odeSolverName);
		_odeSolver = dynamic_cast<ODESolver*>(def);
	}

	resultAll &= _parentModel->getDataManager()->check(
			Util::TypeOf<ODESolver>(), _odeSolver, "ODESolver", errorMessage);

	return resultAll;
}

// ---------------------------------------------------------------------------
// _loadInstance
// ---------------------------------------------------------------------------

bool ContinuousSystemComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_odeSolverName = fields->loadField("odeSolver", DEFAULT.odeSolverName);
		_odeSolver = nullptr;
		if (!_odeSolverName.empty()) {
			ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(
					Util::TypeOf<ODESolver>(), _odeSolverName);
			_odeSolver = dynamic_cast<ODESolver*>(def);
		}
	}
	return res;
}

// ---------------------------------------------------------------------------
// _saveInstance
// ---------------------------------------------------------------------------

void ContinuousSystemComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("odeSolver",
	                  _odeSolver != nullptr ? _odeSolver->getName() : _odeSolverName,
	                  DEFAULT.odeSolverName,
	                  saveDefaultValues);
}

// ---------------------------------------------------------------------------
// _createEditableDataDefinitions
// ---------------------------------------------------------------------------

void ContinuousSystemComponent::_createEditableDataDefinitions() {
	if (_odeSolver != nullptr) {
		_mandatoryEditableDataDefinitionInsert("ODESolver", _odeSolver);
	} else {
		_mandatoryEditableDataDefinitionRemove("ODESolver");
	}
}
