/*
 * File:   BioRunnerCommand.cpp
 * Author: GenESyS
 *
 * Component that executes BioSimulatorRunner commands when entities arrive.
 */

#include "plugins/components/BiochemicalSimulation/BioRunnerCommand.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioRunnerCommand::GetPluginInformation;
}
#endif

ModelDataDefinition* BioRunnerCommand::NewInstance(Model* model, std::string name) {
	return new BioRunnerCommand(model, name);
}

BioRunnerCommand::BioRunnerCommand(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<BioRunnerCommand>(), name) {
	auto* propRunner = new SimulationControlGenericClass<BioSimulatorRunner*, Model*, BioSimulatorRunner>(
			_parentModel,
			std::bind(&BioRunnerCommand::getRunner, this),
			std::bind(&BioRunnerCommand::setRunner, this, std::placeholders::_1),
			Util::TypeOf<BioRunnerCommand>(), getName(), "Runner", "");
	auto* propCommand = new SimulationControlGeneric<std::string>(
			std::bind(&BioRunnerCommand::getCommand, this),
			std::bind(&BioRunnerCommand::setCommand, this, std::placeholders::_1),
			Util::TypeOf<BioRunnerCommand>(), getName(), "Command", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&BioRunnerCommand::getLastSucceeded, this),
			std::bind(&BioRunnerCommand::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<BioRunnerCommand>(), getName(), "LastSucceeded", "");
	auto* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&BioRunnerCommand::getLastStatus, this),
			std::bind(&BioRunnerCommand::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<BioRunnerCommand>(), getName(), "LastStatus", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioRunnerCommand::getLastMessage, this),
			std::bind(&BioRunnerCommand::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<BioRunnerCommand>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propRunner);
	_parentModel->getControls()->insert(propCommand);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propRunner);
	_addSimulationControl(propCommand);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastStatus);
	_addSimulationControl(propLastMessage);
}

PluginInformation* BioRunnerCommand::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioRunnerCommand>(), &BioRunnerCommand::LoadInstance, &BioRunnerCommand::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("biosimulatorrunner.so");
	info->setDescriptionHelp("Executes one configured BioSimulatorRunner command when an entity arrives, "
	                         "stores command status and message, and then forwards the entity.");
	return info;
}

ModelComponent* BioRunnerCommand::LoadInstance(Model* model, PersistenceRecord* fields) {
	BioRunnerCommand* newComponent = new BioRunnerCommand(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string BioRunnerCommand::show() {
	return ModelComponent::show() +
	       ",runner=\"" + (_runner != nullptr ? _runner->getName() : std::string()) + "\"" +
	       ",command=\"" + _command + "\"" +
	       ",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0) +
	       ",lastStatus=\"" + _lastStatus + "\"";
}

bool BioRunnerCommand::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string runnerName = fields->loadField("runner", DEFAULT.runnerName);
		_runner = nullptr;
		if (!runnerName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSimulatorRunner>(), runnerName);
			_runner = dynamic_cast<BioSimulatorRunner*>(definition);
		}
		_command = fields->loadField("command", DEFAULT.command);
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void BioRunnerCommand::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("runner", _runner != nullptr ? _runner->getName() : DEFAULT.runnerName, DEFAULT.runnerName, saveDefaultValues);
	fields->saveField("command", _command, DEFAULT.command, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool BioRunnerCommand::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<BioSimulatorRunner>(), _runner, "Runner", errorMessage);
	if (_command.empty()) {
		errorMessage += "BioRunnerCommand \"" + getName() + "\" must define a non-empty command. ";
		resultAll = false;
	}
	return resultAll;
}

void BioRunnerCommand::_createInternalAndAttachedData() {
	if (_runner != nullptr) {
		_attachedDataInsert("BioSimulatorRunner", _runner);
	} else {
		_attachedDataRemove("BioSimulatorRunner");
	}
}

void BioRunnerCommand::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_runner == nullptr) {
		_lastSucceeded = false;
		_lastStatus = "Failed";
		_lastMessage = "BioRunnerCommand requires a referenced BioSimulatorRunner.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	_runner->setCommand(_command);
	std::string errorMessage;
	_lastSucceeded = _runner->executeCommand(errorMessage);
	_lastStatus = _runner->getLastStatus();
	if (_lastSucceeded) {
		_lastMessage = _runner->getLastResponsePayload();
		traceSimulation(this, TraceManager::Level::L2_results,
		                "BioRunnerCommand executed \"" + _command + "\" with status \"" + _lastStatus + "\".");
	} else {
		_lastMessage = errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "BioRunnerCommand failed for \"" + _command + "\": " + errorMessage);
	}
	_forwardEntity(entity);
}

void BioRunnerCommand::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "BioRunnerCommand dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void BioRunnerCommand::setRunner(BioSimulatorRunner* runner) {
	_runner = runner;
}

BioSimulatorRunner* BioRunnerCommand::getRunner() const {
	return _runner;
}

void BioRunnerCommand::setCommand(std::string command) {
	_command = command;
}

std::string BioRunnerCommand::getCommand() const {
	return _command;
}

void BioRunnerCommand::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool BioRunnerCommand::getLastSucceeded() const {
	return _lastSucceeded;
}

void BioRunnerCommand::setLastStatus(std::string lastStatus) {
	_lastStatus = lastStatus;
}

std::string BioRunnerCommand::getLastStatus() const {
	return _lastStatus;
}

void BioRunnerCommand::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string BioRunnerCommand::getLastMessage() const {
	return _lastMessage;
}

void BioRunnerCommand::_createReportStatisticsDataDefinitions() {
}

void BioRunnerCommand::_createEditableDataDefinitions() {
}

void BioRunnerCommand::_createOthersDataDefinitions() {
}
