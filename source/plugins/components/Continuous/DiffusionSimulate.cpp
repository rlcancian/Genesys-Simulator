/*
 * File:   DiffusionSimulate.cpp
 * Author: GenESyS DCS (Tema 8.2)
 *
 * Component that advances a DiffusionField (continuous N-D diffusion) when
 * entities arrive, mirroring the BioSimulate -> BioNetwork pattern.
 */

#include "plugins/components/Continuous/DiffusionSimulate.h"

#include <functional>

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DiffusionSimulate::GetPluginInformation;
}
#endif

ModelDataDefinition* DiffusionSimulate::NewInstance(Model* model, std::string name) {
	return new DiffusionSimulate(model, name);
}

DiffusionSimulate::DiffusionSimulate(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<DiffusionSimulate>(), name) {
	auto* propDiffusionField = new SimulationControlGenericClass<DiffusionField*, Model*, DiffusionField>(
			_parentModel,
			std::bind(&DiffusionSimulate::getDiffusionField, this),
			std::bind(&DiffusionSimulate::setDiffusionField, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "DiffusionField", "");
	auto* propUseFieldTimeWindow = new SimulationControlGeneric<bool>(
			std::bind(&DiffusionSimulate::getUseFieldTimeWindow, this),
			std::bind(&DiffusionSimulate::setUseFieldTimeWindow, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "UseFieldTimeWindow", "");
	auto* propStartTime = new SimulationControlDouble(
			std::bind(&DiffusionSimulate::getStartTime, this),
			std::bind(&DiffusionSimulate::setStartTime, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "StartTime", "");
	auto* propStopTime = new SimulationControlDouble(
			std::bind(&DiffusionSimulate::getStopTime, this),
			std::bind(&DiffusionSimulate::setStopTime, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "StopTime", "");
	auto* propStepSize = new SimulationControlDouble(
			std::bind(&DiffusionSimulate::getStepSize, this),
			std::bind(&DiffusionSimulate::setStepSize, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "StepSize", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&DiffusionSimulate::getLastSucceeded, this),
			std::bind(&DiffusionSimulate::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "LastSucceeded", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&DiffusionSimulate::getLastMessage, this),
			std::bind(&DiffusionSimulate::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<DiffusionSimulate>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propDiffusionField);
	_parentModel->getControls()->insert(propUseFieldTimeWindow);
	_parentModel->getControls()->insert(propStartTime);
	_parentModel->getControls()->insert(propStopTime);
	_parentModel->getControls()->insert(propStepSize);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propDiffusionField);
	_addSimulationControl(propUseFieldTimeWindow);
	_addSimulationControl(propStartTime);
	_addSimulationControl(propStopTime);
	_addSimulationControl(propStepSize);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastMessage);
}

PluginInformation* DiffusionSimulate::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DiffusionSimulate>(), &DiffusionSimulate::LoadInstance, &DiffusionSimulate::NewInstance);
	info->setCategory("Mathematical/PDE");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("diffusionfield.so");
	info->setDescriptionHelp("Triggers a DiffusionField continuous N-dimensional diffusion solve when an "
	                         "entity arrives. The component can run the field with its own configured "
	                         "start/stop/step or reuse the field time window, and then forwards the entity. "
	                         "For event-clock-synchronized advance, enable AutoSchedule on the DiffusionField.");
	return info;
}

ModelComponent* DiffusionSimulate::LoadInstance(Model* model, PersistenceRecord* fields) {
	DiffusionSimulate* newComponent = new DiffusionSimulate(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string DiffusionSimulate::show() {
	return ModelComponent::show() +
	       ",diffusionField=\"" + (_diffusionField != nullptr ? _diffusionField->getName() : std::string()) + "\"" +
	       ",useFieldTimeWindow=" + std::to_string(_useFieldTimeWindow ? 1 : 0) +
	       ",startTime=" + Util::StrTruncIfInt(std::to_string(_startTime)) +
	       ",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
	       ",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
	       ",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0);
}

bool DiffusionSimulate::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string diffusionFieldName = fields->loadField("diffusionField", DEFAULT.diffusionFieldName);
		_diffusionField = nullptr;
		if (!diffusionFieldName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<DiffusionField>(), diffusionFieldName);
			_diffusionField = dynamic_cast<DiffusionField*>(definition);
		}
		_useFieldTimeWindow = fields->loadField("useFieldTimeWindow", DEFAULT.useFieldTimeWindow ? 1u : 0u) != 0u;
		_startTime = fields->loadField("startTime", DEFAULT.startTime);
		_stopTime = fields->loadField("stopTime", DEFAULT.stopTime);
		_stepSize = fields->loadField("stepSize", DEFAULT.stepSize);
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void DiffusionSimulate::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("diffusionField", _diffusionField != nullptr ? _diffusionField->getName() : DEFAULT.diffusionFieldName, DEFAULT.diffusionFieldName, saveDefaultValues);
	fields->saveField("useFieldTimeWindow", _useFieldTimeWindow ? 1u : 0u, DEFAULT.useFieldTimeWindow ? 1u : 0u, saveDefaultValues);
	fields->saveField("startTime", _startTime, DEFAULT.startTime, saveDefaultValues);
	fields->saveField("stopTime", _stopTime, DEFAULT.stopTime, saveDefaultValues);
	fields->saveField("stepSize", _stepSize, DEFAULT.stepSize, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool DiffusionSimulate::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createEditableDataDefinitions();

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<DiffusionField>(), _diffusionField, "DiffusionField", errorMessage);
	if (!_useFieldTimeWindow) {
		if (_stepSize <= 0.0) {
			errorMessage += "DiffusionSimulate \"" + getName() + "\" must define stepSize > 0 when UseFieldTimeWindow=false. ";
			resultAll = false;
		}
		if (_stopTime < _startTime) {
			errorMessage += "DiffusionSimulate \"" + getName() + "\" must define stopTime >= startTime when UseFieldTimeWindow=false. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void DiffusionSimulate::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	std::string message;
	if (_diffusionField == nullptr) {
		_lastSucceeded = false;
		_lastMessage = "DiffusionSimulate requires a referenced DiffusionField.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	std::string errorMessage;
	bool success = false;
	if (_useFieldTimeWindow) {
		success = _diffusionField->simulate(errorMessage);
	} else {
		success = _diffusionField->simulate(_startTime, _stopTime, _stepSize, errorMessage);
	}

	_lastSucceeded = success;
	if (success) {
		message = "DiffusionSimulate executed DiffusionField \"" + _diffusionField->getName() +
		          "\" with status \"" + _diffusionField->getLastStatus() + "\".";
		traceSimulation(this, TraceManager::Level::L2_results, message);
		_lastMessage = message;
	} else {
		_lastMessage = errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "DiffusionSimulate failed for DiffusionField \"" + _diffusionField->getName() + "\": " + errorMessage);
	}
	_forwardEntity(entity);
}

void DiffusionSimulate::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "DiffusionSimulate dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void DiffusionSimulate::setDiffusionField(DiffusionField* diffusionField) {
	_diffusionField = diffusionField;
}

DiffusionField* DiffusionSimulate::getDiffusionField() const {
	return _diffusionField;
}

void DiffusionSimulate::setUseFieldTimeWindow(bool useFieldTimeWindow) {
	_useFieldTimeWindow = useFieldTimeWindow;
}

bool DiffusionSimulate::getUseFieldTimeWindow() const {
	return _useFieldTimeWindow;
}

void DiffusionSimulate::setStartTime(double startTime) {
	_startTime = startTime;
}

double DiffusionSimulate::getStartTime() const {
	return _startTime;
}

void DiffusionSimulate::setStopTime(double stopTime) {
	_stopTime = stopTime;
}

double DiffusionSimulate::getStopTime() const {
	return _stopTime;
}

void DiffusionSimulate::setStepSize(double stepSize) {
	_stepSize = stepSize;
}

double DiffusionSimulate::getStepSize() const {
	return _stepSize;
}

void DiffusionSimulate::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool DiffusionSimulate::getLastSucceeded() const {
	return _lastSucceeded;
}

void DiffusionSimulate::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string DiffusionSimulate::getLastMessage() const {
	return _lastMessage;
}

void DiffusionSimulate::_createEditableDataDefinitions() {
	if (_diffusionField != nullptr) {
		_optionalEditableDataDefinitionInsert("DiffusionField", _diffusionField);
	} else {
		_optionalEditableDataDefinitionRemove("DiffusionField");
	}
}
