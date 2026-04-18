/*
 * File:   RSimulatorRunner.cpp
 * Author: GenESyS
 *
 * R integration data definition.
 */

#include "plugins/data/ExternalIntegration/RSimulatorRunner.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include <sys/wait.h>
#include <unistd.h>

#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &RSimulatorRunner::GetPluginInformation;
}
#endif

namespace {

std::string trim(const std::string& text) {
	const size_t first = text.find_first_not_of(" \t\n\r\f\v");
	if (first == std::string::npos) {
		return "";
	}
	const size_t last = text.find_last_not_of(" \t\n\r\f\v");
	return text.substr(first, last - first + 1);
}

std::string shellQuote(const std::string& text) {
	std::string quoted = "'";
	for (const char ch : text) {
		if (ch == '\'') {
			quoted += "'\\''";
		} else {
			quoted += ch;
		}
	}
	quoted += "'";
	return quoted;
}

std::string readTextFile(const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		return "";
	}
	std::ostringstream contents;
	contents << file.rdbuf();
	return contents.str();
}

int normalizeSystemExitCode(int status) {
	if (status == -1) {
		return -1;
	}
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	}
	return status;
}

bool hasPathSeparator(const std::string& text) {
	return text.find('/') != std::string::npos || text.find('\\') != std::string::npos;
}

std::filesystem::path resolveWorkingDirectory(const std::string& configuredWorkingDirectory) {
	if (trim(configuredWorkingDirectory).empty()) {
		return std::filesystem::temp_directory_path();
	}
	return std::filesystem::path(configuredWorkingDirectory);
}

std::string makeUniqueStem(const std::string& name) {
	const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
	return "genesys_r_" + name + "_" + std::to_string(static_cast<long long>(::getpid())) + "_" + std::to_string(now);
}

bool executableLooksAvailable(const std::string& executable, std::string& errorMessage) {
	if (trim(executable).empty()) {
		errorMessage = "R executable must not be empty.";
		return false;
	}
	if (hasPathSeparator(executable)) {
		if (!std::filesystem::exists(executable)) {
			errorMessage = "R executable \"" + executable + "\" was not found.";
			return false;
		}
		return true;
	}

	const std::string command = "command -v " + shellQuote(executable) + " >/dev/null 2>&1";
	const int exitCode = normalizeSystemExitCode(std::system(command.c_str()));
	if (exitCode != 0) {
		errorMessage = "R executable \"" + executable + "\" was not found in PATH.";
		return false;
	}
	return true;
}

} // namespace

ModelDataDefinition* RSimulatorRunner::NewInstance(Model* model, std::string name) {
	return new RSimulatorRunner(model, name);
}

