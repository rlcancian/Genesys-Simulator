/*
 * File:   RSimulator.cpp
 * Author: GenESyS
 *
 * Component that executes R commands when entities arrive.
 */

#include "plugins/components/ExternalIntegration/RSimulator.h"

#include <functional>
#include <sstream>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &RSimulator::GetPluginInformation;
}
#endif

namespace {

std::string trimText(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\n\r\f\v");
	if (first == std::string::npos) {
		return "";
	}
	const size_t last = text.find_last_not_of(" \t\n\r\f\v");
	return text.substr(first, last - first + 1);
}

std::string truncateForTrace(const std::string& text, size_t maxSize = 512u) {
	if (text.size() <= maxSize) {
		return text;
	}
	return text.substr(0u, maxSize) + "...";
}

} // namespace

ModelDataDefinition* RSimulator::NewInstance(Model* model, std::string name) {
	return new RSimulator(model, name);
}

RSimulator::RSimulator(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<RSimulator>(), name) {
	SimulationControlGeneric<std::string>* propRExecutable = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulator::getRExecutable, this), std::bind(&RSimulator::setRExecutable, this, std::placeholders::_1),
			Util::TypeOf<RSimulator>(), getName(), "RExecutable", "");
	SimulationControlGeneric<std::string>* propWorkingDirectory = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulator::getWorkingDirectory, this), std::bind(&RSimulator::setWorkingDirectory, this, std::placeholders::_1),
			Util::TypeOf<RSimulator>(), getName(), "WorkingDirectory", "");
	SimulationControlGeneric<std::string>* propPreludeScript = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulator::getPreludeScript, this), std::bind(&RSimulator::setPreludeScript, this, std::placeholders::_1),
			Util::TypeOf<RSimulator>(), getName(), "PreludeScript", "");
	SimulationControlGeneric<std::string>* propCommandList = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulator::getCommandList, this), std::bind(&RSimulator::setCommandList, this, std::placeholders::_1),
			Util::TypeOf<RSimulator>(), getName(), "CommandList", "");

	_parentModel->getControls()->insert(propRExecutable);
	_parentModel->getControls()->insert(propWorkingDirectory);
	_parentModel->getControls()->insert(propPreludeScript);
	_parentModel->getControls()->insert(propCommandList);

	_addProperty(propRExecutable);
	_addProperty(propWorkingDirectory);
	_addProperty(propPreludeScript);
	_addProperty(propCommandList);
}

std::string RSimulator::show() {
	return ModelComponent::show() +
			",rExecutable=\"" + _rExecutable + "\"" +
			",workingDirectory=\"" + _workingDirectory + "\"" +
			",preludeScriptSize=" + std::to_string(_preludeScript.size()) +
			",commands=" + std::to_string(_commands->size()) +
			",runner=\"" + (_runner != nullptr ? _runner->getName() : std::string("")) + "\"";
}

ModelComponent* RSimulator::LoadInstance(Model* model, PersistenceRecord *fields) {
	RSimulator* newComponent = new RSimulator(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

PluginInformation* RSimulator::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<RSimulator>(), &RSimulator::LoadInstance, &RSimulator::NewInstance);
	info->setCategory("External statistical integration");
	info->insertDynamicLibFileDependence("rsimulatorrunner.so");
	info->setDescriptionHelp("Executes a sequence of R commands whenever an entity reaches the component. Commands are delegated to an internal RSimulatorRunner, which writes a temporary R script, runs Rscript --vanilla and stores stdout, stderr, status and exit code. This first component version traces the runner result and then forwards the entity.");
	info->setObservation("Requires the RSimulatorRunner data plugin and Rscript available in the operating system.");
	return info;
}

void RSimulator::setRExecutable(std::string rExecutable) {
	_rExecutable = rExecutable;
	_syncRunnerConfiguration();
}

std::string RSimulator::getRExecutable() const {
	return _rExecutable;
}

void RSimulator::setWorkingDirectory(std::string workingDirectory) {
	_workingDirectory = workingDirectory;
	_syncRunnerConfiguration();
}

std::string RSimulator::getWorkingDirectory() const {
	return _workingDirectory;
}

void RSimulator::setPreludeScript(std::string preludeScript) {
	_preludeScript = preludeScript;
	_syncRunnerConfiguration();
}

std::string RSimulator::getPreludeScript() const {
	return _preludeScript;
}

void RSimulator::insertCommand(std::string command) {
	_commands->insert(command);
}

void RSimulator::setCommands(std::list<std::string> commands) {
	_commands->clear();
	for (const std::string& command : commands) {
		_commands->insert(command);
	}
}

List<std::string>* RSimulator::getCommands() const {
	return _commands;
}

