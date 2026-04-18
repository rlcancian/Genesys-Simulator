/*
 * File:   BioSimulatorRunner.cpp
 * Author: GenESyS
 *
 * Structural biochemical simulator runner data definition.
 */

#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioSimulatorRunner::GetPluginInformation;
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

bool startsWith(const std::string& text, const std::string& prefix) {
	return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

std::string jsonEscape(const std::string& value) {
	std::string escaped;
	for (char ch : value) {
		switch (ch) {
			case '\\':
				escaped += "\\\\";
				break;
			case '"':
				escaped += "\\\"";
				break;
			case '\n':
				escaped += "\\n";
				break;
			case '\r':
				escaped += "\\r";
				break;
			case '\t':
				escaped += "\\t";
				break;
			default:
				escaped += ch;
				break;
		}
	}
	return escaped;
}

std::string formatDouble(double value) {
	std::ostringstream out;
	out << std::setprecision(15) << value;
	return out.str();
}

bool parseDouble(const std::string& text, double* value) {
	try {
		const std::string normalized = trim(text);
		size_t consumed = 0;
		const double parsed = std::stod(normalized, &consumed);
		if (consumed != normalized.size()) {
			return false;
		}
		*value = parsed;
		return true;
	} catch (...) {
		return false;
	}
}

bool parseUnsigned(const std::string& text, unsigned int* value) {
	try {
		const std::string normalized = trim(text);
		if (normalized.empty() || normalized.front() == '-' || normalized.front() == '+') {
			return false;
		}
		size_t consumed = 0;
		const unsigned long parsed = std::stoul(normalized, &consumed);
		if (consumed != normalized.size() || parsed == 0 || parsed > std::numeric_limits<unsigned int>::max()) {
			return false;
		}
		*value = static_cast<unsigned int>(parsed);
		return true;
	} catch (...) {
		return false;
	}
}

std::vector<std::string> splitArgs(const std::string& args) {
	std::vector<std::string> result;
	std::string token;
	bool insideQuotes = false;
	for (char ch : args) {
		if (ch == '"') {
			insideQuotes = !insideQuotes;
			token += ch;
			continue;
		}
		if (ch == ',' && !insideQuotes) {
			result.push_back(trim(token));
			token.clear();
			continue;
		}
		token += ch;
	}
	result.push_back(trim(token));
	return result;
}

bool parseCall(const std::string& command, const std::string& name, std::string* args, std::string& errorMessage) {
	const std::string normalized = trim(command);
	const std::string prefix = name + "(";
	if (!startsWith(normalized, prefix)) {
		return false;
	}
	if (normalized.empty() || normalized.back() != ')') {
		errorMessage = "Command \"" + normalized + "\" has malformed parentheses.";
		return false;
	}
	if (normalized.find(')', prefix.size()) != normalized.size() - 1) {
		errorMessage = "Command \"" + normalized + "\" has malformed parentheses.";
		return false;
	}
	*args = normalized.substr(prefix.size(), normalized.size() - prefix.size() - 1);
	return true;
}

bool parseQuotedSymbol(const std::string& text, std::string* symbol) {
	const std::string normalized = trim(text);
	if (normalized.size() < 2 || normalized.front() != '"' || normalized.back() != '"') {
		return false;
	}
	*symbol = normalized.substr(1, normalized.size() - 2);
	return !symbol->empty() && symbol->find('"') == std::string::npos;
}

std::string makePayloadPrefix(bool success, const std::string& status, const std::string& backend, const std::string& command, const std::string& resultType) {
	return "{\"success\":" + std::string(success ? "true" : "false") +
			",\"status\":\"" + jsonEscape(status) + "\"" +
			",\"backend\":\"" + jsonEscape(backend) + "\"" +
			",\"command\":\"" + jsonEscape(command) + "\"" +
			",\"resultType\":\"" + jsonEscape(resultType) + "\"";
}

} // namespace

ModelDataDefinition* BioSimulatorRunner::NewInstance(Model* model, std::string name) {
	return new BioSimulatorRunner(model, name);
}

