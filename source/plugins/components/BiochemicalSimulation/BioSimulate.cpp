/*
 * File:   BioSimulate.cpp
 * Author: GenESyS
 *
 * Component that executes BioNetwork simulation when entities arrive.
 */

#include "plugins/components/BiochemicalSimulation/BioSimulate.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioSimulate::GetPluginInformation;
}
#endif

ModelDataDefinition* BioSimulate::NewInstance(Model* model, std::string name) {
	return new BioSimulate(model, name);
}

BioSimulate::BioSimulate(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<BioSimulate>(), name) {
	auto* propBioNetwork = new SimulationControlGenericClass<BioNetwork*, Model*, BioNetwork>(
			_parentModel,
			std::bind(&BioSimulate::getBioNetwork, this),
			std::bind(&BioSimulate::setBioNetwork, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "BioNetwork", "");
	auto* propUseNetworkTimeWindow = new SimulationControlGeneric<bool>(
			std::bind(&BioSimulate::getUseNetworkTimeWindow, this),
			std::bind(&BioSimulate::setUseNetworkTimeWindow, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "UseNetworkTimeWindow", "");
	auto* propStartTime = new SimulationControlDouble(
			std::bind(&BioSimulate::getStartTime, this),
			std::bind(&BioSimulate::setStartTime, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "StartTime", "");
	auto* propStopTime = new SimulationControlDouble(
			std::bind(&BioSimulate::getStopTime, this),
			std::bind(&BioSimulate::setStopTime, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "StopTime", "");
	auto* propStepSize = new SimulationControlDouble(
			std::bind(&BioSimulate::getStepSize, this),
			std::bind(&BioSimulate::setStepSize, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "StepSize", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&BioSimulate::getLastSucceeded, this),
			std::bind(&BioSimulate::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "LastSucceeded", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulate::getLastMessage, this),
			std::bind(&BioSimulate::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<BioSimulate>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propBioNetwork);
	_parentModel->getControls()->insert(propUseNetworkTimeWindow);
	_parentModel->getControls()->insert(propStartTime);
	_parentModel->getControls()->insert(propStopTime);
	_parentModel->getControls()->insert(propStepSize);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propBioNetwork);
	_addSimulationControl(propUseNetworkTimeWindow);
	_addSimulationControl(propStartTime);
	_addSimulationControl(propStopTime);
	_addSimulationControl(propStepSize);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastMessage);
}

PluginInformation* BioSimulate::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioSimulate>(), &BioSimulate::LoadInstance, &BioSimulate::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("bionetwork.so");
	info->setDescriptionHelp("Triggers BioNetwork deterministic simulation when an entity arrives. "
	                         "The component can run the BioNetwork with its own configured start/stop/step "
	                         "or reuse the network time window, and then forwards the entity.");
	return info;
}

ModelComponent* BioSimulate::LoadInstance(Model* model, PersistenceRecord* fields) {
	BioSimulate* newComponent = new BioSimulate(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string BioSimulate::show() {
	return ModelComponent::show() +
	       ",bioNetwork=\"" + (_bioNetwork != nullptr ? _bioNetwork->getName() : std::string()) + "\"" +
	       ",useNetworkTimeWindow=" + std::to_string(_useNetworkTimeWindow ? 1 : 0) +
	       ",startTime=" + Util::StrTruncIfInt(std::to_string(_startTime)) +
	       ",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
	       ",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
	       ",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0);
}

bool BioSimulate::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string bioNetworkName = fields->loadField("bioNetwork", DEFAULT.bioNetworkName);
		_bioNetwork = nullptr;
		if (!bioNetworkName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), bioNetworkName);
			_bioNetwork = dynamic_cast<BioNetwork*>(definition);
		}
		_useNetworkTimeWindow = fields->loadField("useNetworkTimeWindow", DEFAULT.useNetworkTimeWindow ? 1u : 0u) != 0u;
		_startTime = fields->loadField("startTime", DEFAULT.startTime);
		_stopTime = fields->loadField("stopTime", DEFAULT.stopTime);
		_stepSize = fields->loadField("stepSize", DEFAULT.stepSize);
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void BioSimulate::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("bioNetwork", _bioNetwork != nullptr ? _bioNetwork->getName() : DEFAULT.bioNetworkName, DEFAULT.bioNetworkName, saveDefaultValues);
	fields->saveField("useNetworkTimeWindow", _useNetworkTimeWindow ? 1u : 0u, DEFAULT.useNetworkTimeWindow ? 1u : 0u, saveDefaultValues);
	fields->saveField("startTime", _startTime, DEFAULT.startTime, saveDefaultValues);
	fields->saveField("stopTime", _stopTime, DEFAULT.stopTime, saveDefaultValues);
	fields->saveField("stepSize", _stepSize, DEFAULT.stepSize, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool BioSimulate::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<BioNetwork>(), _bioNetwork, "BioNetwork", errorMessage);
	if (!_useNetworkTimeWindow) {
		if (_stepSize <= 0.0) {
			errorMessage += "BioSimulate \"" + getName() + "\" must define stepSize > 0 when UseNetworkTimeWindow=false. ";
			resultAll = false;
		}
		if (_stopTime < _startTime) {
			errorMessage += "BioSimulate \"" + getName() + "\" must define stopTime >= startTime when UseNetworkTimeWindow=false. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void BioSimulate::_createInternalAndAttachedData() {
	if (_bioNetwork != nullptr) {
		_attachedDataInsert("BioNetwork", _bioNetwork);
	} else {
		_attachedDataRemove("BioNetwork");
	}
}

void BioSimulate::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	std::string message;
	if (_bioNetwork == nullptr) {
		_lastSucceeded = false;
		_lastMessage = "BioSimulate requires a referenced BioNetwork.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	std::string errorMessage;
	bool success = false;
	if (_useNetworkTimeWindow) {
		success = _bioNetwork->simulate(errorMessage);
	} else {
		success = _bioNetwork->simulate(_startTime, _stopTime, _stepSize, errorMessage);
	}

	_lastSucceeded = success;
	if (success) {
		message = "BioSimulate executed BioNetwork \"" + _bioNetwork->getName() +
		          "\" with status \"" + _bioNetwork->getLastStatus() + "\".";
		traceSimulation(this, TraceManager::Level::L2_results, message);
		_lastMessage = message;
	} else {
		_lastMessage = errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "BioSimulate failed for BioNetwork \"" + _bioNetwork->getName() + "\": " + errorMessage);
	}
	_forwardEntity(entity);
}

void BioSimulate::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "BioSimulate dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void BioSimulate::setBioNetwork(BioNetwork* bioNetwork) {
	_bioNetwork = bioNetwork;
}

BioNetwork* BioSimulate::getBioNetwork() const {
	return _bioNetwork;
}

void BioSimulate::setUseNetworkTimeWindow(bool useNetworkTimeWindow) {
	_useNetworkTimeWindow = useNetworkTimeWindow;
}

bool BioSimulate::getUseNetworkTimeWindow() const {
	return _useNetworkTimeWindow;
}

void BioSimulate::setStartTime(double startTime) {
	_startTime = startTime;
}

double BioSimulate::getStartTime() const {
	return _startTime;
}

void BioSimulate::setStopTime(double stopTime) {
	_stopTime = stopTime;
}

double BioSimulate::getStopTime() const {
	return _stopTime;
}

void BioSimulate::setStepSize(double stepSize) {
	_stepSize = stepSize;
}

double BioSimulate::getStepSize() const {
	return _stepSize;
}

void BioSimulate::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool BioSimulate::getLastSucceeded() const {
	return _lastSucceeded;
}

void BioSimulate::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string BioSimulate::getLastMessage() const {
	return _lastMessage;
}

void BioSimulate::_createReportStatisticsDataDefinitions() {
}

void BioSimulate::_createEditableDataDefinitions() {
}

void BioSimulate::_createOthersDataDefinitions() {
}
