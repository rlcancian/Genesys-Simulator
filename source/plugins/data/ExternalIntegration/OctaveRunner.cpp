/*
 * File:   OctaveRunner.cpp
 * Author: GenESyS
 *
 * Octave integration data definition.
 */

#include "plugins/data/ExternalIntegration/OctaveRunner.h"

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

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &OctaveRunner::GetPluginInformation;
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
		errorMessage = "Octave executable must not be empty.";
		return false;
	}
	if (hasPathSeparator(executable)) {
		if (!std::filesystem::exists(executable)) {
			errorMessage = "Octave executable \"" + executable + "\" was not found.";
			return false;
		}
		return true;
	}

	const std::string command = "command -v " + shellQuote(executable) + " >/dev/null 2>&1";
	const int exitCode = normalizeSystemExitCode(std::system(command.c_str()));
	if (exitCode != 0) {
		errorMessage = "Octave executable \"" + executable + "\" was not found in PATH.";
		return false;
	}
	return true;
}

} // namespace

ModelDataDefinition* OctaveRunner::NewInstance(Model* model, std::string name) {
	return new OctaveRunner(model, name);
}

OctaveRunner::OctaveRunner(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<OctaveRunner>(), name) {
	SimulationControlGeneric<std::string>* propOctaveExecutable = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getOctaveExecutable, this), std::bind(&OctaveRunner::setOctaveExecutable, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "OctaveExecutable", "");
	SimulationControlGeneric<std::string>* propWorkingDirectory = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getWorkingDirectory, this), std::bind(&OctaveRunner::setWorkingDirectory, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "WorkingDirectory", "");
	SimulationControlGeneric<std::string>* propPreludeScript = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getPreludeScript, this), std::bind(&OctaveRunner::setPreludeScript, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "PreludeScript", "");
	SimulationControlGeneric<std::string>* propCommand = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getCommand, this), std::bind(&OctaveRunner::setCommand, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "Command", "");
	SimulationControlGeneric<std::string>* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastStatus, this), std::bind(&OctaveRunner::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastStatus", "");
	SimulationControlGeneric<int>* propLastExitCode = new SimulationControlGeneric<int>(
			std::bind(&OctaveRunner::getLastExitCode, this), std::bind(&OctaveRunner::setLastExitCode, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastExitCode", "");
	SimulationControlGeneric<std::string>* propLastStdout = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastStdout, this), std::bind(&OctaveRunner::setLastStdout, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastStdout", "");
	SimulationControlGeneric<std::string>* propLastStderr = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastStderr, this), std::bind(&OctaveRunner::setLastStderr, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastStderr", "");
	SimulationControlGeneric<std::string>* propLastResponsePayload = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastResponsePayload, this), std::bind(&OctaveRunner::setLastResponsePayload, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastResponsePayload", "");
	SimulationControlGeneric<std::string>* propLastScriptFilename = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastScriptFilename, this), std::bind(&OctaveRunner::setLastScriptFilename, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastScriptFilename", "");
	SimulationControlGeneric<std::string>* propLastResponseFilename = new SimulationControlGeneric<std::string>(
			std::bind(&OctaveRunner::getLastResponseFilename, this), std::bind(&OctaveRunner::setLastResponseFilename, this, std::placeholders::_1),
			Util::TypeOf<OctaveRunner>(), getName(), "LastResponseFilename", "");

	_parentModel->getControls()->insert(propOctaveExecutable);
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

	_addSimulationControl(propOctaveExecutable);
	_addSimulationControl(propWorkingDirectory);
	_addSimulationControl(propPreludeScript);
	_addSimulationControl(propCommand);
	_addSimulationControl(propLastStatus);
	_addSimulationControl(propLastExitCode);
	_addSimulationControl(propLastStdout);
	_addSimulationControl(propLastStderr);
	_addSimulationControl(propLastResponsePayload);
	_addSimulationControl(propLastScriptFilename);
	_addSimulationControl(propLastResponseFilename);
}

PluginInformation* OctaveRunner::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<OctaveRunner>(), &OctaveRunner::LoadInstance, &OctaveRunner::NewInstance);
	info->setCategory("ExternalIntegration");
	info->setDescriptionHelp("Executes Octave code through Rscript. The runner writes the configured prelude and command into a temporary .Octave script, runs Rscript --vanilla, captures stdout/stderr and stores status, exit code and response payload in the model data definition.");
	info->setObservation("First Octave integration runner. It executes local Octave scripts through Rscript and does not require additional Octave packages.");
	info->insertSystemDependency(SystemDependency(
			SystemDependency::OS::Linux,
			"Octave",
			"sudo apt install -y octave",
			"octave --version"));
	return info;
}

