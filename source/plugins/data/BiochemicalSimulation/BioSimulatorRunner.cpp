/*
 * File:   BioSimulatorRunner.cpp
 * Author: GenESyS
 *
 * Structural biochemical simulator runner data definition.
 */

#include "plugins/data/BiochemicalSimulation/BioSimulatorRunner.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include "kernel/TraitsKernel.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/util/List.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioSBMLBridge.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

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

std::string jsonArray(const std::vector<std::string>& values) {
	std::string json = "[";
	for (const std::string& value : values) {
		json += "\"" + jsonEscape(value) + "\",";
	}
	if (!values.empty()) {
		json.pop_back();
	}
	json += "]";
	return json;
}

std::filesystem::path resolveSourcePath(const std::string& configuredPath, const std::string& workingDirectory) {
	const std::filesystem::path sourcePath(configuredPath);
	if (sourcePath.is_absolute() || trim(workingDirectory).empty()) {
		return sourcePath;
	}
	return std::filesystem::path(workingDirectory) / sourcePath;
}

bool readTextFile(const std::filesystem::path& path, std::string* contents, std::string& errorMessage) {
	if (contents == nullptr) {
		errorMessage = "Internal error: invalid output buffer while reading SBML file.";
		return false;
	}
	std::ifstream input(path);
	if (!input.is_open()) {
		errorMessage = "Could not open SBML input file \"" + path.string() + "\".";
		return false;
	}
	std::ostringstream text;
	text << input.rdbuf();
	*contents = text.str();
	return true;
}

bool writeTextFile(const std::filesystem::path& path, const std::string& contents, std::string& errorMessage) {
	std::ofstream output(path, std::ios::trunc);
	if (!output.is_open()) {
		errorMessage = "Could not write SBML output file \"" + path.string() + "\".";
		return false;
	}
	output << contents;
	if (!output.good()) {
		errorMessage = "Failed while writing SBML output file \"" + path.string() + "\".";
		return false;
	}
	return true;
}

BioNetwork* resolveBioNetwork(Model* model, const std::string& targetBioNetworkName, std::string& errorMessage) {
	if (model == nullptr || model->getDataManager() == nullptr) {
		errorMessage = "BioSimulatorRunner requires a valid model to resolve BioNetwork definitions.";
		return nullptr;
	}

	const std::string requestedName = trim(targetBioNetworkName);
	if (!requestedName.empty()) {
		ModelDataDefinition* definition = model->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), requestedName);
		auto* network = dynamic_cast<BioNetwork*>(definition);
		if (network == nullptr) {
			errorMessage = "BioSimulatorRunner could not resolve BioNetwork \"" + requestedName + "\".";
		}
		return network;
	}

	List<ModelDataDefinition*>* definitions = model->getDataManager()->getDataDefinitionList(Util::TypeOf<BioNetwork>());
	if (definitions == nullptr || definitions->size() == 0) {
		errorMessage = "BioSimulatorRunner requires at least one BioNetwork or an explicit targetBioNetworkName.";
		return nullptr;
	}

	// Prefer the unique BioNetwork when the model has only one; otherwise require an explicit target name.
	BioNetwork* uniqueNetwork = nullptr;
	for (ModelDataDefinition* definition : *definitions->list()) {
		auto* network = dynamic_cast<BioNetwork*>(definition);
		if (network == nullptr) {
			continue;
		}
		if (uniqueNetwork != nullptr) {
			errorMessage = "BioSimulatorRunner found multiple BioNetwork definitions; set targetBioNetworkName to select one.";
			return nullptr;
		}
		uniqueNetwork = network;
	}
	if (uniqueNetwork == nullptr) {
		errorMessage = "BioSimulatorRunner could not resolve any BioNetwork definitions.";
		return nullptr;
	}
	return uniqueNetwork;
}