BioSimulatorRunner::BioSimulatorRunner(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<BioSimulatorRunner>(), name) {
	SimulationControlGeneric<std::string>* propBackend = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getBackend, this), std::bind(&BioSimulatorRunner::setBackend, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "Backend", "");
	SimulationControlGeneric<std::string>* propModelSourceType = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getModelSourceType, this), std::bind(&BioSimulatorRunner::setModelSourceType, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "ModelSourceType", "");
	SimulationControlGeneric<std::string>* propModelSource = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getModelSource, this), std::bind(&BioSimulatorRunner::setModelSource, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "ModelSource", "");
	SimulationControlGeneric<std::string>* propCommand = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getCommand, this), std::bind(&BioSimulatorRunner::setCommand, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "Command", "");
	SimulationControlGeneric<std::string>* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getLastStatus, this), std::bind(&BioSimulatorRunner::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "LastStatus", "");
	SimulationControlGeneric<std::string>* propLastErrorMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getLastErrorMessage, this), std::bind(&BioSimulatorRunner::setLastErrorMessage, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "LastErrorMessage", "");
	SimulationControlGeneric<std::string>* propLastResponsePayload = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getLastResponsePayload, this), std::bind(&BioSimulatorRunner::setLastResponsePayload, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "LastResponsePayload", "");
	SimulationControlGeneric<std::string>* propLastResponseFilename = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getLastResponseFilename, this), std::bind(&BioSimulatorRunner::setLastResponseFilename, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "LastResponseFilename", "");
	SimulationControlGeneric<std::string>* propWorkingDirectory = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getWorkingDirectory, this), std::bind(&BioSimulatorRunner::setWorkingDirectory, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "WorkingDirectory", "");
	SimulationControlGeneric<std::string>* propWorkingInputFilename = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getWorkingInputFilename, this), std::bind(&BioSimulatorRunner::setWorkingInputFilename, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "WorkingInputFilename", "");
	SimulationControlGeneric<std::string>* propWorkingOutputFilename = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getWorkingOutputFilename, this), std::bind(&BioSimulatorRunner::setWorkingOutputFilename, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "WorkingOutputFilename", "");
	SimulationControlGeneric<std::string>* propEndpointOrLibrary = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getEndpointOrLibrary, this), std::bind(&BioSimulatorRunner::setEndpointOrLibrary, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "EndpointOrLibrary", "");
	SimulationControlGeneric<unsigned int>* propTimeoutSeconds = new SimulationControlGeneric<unsigned int>(
			std::bind(&BioSimulatorRunner::getTimeoutSeconds, this), std::bind(&BioSimulatorRunner::setTimeoutSeconds, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "TimeoutSeconds", "");
	SimulationControlGeneric<bool>* propAutoValidateModel = new SimulationControlGeneric<bool>(
			std::bind(&BioSimulatorRunner::getAutoValidateModel, this), std::bind(&BioSimulatorRunner::setAutoValidateModel, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "AutoValidateModel", "");

	_parentModel->getControls()->insert(propBackend);
	_parentModel->getControls()->insert(propModelSourceType);
	_parentModel->getControls()->insert(propModelSource);
	_parentModel->getControls()->insert(propCommand);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastErrorMessage);
	_parentModel->getControls()->insert(propLastResponsePayload);
	_parentModel->getControls()->insert(propLastResponseFilename);
	_parentModel->getControls()->insert(propWorkingDirectory);
	_parentModel->getControls()->insert(propWorkingInputFilename);
	_parentModel->getControls()->insert(propWorkingOutputFilename);
	_parentModel->getControls()->insert(propEndpointOrLibrary);
	_parentModel->getControls()->insert(propTimeoutSeconds);
	_parentModel->getControls()->insert(propAutoValidateModel);

	_addProperty(propBackend);
	_addProperty(propModelSourceType);
	_addProperty(propModelSource);
	_addProperty(propCommand);
	_addProperty(propLastStatus);
	_addProperty(propLastErrorMessage);
	_addProperty(propLastResponsePayload);
	_addProperty(propLastResponseFilename);
	_addProperty(propWorkingDirectory);
	_addProperty(propWorkingInputFilename);
	_addProperty(propWorkingOutputFilename);
	_addProperty(propEndpointOrLibrary);
	_addProperty(propTimeoutSeconds);
	_addProperty(propAutoValidateModel);
}