RSimulatorRunner::RSimulatorRunner(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<RSimulatorRunner>(), name) {
	SimulationControlGeneric<std::string>* propRExecutable = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getRExecutable, this), std::bind(&RSimulatorRunner::setRExecutable, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "RExecutable", "");
	SimulationControlGeneric<std::string>* propWorkingDirectory = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getWorkingDirectory, this), std::bind(&RSimulatorRunner::setWorkingDirectory, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "WorkingDirectory", "");
	SimulationControlGeneric<std::string>* propPreludeScript = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getPreludeScript, this), std::bind(&RSimulatorRunner::setPreludeScript, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "PreludeScript", "");
	SimulationControlGeneric<std::string>* propCommand = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getCommand, this), std::bind(&RSimulatorRunner::setCommand, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "Command", "");
	SimulationControlGeneric<std::string>* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastStatus, this), std::bind(&RSimulatorRunner::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastStatus", "");
	SimulationControlGeneric<int>* propLastExitCode = new SimulationControlGeneric<int>(
			std::bind(&RSimulatorRunner::getLastExitCode, this), std::bind(&RSimulatorRunner::setLastExitCode, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastExitCode", "");
	SimulationControlGeneric<std::string>* propLastStdout = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastStdout, this), std::bind(&RSimulatorRunner::setLastStdout, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastStdout", "");
	SimulationControlGeneric<std::string>* propLastStderr = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastStderr, this), std::bind(&RSimulatorRunner::setLastStderr, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastStderr", "");
	SimulationControlGeneric<std::string>* propLastResponsePayload = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastResponsePayload, this), std::bind(&RSimulatorRunner::setLastResponsePayload, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastResponsePayload", "");
	SimulationControlGeneric<std::string>* propLastScriptFilename = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastScriptFilename, this), std::bind(&RSimulatorRunner::setLastScriptFilename, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastScriptFilename", "");
	SimulationControlGeneric<std::string>* propLastResponseFilename = new SimulationControlGeneric<std::string>(
			std::bind(&RSimulatorRunner::getLastResponseFilename, this), std::bind(&RSimulatorRunner::setLastResponseFilename, this, std::placeholders::_1),
			Util::TypeOf<RSimulatorRunner>(), getName(), "LastResponseFilename", "");

	_parentModel->getControls()->insert(propRExecutable);
	_parentModel->getControls()->insert(propWorkingDirectory);
	_parentModel->getControls()->insert(propPreludeScript);
	_parentModel->getControls()->insert(propCommand);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastExitCode);
	_parentModel->getControls()->insert(propLastStdout);
	_parentModel->getControls()->insert(propLastStderr);
	_parentModel->getControls()->insert(propLastResponsePayload);
	_parentModel->getControls()->insert(propLastScriptFilename);
	_parentModel->getControls()->insert(propLastResponseFilename);

	_addProperty(propRExecutable);
	_addProperty(propWorkingDirectory);
	_addProperty(propPreludeScript);
	_addProperty(propCommand);
	_addProperty(propLastStatus);
	_addProperty(propLastExitCode);
	_addProperty(propLastStdout);
	_addProperty(propLastStderr);
	_addProperty(propLastResponsePayload);
	_addProperty(propLastScriptFilename);
	_addProperty(propLastResponseFilename);
}

PluginInformation* RSimulatorRunner::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<RSimulatorRunner>(), &RSimulatorRunner::LoadInstance, &RSimulatorRunner::NewInstance);
	info->setCategory("External statistical integration");
	info->setDescriptionHelp("Executes R code through Rscript. The runner writes the configured prelude and command into a temporary .R script, runs Rscript --vanilla, captures stdout/stderr and stores status, exit code and response payload in the model data definition.");
	info->setObservation("First R integration runner. It executes local R scripts through Rscript and does not require additional R packages.");
	info->insertSystemDependency(SystemDependency(
			SystemDependency::OS::Linux,
			"R",
			"sudo apt install -y r-base",
			"Rscript --version"));
	return info;
}

ModelDataDefinition* RSimulatorRunner::LoadInstance(Model* model, PersistenceRecord *fields) {
	RSimulatorRunner* newElement = new RSimulatorRunner(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string RSimulatorRunner::show() {
	return ModelDataDefinition::show() +
			",rExecutable=\"" + _rExecutable + "\"" +
			",workingDirectory=\"" + _workingDirectory + "\"" +
			",commandSize=" + std::to_string(_command.size()) +
			",preludeScriptSize=" + std::to_string(_preludeScript.size()) +
			",lastStatus=\"" + _lastStatus + "\"" +
			",lastExitCode=" + std::to_string(_lastExitCode) +
			",lastStdoutSize=" + std::to_string(_lastStdout.size()) +
			",lastStderrSize=" + std::to_string(_lastStderr.size()) +
			",lastResponsePayloadSize=" + std::to_string(_lastResponsePayload.size()) +
			",lastScriptFilename=\"" + _lastScriptFilename + "\"" +
			",lastResponseFilename=\"" + _lastResponseFilename + "\"";
}

bool RSimulatorRunner::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		// Persistence mirrors _saveInstance so a runner can be re-executed after model reload.
		_rExecutable = fields->loadField("rExecutable", DEFAULT.rExecutable);
		_workingDirectory = fields->loadField("workingDirectory", DEFAULT.workingDirectory);
		_preludeScript = fields->loadField("preludeScript", DEFAULT.preludeScript);
		_command = fields->loadField("command", DEFAULT.command);
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastExitCode = fields->loadField("lastExitCode", DEFAULT.lastExitCode);
		_lastStdout = fields->loadField("lastStdout", DEFAULT.lastStdout);
		_lastStderr = fields->loadField("lastStderr", DEFAULT.lastStderr);
		_lastResponsePayload = fields->loadField("lastResponsePayload", DEFAULT.lastResponsePayload);
		_lastScriptFilename = fields->loadField("lastScriptFilename", DEFAULT.lastScriptFilename);
		_lastResponseFilename = fields->loadField("lastResponseFilename", DEFAULT.lastResponseFilename);
	}
	return res;
}