void RSimulator::setCommandList(std::string commandList) {
	_commands->clear();
	std::istringstream stream(commandList);
	std::string line;
	while (std::getline(stream, line)) {
		if (_isNonEmptyCommand(line)) {
			_commands->insert(line);
		}
	}
}

std::string RSimulator::getCommandList() const {
	std::string commandList;
	for (const std::string& command : *_commands->list()) {
		if (!commandList.empty()) {
			commandList += "\n";
		}
		commandList += command;
	}
	return commandList;
}

RSimulatorRunner* RSimulator::getRunner() {
	_ensureRunner();
	return _runner;
}

void RSimulator::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;

	_ensureRunner();
	_syncRunnerConfiguration();

	unsigned int executedCommands = 0u;
	for (const std::string& command : *_commands->list()) {
		if (!_isNonEmptyCommand(command)) {
			continue;
		}

		executedCommands++;
		_runner->setCommand(command);
		std::string errorMessage;
		const bool success = _runner->executeCommand(errorMessage);

		// This first component version reports R interaction through trace while the runner keeps full fields.
		std::string traceMessage = "RSimulator command #" + std::to_string(executedCommands) +
				" status=" + _runner->getLastStatus() +
				", exitCode=" + std::to_string(_runner->getLastExitCode()) +
				", command=\"" + truncateForTrace(trimText(command), 160u) + "\"";
		if (!_runner->getLastStdout().empty()) {
			traceMessage += ", stdout=\"" + truncateForTrace(_runner->getLastStdout()) + "\"";
		}
		if (!_runner->getLastStderr().empty()) {
			traceMessage += ", stderr=\"" + truncateForTrace(_runner->getLastStderr()) + "\"";
		}
		if (!success && !errorMessage.empty()) {
			traceMessage += ", error=\"" + truncateForTrace(errorMessage) + "\"";
		}
		traceSimulation(this, success ? TraceManager::Level::L2_results : TraceManager::Level::L1_errorFatal, traceMessage);
	}

	if (executedCommands == 0u) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal, "RSimulator has no R commands to execute.");
	}

	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "RSimulator dispatch skipped: invalid front connection");
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

bool RSimulator::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_rExecutable = fields->loadField("rExecutable", DEFAULT.rExecutable);
		_workingDirectory = fields->loadField("workingDirectory", DEFAULT.workingDirectory);
		_preludeScript = fields->loadField("preludeScript", DEFAULT.preludeScript);
		_commands->clear();
		const unsigned int commandCount = fields->loadField("commands", 0u);
		for (unsigned int i = 0u; i < commandCount; i++) {
			_commands->insert(fields->loadField("command" + Util::StrIndex(i), ""));
		}
		_syncRunnerConfiguration();
	}
	return res;
}

void RSimulator::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("rExecutable", _rExecutable, DEFAULT.rExecutable, saveDefaultValues);
	fields->saveField("workingDirectory", _workingDirectory, DEFAULT.workingDirectory, saveDefaultValues);
	fields->saveField("preludeScript", _preludeScript, DEFAULT.preludeScript, saveDefaultValues);
	fields->saveField("commands", _commands->size(), 0u, saveDefaultValues);
	unsigned int i = 0u;
	for (const std::string& command : *_commands->list()) {
		fields->saveField("command" + Util::StrIndex(i), command, std::string(""), saveDefaultValues);
		i++;
	}
}

bool RSimulator::_check(std::string& errorMessage) {
	bool resultAll = ModelDataDefinition::_check(errorMessage);
	_ensureRunner();
	_syncRunnerConfiguration();

	if (trimText(_rExecutable).empty()) {
		errorMessage += "RSimulator \"" + getName() + "\" must define a non-empty RExecutable. ";
		resultAll = false;
	}

	bool hasCommand = false;
	for (const std::string& command : *_commands->list()) {
		hasCommand = hasCommand || _isNonEmptyCommand(command);
	}
	if (!hasCommand) {
		errorMessage += "RSimulator \"" + getName() + "\" must define at least one R command. ";
		resultAll = false;
	}
	return resultAll;
}

void RSimulator::_createInternalAndAttachedData() {
	_ensureRunner();
	_syncRunnerConfiguration();
	if (_runner != nullptr) {
		_internalDataInsert("RSimulatorRunner", _runner);
	}
}

void RSimulator::_ensureRunner() {
	if (_runner == nullptr) {
		_runner = new RSimulatorRunner(_parentModel, getName() + ".RSimulatorRunner");
	}
}

void RSimulator::_syncRunnerConfiguration() {
	if (_runner == nullptr) {
		return;
	}
	_runner->setRExecutable(_rExecutable);
	_runner->setWorkingDirectory(_workingDirectory);
	_runner->setPreludeScript(_preludeScript);
}

bool RSimulator::_isNonEmptyCommand(const std::string& command) const {
	return !trimText(command).empty();
}
