/*
 * File:   ContinuousSystemComponent.cpp
 * Author: GenESyS
 *
 * ModelComponent that drives continuous-time simulation by delegating
 * numerical integration to an ODESolver data definition.
 */

#include "plugins/components/Continuous/ContinuousSystemComponent.h"

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "plugins/data/Logic/Variable.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ContinuousSystemComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* ContinuousSystemComponent::NewInstance(Model* model, std::string name) {
	return new ContinuousSystemComponent(model, name);
}

ModelComponent* ContinuousSystemComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	ContinuousSystemComponent* newComponent = new ContinuousSystemComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception&) {
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

ContinuousSystemComponent::ContinuousSystemComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<ContinuousSystemComponent>(), name) {
}

std::string ContinuousSystemComponent::show() {
	return ModelComponent::show() + ",odeSolver=\"" + _odeSolverName + "\"";
}

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

void ContinuousSystemComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_odeSolver == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "ContinuousSystemComponent \"" + getName() + "\": ODESolver not set.");
		_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
		return;
	}

	const double simTime = _parentModel->getSimulation()->getSimulatedTime();
	_odeSolver->integrate(simTime);
	_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
}

bool ContinuousSystemComponent::_check(std::string& errorMessage) {
	_createEditableDataDefinitions();
	if (_odeSolver == nullptr && !_odeSolverName.empty()) {
		ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(
				Util::TypeOf<ODESolver>(), _odeSolverName);
		_odeSolver = dynamic_cast<ODESolver*>(def);
	}
	return _parentModel->getDataManager()->check(
			Util::TypeOf<ODESolver>(), _odeSolver, "ODESolver", errorMessage);
}

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

void ContinuousSystemComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("odeSolver",
	                  _odeSolver != nullptr ? _odeSolver->getName() : _odeSolverName,
	                  DEFAULT.odeSolverName,
	                  saveDefaultValues);
}

void ContinuousSystemComponent::_createEditableDataDefinitions() {
	if (_odeSolver != nullptr) {
		_mandatoryEditableDataDefinitionInsert("ODESolver", _odeSolver);
	} else {
		_mandatoryEditableDataDefinitionRemove("ODESolver");
	}
}