PluginInformation* BioSimulatorRunner::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioSimulatorRunner>(), &BioSimulatorRunner::LoadInstance, &BioSimulatorRunner::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Structural biochemical simulator runner. This phase persists configuration and executes deterministic local stub commands without integrating a real biochemical backend.");
	info->insertSystemDependency(SystemDependency(
			SystemDependency::OS::Linux,
			"libSBML",
			"sudo apt install libsbml5-dev -y",
			"pkg-config --exists libsbml"));
	return info;
}

ModelDataDefinition* BioSimulatorRunner::LoadInstance(Model* model, PersistenceRecord *fields) {
	BioSimulatorRunner* newElement = new BioSimulatorRunner(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string BioSimulatorRunner::show() {
	return ModelDataDefinition::show() +
			",backend=\"" + _backend + "\"" +
			",modelSourceType=\"" + _modelSourceType + "\"" +
			",command=\"" + _command + "\"" +
			",lastStatus=\"" + _lastStatus + "\"" +
			",workingDirectory=\"" + _workingDirectory + "\"" +
			",workingInputFilename=\"" + _workingInputFilename + "\"" +
			",workingOutputFilename=\"" + _workingOutputFilename + "\"" +
			",endpointOrLibrary=\"" + _endpointOrLibrary + "\"" +
			",timeoutSeconds=" + std::to_string(_timeoutSeconds) +
			",lastResponsePayloadSize=" + std::to_string(_lastResponsePayload.size());
}

bool BioSimulatorRunner::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_backend = fields->loadField("backend", DEFAULT.backend);
		_modelSourceType = fields->loadField("modelSourceType", DEFAULT.modelSourceType);
		_modelSource = fields->loadField("modelSource", DEFAULT.modelSource);
		_command = fields->loadField("command", DEFAULT.command);
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastErrorMessage = fields->loadField("lastErrorMessage", DEFAULT.lastErrorMessage);
		_lastResponsePayload = fields->loadField("lastResponsePayload", DEFAULT.lastResponsePayload);
		_lastResponseFilename = fields->loadField("lastResponseFilename", DEFAULT.lastResponseFilename);
		_workingDirectory = fields->loadField("workingDirectory", DEFAULT.workingDirectory);
		_workingInputFilename = fields->loadField("workingInputFilename", DEFAULT.workingInputFilename);
		_workingOutputFilename = fields->loadField("workingOutputFilename", DEFAULT.workingOutputFilename);
		_endpointOrLibrary = fields->loadField("endpointOrLibrary", DEFAULT.endpointOrLibrary);
		_timeoutSeconds = fields->loadField("timeoutSeconds", DEFAULT.timeoutSeconds);
		_autoValidateModel = fields->loadField("autoValidateModel", DEFAULT.autoValidateModel ? 1u : 0u) != 0u;
	}
	return res;
}

void BioSimulatorRunner::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("backend", _backend, DEFAULT.backend, saveDefaultValues);
	fields->saveField("modelSourceType", _modelSourceType, DEFAULT.modelSourceType, saveDefaultValues);
	fields->saveField("modelSource", _modelSource, DEFAULT.modelSource, saveDefaultValues);
	fields->saveField("command", _command, DEFAULT.command, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastErrorMessage", _lastErrorMessage, DEFAULT.lastErrorMessage, saveDefaultValues);
	fields->saveField("lastResponsePayload", _lastResponsePayload, DEFAULT.lastResponsePayload, saveDefaultValues);
	fields->saveField("lastResponseFilename", _lastResponseFilename, DEFAULT.lastResponseFilename, saveDefaultValues);
	fields->saveField("workingDirectory", _workingDirectory, DEFAULT.workingDirectory, saveDefaultValues);
	fields->saveField("workingInputFilename", _workingInputFilename, DEFAULT.workingInputFilename, saveDefaultValues);
	fields->saveField("workingOutputFilename", _workingOutputFilename, DEFAULT.workingOutputFilename, saveDefaultValues);
	fields->saveField("endpointOrLibrary", _endpointOrLibrary, DEFAULT.endpointOrLibrary, saveDefaultValues);
	fields->saveField("timeoutSeconds", _timeoutSeconds, DEFAULT.timeoutSeconds, saveDefaultValues);
	fields->saveField("autoValidateModel", _autoValidateModel ? 1u : 0u, DEFAULT.autoValidateModel ? 1u : 0u, saveDefaultValues);
}