ModelDataDefinition* OctaveRunner::LoadInstance(Model* model, PersistenceRecord *fields) {
	OctaveRunner* newElement = new OctaveRunner(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load OctaveRunner instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string OctaveRunner::show() {
	return ModelDataDefinition::show() +
			",octaveExecutable=\"" + _octaveExecutable + "\"" +
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

bool OctaveRunner::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		// Persistence mirrors _saveInstance so a runner can be re-executed after model reload.
		_octaveExecutable = fields->loadField("octaveExecutable", DEFAULT.octaveExecutable);
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

void OctaveRunner::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	// Persist both configuration and observability fields so the last Octave execution is reproducible/auditable.
	fields->saveField("octaveExecutable", _octaveExecutable, DEFAULT.octaveExecutable, saveDefaultValues);
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

bool OctaveRunner::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (trim(_octaveExecutable).empty()) {
		errorMessage += "OctaveRunner \"" + getName() + "\" must define a non-empty OctaveExecutable. ";
		resultAll = false;
	}
	if (trim(_command).empty()) {
		errorMessage += "OctaveRunner \"" + getName() + "\" must define a non-empty command. ";
		resultAll = false;
	}
	if (!trim(_workingDirectory).empty()) {
		std::error_code errorCode;
		const std::filesystem::path workingDir(_workingDirectory);
		if (!std::filesystem::exists(workingDir, errorCode) || !std::filesystem::is_directory(workingDir, errorCode)) {
			errorMessage += "OctaveRunner \"" + getName() + "\" workingDirectory must exist and be a directory. ";
			resultAll = false;
		}
	}
	return resultAll;
}

// void OctaveRunner::_createAttachedAttributes() {
// }

bool OctaveRunner::executeCommand(std::string& errorMessage) {
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
	if (!executableLooksAvailable(_octaveExecutable, executableError)) {
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
	const std::filesystem::path scriptPath = workingDir / (stem + ".Octave");
	const std::filesystem::path stdoutPath = workingDir / (stem + ".stdout");
	const std::filesystem::path stderrPath = workingDir / (stem + ".stderr");

	{
		std::ofstream script(scriptPath);
		if (!script.is_open()) {
			_lastStatus = "Failed";
			_lastExitCode = -1;
			_lastStdout.clear();
			_lastStderr = "Could not create Octave script file \"" + scriptPath.string() + "\".";
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
			shellQuote(_octaveExecutable) + " --vanilla " + shellQuote(scriptPath.string()) +
			" >" + shellQuote(stdoutPath.string()) +
			" 2>" + shellQuote(stderrPath.string());

	// Octave execution is isolated to a generated script; stdout/stderr are captured in files and then persisted.
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

void OctaveRunner::setOctaveExecutable(std::string octaveExecutable) {
	_octaveExecutable = octaveExecutable;
}

std::string OctaveRunner::getOctaveExecutable() const {
	return _octaveExecutable;
}

void OctaveRunner::setWorkingDirectory(std::string workingDirectory) {
	_workingDirectory = workingDirectory;
}

std::string OctaveRunner::getWorkingDirectory() const {
	return _workingDirectory;
}

void OctaveRunner::setPreludeScript(std::string preludeScript) {
	_preludeScript = preludeScript;
}

std::string OctaveRunner::getPreludeScript() const {
	return _preludeScript;
}

void OctaveRunner::setCommand(std::string command) {
	_command = command;
}

std::string OctaveRunner::getCommand() const {
	return _command;
}

void OctaveRunner::setLastStatus(std::string lastStatus) {
	_lastStatus = lastStatus;
}

std::string OctaveRunner::getLastStatus() const {
	return _lastStatus;
}

void OctaveRunner::setLastExitCode(int lastExitCode) {
	_lastExitCode = lastExitCode;
}

int OctaveRunner::getLastExitCode() const {
	return _lastExitCode;
}

void OctaveRunner::setLastStdout(std::string lastStdout) {
	_lastStdout = lastStdout;
}

std::string OctaveRunner::getLastStdout() const {
	return _lastStdout;
}

void OctaveRunner::setLastStderr(std::string lastStderr) {
	_lastStderr = lastStderr;
}

std::string OctaveRunner::getLastStderr() const {
	return _lastStderr;
}

void OctaveRunner::setLastResponsePayload(std::string lastResponsePayload) {
	_lastResponsePayload = lastResponsePayload;
}

std::string OctaveRunner::getLastResponsePayload() const {
	return _lastResponsePayload;
}

void OctaveRunner::setLastScriptFilename(std::string lastScriptFilename) {
	_lastScriptFilename = lastScriptFilename;
}

std::string OctaveRunner::getLastScriptFilename() const {
	return _lastScriptFilename;
}

void OctaveRunner::setLastResponseFilename(std::string lastResponseFilename) {
	_lastResponseFilename = lastResponseFilename;
}

std::string OctaveRunner::getLastResponseFilename() const {
	return _lastResponseFilename;
}

// void OctaveRunner::_createInternalStatisticReporters() { }

// void OctaveRunner::_createEditableDataDefinitions() { }

// void OctaveRunner::_createAttachedAttributes() { }