void RSimulatorRunner::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	// Persist both configuration and observability fields so the last R execution is reproducible/auditable.
	fields->saveField("rExecutable", _rExecutable, DEFAULT.rExecutable, saveDefaultValues);
	fields->saveField("workingDirectory", _workingDirectory, DEFAULT.workingDirectory, saveDefaultValues);
	fields->saveField("preludeScript", _preludeScript, DEFAULT.preludeScript, saveDefaultValues);
	fields->saveField("command", _command, DEFAULT.command, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastExitCode", _lastExitCode, DEFAULT.lastExitCode, saveDefaultValues);
	fields->saveField("lastStdout", _lastStdout, DEFAULT.lastStdout, saveDefaultValues);
	fields->saveField("lastStderr", _lastStderr, DEFAULT.lastStderr, saveDefaultValues);
	fields->saveField("lastResponsePayload", _lastResponsePayload, DEFAULT.lastResponsePayload, saveDefaultValues);
	fields->saveField("lastScriptFilename", _lastScriptFilename, DEFAULT.lastScriptFilename, saveDefaultValues);
	fields->saveField("lastResponseFilename", _lastResponseFilename, DEFAULT.lastResponseFilename, saveDefaultValues);
}

bool RSimulatorRunner::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (trim(_rExecutable).empty()) {
		errorMessage += "RSimulatorRunner \"" + getName() + "\" must define a non-empty RExecutable. ";
		resultAll = false;
	}
	if (trim(_command).empty()) {
		errorMessage += "RSimulatorRunner \"" + getName() + "\" must define a non-empty command. ";
		resultAll = false;
	}
	if (!trim(_workingDirectory).empty()) {
		std::error_code errorCode;
		const std::filesystem::path workingDir(_workingDirectory);
		if (!std::filesystem::exists(workingDir, errorCode) || !std::filesystem::is_directory(workingDir, errorCode)) {
			errorMessage += "RSimulatorRunner \"" + getName() + "\" workingDirectory must exist and be a directory. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void RSimulatorRunner::_createInternalAndAttachedData() {
}

bool RSimulatorRunner::executeCommand(std::string& errorMessage) {
	errorMessage.clear();

	std::string checkError;
	if (!_check(checkError)) {
		_lastStatus = "Failed";
		_lastExitCode = -1;
		_lastStdout.clear();
		_lastStderr.clear();
		_lastResponsePayload.clear();
		_lastScriptFilename.clear();
		_lastResponseFilename.clear();
		errorMessage = checkError;
		return false;
	}

	std::string executableError;
	if (!executableLooksAvailable(_rExecutable, executableError)) {
		_lastStatus = "Failed";
		_lastExitCode = -1;
		_lastStdout.clear();
		_lastStderr = executableError;
		_lastResponsePayload.clear();
		_lastScriptFilename.clear();
		_lastResponseFilename.clear();
		errorMessage = executableError;
		return false;
	}

	std::error_code errorCode;
	const std::filesystem::path workingDir = resolveWorkingDirectory(_workingDirectory);
	if (!std::filesystem::exists(workingDir, errorCode) || !std::filesystem::is_directory(workingDir, errorCode)) {
		_lastStatus = "Failed";
		_lastExitCode = -1;
		_lastStdout.clear();
		_lastStderr = "Working directory \"" + workingDir.string() + "\" is not available.";
		_lastResponsePayload.clear();
		_lastScriptFilename.clear();
		_lastResponseFilename.clear();
		errorMessage = _lastStderr;
		return false;
	}

	const std::string stem = makeUniqueStem(getName());
	const std::filesystem::path scriptPath = workingDir / (stem + ".R");
	const std::filesystem::path stdoutPath = workingDir / (stem + ".stdout");
	const std::filesystem::path stderrPath = workingDir / (stem + ".stderr");

	{
		std::ofstream script(scriptPath);
		if (!script.is_open()) {
			_lastStatus = "Failed";
			_lastExitCode = -1;
			_lastStdout.clear();
			_lastStderr = "Could not create R script file \"" + scriptPath.string() + "\".";
			_lastResponsePayload.clear();
			_lastScriptFilename.clear();
			_lastResponseFilename.clear();
			errorMessage = _lastStderr;
			return false;
		}

		// Script assembly keeps the optional prelude separate from the user command block.
		if (!trim(_preludeScript).empty()) {
			script << _preludeScript << "\n";
		}
		script << _command << "\n";
	}

	const std::string execCommand =
			"cd " + shellQuote(workingDir.string()) + " && " +
			shellQuote(_rExecutable) + " --vanilla " + shellQuote(scriptPath.string()) +
			" >" + shellQuote(stdoutPath.string()) +
			" 2>" + shellQuote(stderrPath.string());

	// R execution is isolated to a generated script; stdout/stderr are captured in files and then persisted.
	const int exitCode = normalizeSystemExitCode(std::system(execCommand.c_str()));
	const std::string capturedStdout = readTextFile(stdoutPath);
	const std::string capturedStderr = readTextFile(stderrPath);
	const bool success = exitCode == 0;

	_lastStatus = success ? "Completed" : "Failed";
	_lastExitCode = exitCode;
	_lastStdout = capturedStdout;
	_lastStderr = capturedStderr;
	_lastResponsePayload = capturedStdout;
	_lastScriptFilename = scriptPath.string();
	_lastResponseFilename = stdoutPath.string();

	if (!success) {
		errorMessage = capturedStderr.empty()
				? "Rscript returned exit code " + std::to_string(exitCode) + "."
				: capturedStderr;
		return false;
	}
	return true;
}

void RSimulatorRunner::setRExecutable(std::string rExecutable) {
	_rExecutable = rExecutable;
}

std::string RSimulatorRunner::getRExecutable() const {
	return _rExecutable;
}

void RSimulatorRunner::setWorkingDirectory(std::string workingDirectory) {
	_workingDirectory = workingDirectory;
}

std::string RSimulatorRunner::getWorkingDirectory() const {
	return _workingDirectory;
}

void RSimulatorRunner::setPreludeScript(std::string preludeScript) {
	_preludeScript = preludeScript;
}

std::string RSimulatorRunner::getPreludeScript() const {
	return _preludeScript;
}

void RSimulatorRunner::setCommand(std::string command) {
	_command = command;
}

std::string RSimulatorRunner::getCommand() const {
	return _command;
}

void RSimulatorRunner::setLastStatus(std::string lastStatus) {
	_lastStatus = lastStatus;
}

std::string RSimulatorRunner::getLastStatus() const {
	return _lastStatus;
}

void RSimulatorRunner::setLastExitCode(int lastExitCode) {
	_lastExitCode = lastExitCode;
}

int RSimulatorRunner::getLastExitCode() const {
	return _lastExitCode;
}

void RSimulatorRunner::setLastStdout(std::string lastStdout) {
	_lastStdout = lastStdout;
}

std::string RSimulatorRunner::getLastStdout() const {
	return _lastStdout;
}

void RSimulatorRunner::setLastStderr(std::string lastStderr) {
	_lastStderr = lastStderr;
}

std::string RSimulatorRunner::getLastStderr() const {
	return _lastStderr;
}

void RSimulatorRunner::setLastResponsePayload(std::string lastResponsePayload) {
	_lastResponsePayload = lastResponsePayload;
}

std::string RSimulatorRunner::getLastResponsePayload() const {
	return _lastResponsePayload;
}

void RSimulatorRunner::setLastScriptFilename(std::string lastScriptFilename) {
	_lastScriptFilename = lastScriptFilename;
}

std::string RSimulatorRunner::getLastScriptFilename() const {
	return _lastScriptFilename;
}

void RSimulatorRunner::setLastResponseFilename(std::string lastResponseFilename) {
	_lastResponseFilename = lastResponseFilename;
}

std::string RSimulatorRunner::getLastResponseFilename() const {
	return _lastResponseFilename;
}