ModelDataDefinition* resolveSymbolDefinition(Model* model, const std::string& symbol, std::string* definitionType) {
	if (definitionType != nullptr) {
		definitionType->clear();
	}
	if (model == nullptr || model->getDataManager() == nullptr) {
		return nullptr;
	}

	ModelDataDefinition* definition = model->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), symbol);
	if (definition != nullptr) {
		if (definitionType != nullptr) {
			*definitionType = Util::TypeOf<BioSpecies>();
		}
		return definition;
	}

	definition = model->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), symbol);
	if (definition != nullptr && definitionType != nullptr) {
		*definitionType = Util::TypeOf<BioParameter>();
	}
	return definition;
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
	SimulationControlGeneric<std::string>* propTargetBioNetworkName = new SimulationControlGeneric<std::string>(
			std::bind(&BioSimulatorRunner::getTargetBioNetworkName, this), std::bind(&BioSimulatorRunner::setTargetBioNetworkName, this, std::placeholders::_1),
			Util::TypeOf<BioSimulatorRunner>(), getName(), "TargetBioNetworkName", "");
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
	_parentModel->getControls()->insert(propTargetBioNetworkName);
	_parentModel->getControls()->insert(propWorkingDirectory);
	_parentModel->getControls()->insert(propWorkingInputFilename);
	_parentModel->getControls()->insert(propWorkingOutputFilename);
	_parentModel->getControls()->insert(propEndpointOrLibrary);
	_parentModel->getControls()->insert(propTimeoutSeconds);
	_parentModel->getControls()->insert(propAutoValidateModel);

	_addSimulationControl(propBackend);
	_addSimulationControl(propModelSourceType);
	_addSimulationControl(propModelSource);
	_addSimulationControl(propCommand);
	_addSimulationControl(propLastStatus);
	_addSimulationControl(propLastErrorMessage);
	_addSimulationControl(propLastResponsePayload);
	_addSimulationControl(propLastResponseFilename);
	_addSimulationControl(propTargetBioNetworkName);
	_addSimulationControl(propWorkingDirectory);
	_addSimulationControl(propWorkingInputFilename);
	_addSimulationControl(propWorkingOutputFilename);
	_addSimulationControl(propEndpointOrLibrary);
	_addSimulationControl(propTimeoutSeconds);
	_addSimulationControl(propAutoValidateModel);
}

PluginInformation* BioSimulatorRunner::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioSimulatorRunner>(), &BioSimulatorRunner::LoadInstance, &BioSimulatorRunner::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Structural biochemical simulator runner. This phase persists configuration and can validate, simulate, query and update a target BioNetwork using the native biochemical backend.");
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
			",targetBioNetworkName=\"" + _targetBioNetworkName + "\"" +
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
		_targetBioNetworkName = fields->loadField("targetBioNetworkName", DEFAULT.targetBioNetworkName);
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
	fields->saveField("targetBioNetworkName", _targetBioNetworkName, DEFAULT.targetBioNetworkName, saveDefaultValues);
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
	if (!_targetBioNetworkName.empty()) {
		ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), _targetBioNetworkName);
		if (dynamic_cast<BioNetwork*>(definition) == nullptr) {
			errorMessage += "BioSimulatorRunner \"" + getName() + "\" references missing BioNetwork \"" + _targetBioNetworkName + "\". ";
			resultAll = false;
		}
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