bool BioSimulatorRunner::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_backend.empty()) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define a non-empty backend. ";
		resultAll = false;
	}
	if (_modelSourceType.empty()) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define a non-empty modelSourceType. ";
		resultAll = false;
	} else if (_modelSourceType != "SBMLString" && _modelSourceType != "SBMLFile") {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" has unsupported modelSourceType \"" + _modelSourceType + "\". ";
		resultAll = false;
	}
	if (_timeoutSeconds == 0) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define timeoutSeconds greater than zero. ";
		resultAll = false;
	}
	if (_workingInputFilename.empty()) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define a non-empty workingInputFilename. ";
		resultAll = false;
	}
	if (_workingOutputFilename.empty()) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define a non-empty workingOutputFilename. ";
		resultAll = false;
	}
	if (_modelSourceType == "SBMLFile" && _modelSource.empty()) {
		errorMessage += "BioSimulatorRunner \"" + getName() + "\" must define modelSource when modelSourceType is SBMLFile. ";
		resultAll = false;
	}
	if (!_command.empty()) {
		const std::string normalized = trim(_command);
		if (normalized.find('(') == std::string::npos || normalized.find(')') == std::string::npos) {
			errorMessage += "BioSimulatorRunner \"" + getName() + "\" command must use function-call syntax. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void BioSimulatorRunner::_createInternalAndAttachedData() {
}

bool BioSimulatorRunner::executeCommand(std::string& errorMessage) {
	errorMessage.clear();
	_lastErrorMessage.clear();
	_lastResponsePayload.clear();
	_lastResponseFilename.clear();

	std::string checkError;
	if (!_check(checkError)) {
		_lastStatus = "Failed";
		_lastErrorMessage = checkError;
		errorMessage = checkError;
		return false;
	}

	const std::string normalizedCommand = trim(_command);
	if (normalizedCommand.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = "BioSimulatorRunner command must not be empty.";
		errorMessage = _lastErrorMessage;
		return false;
	}

	std::string args;
	if (parseCall(normalizedCommand, "validateModel", &args, errorMessage)) {
		if (!trim(args).empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "validateModel() does not accept parameters.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		if (_modelSource.empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "validateModel() requires a non-empty modelSource.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "stub_validation") +
				",\"modelSourceType\":\"" + jsonEscape(_modelSourceType) + "\"}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "simulate", &args, errorMessage)) {
		const std::vector<std::string> parsedArgs = splitArgs(args);
		double start = 0.0;
		double stop = 0.0;
		unsigned int steps = 0;
		if (parsedArgs.size() != 3 || !parseDouble(parsedArgs[0], &start) || !parseDouble(parsedArgs[1], &stop) || !parseUnsigned(parsedArgs[2], &steps)) {
			_lastStatus = "Failed";
			_lastErrorMessage = "simulate(start, stop, steps) requires numeric start/stop and positive integer steps.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		if (stop < start) {
			_lastStatus = "Failed";
			_lastErrorMessage = "simulate(start, stop, steps) requires stop >= start.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponseFilename = _workingOutputFilename;
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "stub_time_course") +
				",\"start\":" + formatDouble(start) +
				",\"stop\":" + formatDouble(stop) +
				",\"steps\":" + std::to_string(steps) + "}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "steadyState", &args, errorMessage)) {
		if (!trim(args).empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "steadyState() does not accept parameters.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "stub_steady_state") +
				",\"converged\":true}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "getValue", &args, errorMessage)) {
		std::string symbol;
		if (!parseQuotedSymbol(args, &symbol)) {
			_lastStatus = "Failed";
			_lastErrorMessage = "getValue(\"symbol\") requires one non-empty quoted symbol.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "stub_value") +
				",\"symbol\":\"" + jsonEscape(symbol) + "\"" +
				",\"value\":0.0}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "setValue", &args, errorMessage)) {
		const std::vector<std::string> parsedArgs = splitArgs(args);
		std::string symbol;
		double value = 0.0;
		if (parsedArgs.size() != 2 || !parseQuotedSymbol(parsedArgs[0], &symbol) || !parseDouble(parsedArgs[1], &value)) {
			_lastStatus = "Failed";
			_lastErrorMessage = "setValue(\"symbol\", value) requires one quoted symbol and one numeric value.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "stub_set_value") +
				",\"symbol\":\"" + jsonEscape(symbol) + "\"" +
				",\"value\":" + formatDouble(value) +
				",\"updated\":true}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "reset", &args, errorMessage)) {
		if (!trim(args).empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "reset() does not accept parameters.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Idle";
		_lastErrorMessage.clear();
		_lastResponsePayload.clear();
		_lastResponseFilename.clear();
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	_lastStatus = "Failed";
	_lastErrorMessage = "Unknown BioSimulatorRunner command \"" + normalizedCommand + "\".";
	errorMessage = _lastErrorMessage;
	return false;
}

void BioSimulatorRunner::setBackend(std::string backend) {
	_backend = backend;
}

std::string BioSimulatorRunner::getBackend() const {
	return _backend;
}

void BioSimulatorRunner::setModelSourceType(std::string modelSourceType) {
	_modelSourceType = modelSourceType;
}

std::string BioSimulatorRunner::getModelSourceType() const {
	return _modelSourceType;
}

void BioSimulatorRunner::setModelSource(std::string modelSource) {
	_modelSource = modelSource;
}

std::string BioSimulatorRunner::getModelSource() const {
	return _modelSource;
}

void BioSimulatorRunner::setCommand(std::string command) {
	_command = command;
}

std::string BioSimulatorRunner::getCommand() const {
	return _command;
}

void BioSimulatorRunner::setLastStatus(std::string lastStatus) {
	_lastStatus = lastStatus;
}

std::string BioSimulatorRunner::getLastStatus() const {
	return _lastStatus;
}

void BioSimulatorRunner::setLastErrorMessage(std::string lastErrorMessage) {
	_lastErrorMessage = lastErrorMessage;
}

std::string BioSimulatorRunner::getLastErrorMessage() const {
	return _lastErrorMessage;
}

void BioSimulatorRunner::setLastResponsePayload(std::string lastResponsePayload) {
	_lastResponsePayload = lastResponsePayload;
}

std::string BioSimulatorRunner::getLastResponsePayload() const {
	return _lastResponsePayload;
}

void BioSimulatorRunner::setLastResponseFilename(std::string lastResponseFilename) {
	_lastResponseFilename = lastResponseFilename;
}

std::string BioSimulatorRunner::getLastResponseFilename() const {
	return _lastResponseFilename;
}

void BioSimulatorRunner::setWorkingDirectory(std::string workingDirectory) {
	_workingDirectory = workingDirectory;
}

std::string BioSimulatorRunner::getWorkingDirectory() const {
	return _workingDirectory;
}

void BioSimulatorRunner::setWorkingInputFilename(std::string workingInputFilename) {
	_workingInputFilename = workingInputFilename;
}

std::string BioSimulatorRunner::getWorkingInputFilename() const {
	return _workingInputFilename;
}

void BioSimulatorRunner::setWorkingOutputFilename(std::string workingOutputFilename) {
	_workingOutputFilename = workingOutputFilename;
}

std::string BioSimulatorRunner::getWorkingOutputFilename() const {
	return _workingOutputFilename;
}

void BioSimulatorRunner::setEndpointOrLibrary(std::string endpointOrLibrary) {
	_endpointOrLibrary = endpointOrLibrary;
}

std::string BioSimulatorRunner::getEndpointOrLibrary() const {
	return _endpointOrLibrary;
}

void BioSimulatorRunner::setTimeoutSeconds(unsigned int timeoutSeconds) {
	_timeoutSeconds = timeoutSeconds;
}

unsigned int BioSimulatorRunner::getTimeoutSeconds() const {
	return _timeoutSeconds;
}

void BioSimulatorRunner::setAutoValidateModel(bool autoValidateModel) {
	_autoValidateModel = autoValidateModel;
}

bool BioSimulatorRunner::getAutoValidateModel() const {
	return _autoValidateModel;
}