// void BioSimulatorRunner::_createAttachedAttributes() {
// }

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
		std::string networkResolutionError;
		BioNetwork* network = resolveBioNetwork(_parentModel, _targetBioNetworkName, networkResolutionError);
		if (network != nullptr) {
			std::string networkCheckError;
			if (!_parentModel->getDataManager()->check(Util::TypeOf<BioNetwork>(), network, "BioNetwork", networkCheckError)) {
				_lastStatus = "Failed";
				_lastErrorMessage = networkCheckError;
				errorMessage = _lastErrorMessage;
				return false;
			}
			_targetBioNetworkName = network->getName();
			_lastStatus = "Completed";
			_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "network_validation") +
					",\"targetBioNetworkName\":\"" + jsonEscape(_targetBioNetworkName) + "\"" +
					",\"networkName\":\"" + jsonEscape(network->getName()) + "\"" +
					",\"speciesCount\":" + std::to_string(network->getSpeciesNames().size()) +
					",\"reactionCount\":" + std::to_string(network->getReactionNames().size()) +
					",\"modelSourceType\":\"" + jsonEscape(_modelSourceType) + "\"}";
			return true;
		}

		if (!networkResolutionError.empty() && networkResolutionError.find("at least one BioNetwork") == std::string::npos) {
			_lastStatus = "Failed";
			_lastErrorMessage = networkResolutionError;
			errorMessage = _lastErrorMessage;
			return false;
		}
		if (_modelSource.empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "validateModel() requires either a BioNetwork or a non-empty modelSource.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "source_validation") +
				",\"modelSourceType\":\"" + jsonEscape(_modelSourceType) + "\"" +
				",\"modelSourceLength\":" + std::to_string(_modelSource.size()) + "}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "importSBML", &args, errorMessage)) {
		const std::string trimmedArgs = trim(args);
		std::string requestedNetworkName;
		if (!trimmedArgs.empty() && !parseQuotedSymbol(trimmedArgs, &requestedNetworkName)) {
			_lastStatus = "Failed";
			_lastErrorMessage = "importSBML() accepts zero arguments or one quoted BioNetwork name.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		std::string sbmlDocument;
		if (_modelSourceType == "SBMLFile") {
			const std::filesystem::path sourcePath = resolveSourcePath(_modelSource, _workingDirectory);
			if (!readTextFile(sourcePath, &sbmlDocument, errorMessage)) {
				_lastStatus = "Failed";
				_lastErrorMessage = errorMessage;
				return false;
			}
			_lastResponseFilename = sourcePath.string();
		} else {
			sbmlDocument = _modelSource;
		}
		if (trim(sbmlDocument).empty()) {
			_lastStatus = "Failed";
			_lastErrorMessage = "importSBML() requires a non-empty SBML source.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		BioSBMLImportResult importResult;
		if (!BioSBMLBridge::importFromString(_parentModel, sbmlDocument, requestedNetworkName, &importResult, errorMessage)) {
			_lastStatus = "Failed";
			_lastErrorMessage = errorMessage;
			return false;
		}
		// Remember the imported network so later simulate/get/set commands have an explicit target.
		_targetBioNetworkName = importResult.networkName;
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "sbml_import") +
				",\"networkName\":\"" + jsonEscape(importResult.networkName) + "\"" +
				",\"targetBioNetworkName\":\"" + jsonEscape(_targetBioNetworkName) + "\"" +
				",\"speciesImported\":" + std::to_string(importResult.speciesImported) +
				",\"parametersImported\":" + std::to_string(importResult.parametersImported) +
				",\"reactionsImported\":" + std::to_string(importResult.reactionsImported) +
				",\"warnings\":" + jsonArray(importResult.warnings) + "}";
		return true;
	}
	if (!errorMessage.empty()) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	if (parseCall(normalizedCommand, "exportSBML", &args, errorMessage)) {
		const std::string trimmedArgs = trim(args);
		std::string requestedNetworkName;
		if (!trimmedArgs.empty() && !parseQuotedSymbol(trimmedArgs, &requestedNetworkName)) {
			_lastStatus = "Failed";
			_lastErrorMessage = "exportSBML() accepts zero arguments or one quoted BioNetwork name.";
			errorMessage = _lastErrorMessage;
			return false;
		}
		if (requestedNetworkName.empty()) {
			requestedNetworkName = _targetBioNetworkName;
		}

		BioSBMLExportResult exportResult;
		std::string sbmlDocument;
		if (!BioSBMLBridge::exportToString(_parentModel, requestedNetworkName, &sbmlDocument, &exportResult, errorMessage)) {
			_lastStatus = "Failed";
			_lastErrorMessage = errorMessage;
			return false;
		}

		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "sbml_export") +
				",\"networkName\":\"" + jsonEscape(exportResult.networkName) + "\"" +
				",\"speciesExported\":" + std::to_string(exportResult.speciesExported) +
				",\"parametersExported\":" + std::to_string(exportResult.parametersExported) +
				",\"reactionsExported\":" + std::to_string(exportResult.reactionsExported) +
				",\"warnings\":" + jsonArray(exportResult.warnings);

		if (_modelSourceType == "SBMLFile") {
			const std::filesystem::path outputPath = resolveSourcePath(_modelSource, _workingDirectory);
			if (!writeTextFile(outputPath, sbmlDocument, errorMessage)) {
				_lastStatus = "Failed";
				_lastErrorMessage = errorMessage;
				return false;
			}
			_lastResponseFilename = outputPath.string();
			_lastResponsePayload += ",\"outputFile\":\"" + jsonEscape(outputPath.string()) + "\"}";
		} else {
			_modelSource = sbmlDocument;
			_lastResponsePayload += ",\"sbml\":\"" + jsonEscape(sbmlDocument) + "\"}";
		}
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
		if (stop <= start) {
			_lastStatus = "Failed";
			_lastErrorMessage = "simulate(start, stop, steps) requires stop > start.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		std::string networkResolutionError;
		BioNetwork* network = resolveBioNetwork(_parentModel, _targetBioNetworkName, networkResolutionError);
		if (network == nullptr) {
			_lastStatus = "Failed";
			_lastErrorMessage = networkResolutionError;
			errorMessage = _lastErrorMessage;
			return false;
		}

		const double stepSize = (stop - start) / static_cast<double>(steps);
		if (stepSize <= 0.0) {
			_lastStatus = "Failed";
			_lastErrorMessage = "simulate(start, stop, steps) produced a non-positive step size.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		network->setStartTime(start);
		network->setStopTime(stop);
		network->setStepSize(stepSize);
		_targetBioNetworkName = network->getName();

		if (!network->simulate(start, stop, stepSize, errorMessage)) {
			_lastStatus = "Failed";
			_lastErrorMessage = errorMessage;
			return false;
		}

		const BioSimulationResult& result = network->getLastSimulationResult();
		_lastStatus = "Completed";
		_lastResponseFilename = _workingOutputFilename;
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "bio_time_course") +
				",\"targetBioNetworkName\":\"" + jsonEscape(_targetBioNetworkName) + "\"" +
				",\"start\":" + formatDouble(start) +
				",\"stop\":" + formatDouble(stop) +
				",\"stepSize\":" + formatDouble(stepSize) +
				",\"steps\":" + std::to_string(steps) +
				",\"sampleCount\":" + std::to_string(result.sampleCount()) +
				",\"finalTime\":" + (result.empty() ? std::string("0") : formatDouble(result.getSamples().back().time)) + "}";
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

		std::string networkResolutionError;
		BioNetwork* network = resolveBioNetwork(_parentModel, _targetBioNetworkName, networkResolutionError);
		if (network == nullptr) {
			_lastStatus = "Failed";
			_lastErrorMessage = networkResolutionError;
			errorMessage = _lastErrorMessage;
			return false;
		}

		// If the network has never been simulated yet, run it once with its own configured window.
		if (network->getLastSimulationResult().empty()) {
			if (!network->simulate(errorMessage)) {
				_lastStatus = "Failed";
				_lastErrorMessage = errorMessage;
				return false;
			}
		}

		BioSteadyStateCheck check;
		if (!network->checkLastSampleSteadyState(1e-9, &check, &errorMessage)) {
			_lastStatus = "Failed";
			_lastErrorMessage = errorMessage;
			return false;
		}

		_targetBioNetworkName = network->getName();
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "steady_state") +
				",\"targetBioNetworkName\":\"" + jsonEscape(_targetBioNetworkName) + "\"" +
				",\"steady\":" + std::string(check.steady ? "true" : "false") +
				",\"tolerance\":" + formatDouble(check.tolerance) +
				",\"maxAbsoluteDerivative\":" + formatDouble(check.maxAbsoluteDerivative) +
				",\"derivativeCount\":" + std::to_string(check.derivatives.size()) + "}";
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
		std::string definitionType;
		ModelDataDefinition* definition = resolveSymbolDefinition(_parentModel, symbol, &definitionType);
		if (definition == nullptr) {
			_lastStatus = "Failed";
			_lastErrorMessage = "getValue(\"symbol\") could not resolve \"" + symbol + "\" as a BioSpecies or BioParameter.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		double value = 0.0;
		if (definitionType == Util::TypeOf<BioSpecies>()) {
			value = dynamic_cast<BioSpecies*>(definition)->getAmount();
		} else if (definitionType == Util::TypeOf<BioParameter>()) {
			value = dynamic_cast<BioParameter*>(definition)->getValue();
		}
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "value_query") +
				",\"symbol\":\"" + jsonEscape(symbol) + "\"" +
				",\"definitionType\":\"" + jsonEscape(definitionType) + "\"" +
				",\"value\":" + formatDouble(value) + "}";
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
		std::string definitionType;
		ModelDataDefinition* definition = resolveSymbolDefinition(_parentModel, symbol, &definitionType);
		if (definition == nullptr) {
			_lastStatus = "Failed";
			_lastErrorMessage = "setValue(\"symbol\", value) could not resolve \"" + symbol + "\" as a BioSpecies or BioParameter.";
			errorMessage = _lastErrorMessage;
			return false;
		}

		if (definitionType == Util::TypeOf<BioSpecies>()) {
			auto* species = dynamic_cast<BioSpecies*>(definition);
			if (species == nullptr || value < 0.0) {
				_lastStatus = "Failed";
				_lastErrorMessage = "setValue(\"symbol\", value) requires a non-negative species amount.";
				errorMessage = _lastErrorMessage;
				return false;
			}
			species->setAmount(value);
		} else if (definitionType == Util::TypeOf<BioParameter>()) {
			auto* parameter = dynamic_cast<BioParameter*>(definition);
			if (parameter == nullptr) {
				_lastStatus = "Failed";
				_lastErrorMessage = "setValue(\"symbol\", value) could not update the resolved BioParameter.";
				errorMessage = _lastErrorMessage;
				return false;
			}
			parameter->setValue(value);
		}
		_parentModel->setHasChanged(true);
		_lastStatus = "Completed";
		_lastResponsePayload = makePayloadPrefix(true, _lastStatus, _backend, normalizedCommand, "value_update") +
				",\"symbol\":\"" + jsonEscape(symbol) + "\"" +
				",\"definitionType\":\"" + jsonEscape(definitionType) + "\"" +
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
		std::string networkResolutionError;
		BioNetwork* network = resolveBioNetwork(_parentModel, _targetBioNetworkName, networkResolutionError);
		if (network != nullptr) {
			// Reset the selected network back to its initial amounts before clearing runner state.
			ModelDataDefinition::InitBetweenReplications(network);
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

void BioSimulatorRunner::setTargetBioNetworkName(std::string targetBioNetworkName) {
	_targetBioNetworkName = targetBioNetworkName;
}

std::string BioSimulatorRunner::getTargetBioNetworkName() const {
	return _targetBioNetworkName;
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

// void BioSimulatorRunner::_createInternalStatisticReporters() { }

// void BioSimulatorRunner::_createEditableDataDefinitions() { }

// void BioSimulatorRunner::_createAttachedAttributes() { }
