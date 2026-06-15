#include "plugins/components/ExternalIntegration/PythonForG.h"

#include <functional>

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PythonForG::GetPluginInformation;
}
#endif

ModelDataDefinition* PythonForG::NewInstance(Model* model, std::string name) {
	return new PythonForG(model, name);
}

PythonForG::PythonForG(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<PythonForG>(), name) {
	_createNonEditableDataDefinitions();

	auto* propInitCode = new SimulationControlSourceCodeString(
			std::bind(&PythonForG::getInitBetweenReplicationCodeProperty, this),
			std::bind(&PythonForG::setInitBetweenReplicationCodeProperty, this, std::placeholders::_1),
			Util::TypeOf<PythonForG>(), getName(), "InitBetweenReplicationCode",
			"Python statements executed once at the start of each replication.",
			false,
			false,
			false,
			std::bind(&PythonForG::_validateInitCode, this, std::placeholders::_1, std::placeholders::_2));
	auto* propDispatchCode = new SimulationControlSourceCodeString(
			std::bind(&PythonForG::getOnDispatchEventCodeProperty, this),
			std::bind(&PythonForG::setOnDispatchEventCodeProperty, this, std::placeholders::_1),
			Util::TypeOf<PythonForG>(), getName(), "OnDispatchEventCode",
			"Python statements executed whenever an entity arrives at this component.",
			false,
			false,
			false,
			std::bind(&PythonForG::_validateDispatchCode, this, std::placeholders::_1, std::placeholders::_2));
	auto* propForwardOnError = new SimulationControlGeneric<bool>(
			std::bind(&PythonForG::isForwardEntityOnError, this), std::bind(&PythonForG::setForwardEntityOnError, this, std::placeholders::_1),
			Util::TypeOf<PythonForG>(), getName(), "ForwardEntityOnError", "");

	_parentModel->getControls()->insert(propInitCode);
	_parentModel->getControls()->insert(propDispatchCode);
	_parentModel->getControls()->insert(propForwardOnError);

	_addSimulationControl(propInitCode);
	_addSimulationControl(propDispatchCode);
	_addSimulationControl(propForwardOnError);
}

PluginInformation* PythonForG::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PythonForG>(), &PythonForG::LoadInstance, &PythonForG::NewInstance);
	info->setCategory("ExternalIntegration");
	info->insertDynamicLibFileDependence("pythonruntime.so");
	info->setDescriptionHelp("Experimental component that executes embedded Python code when an entity arrives. It mirrors CppForG with two hooks: InitBetweenReplicationCode and OnDispatchEventCode. The global Python object 'simulator' exposes the current SimulatorFacade.");
	info->setObservation("PythonForG executes trusted model code in-process. It is experimental and not sandboxed.");
	return info;
}

ModelComponent* PythonForG::LoadInstance(Model* model, PersistenceRecord* fields) {
	PythonForG* newComponent = new PythonForG(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load PythonForG instance: " + std::string(e.what()));
	}
	return newComponent;
}

std::string PythonForG::show() {
	return ModelComponent::show()
			+ ",initCodeSize=" + std::to_string(_initBetweenReplicationCode.size())
			+ ",dispatchCodeSize=" + std::to_string(_onDispatchEventCode.size())
			+ ",forwardEntityOnError=" + std::string(_forwardEntityOnError ? "true" : "false")
			+ ",pythonRuntime=\"" + (_pythonRuntime != nullptr ? _pythonRuntime->getName() : std::string("")) + "\"";
}

void PythonForG::setInitBetweenReplicationCode(std::string initBetweenReplicationCode) {
	_initBetweenReplicationCode = std::move(initBetweenReplicationCode);
	if (_pythonRuntime != nullptr) {
		_pythonRuntime->resetExecutionState();
	}
}

std::string PythonForG::getInitBetweenReplicationCode() const {
	return _initBetweenReplicationCode;
}

void PythonForG::setInitBetweenReplicationCodeProperty(SourceCodeString initBetweenReplicationCode) {
	setInitBetweenReplicationCode(initBetweenReplicationCode.str());
}

SourceCodeString PythonForG::getInitBetweenReplicationCodeProperty() const {
	return SourceCodeString(_initBetweenReplicationCode);
}

void PythonForG::setOnDispatchEventCode(std::string onDispatchEventCode) {
	_onDispatchEventCode = std::move(onDispatchEventCode);
	if (_pythonRuntime != nullptr) {
		_pythonRuntime->resetExecutionState();
	}
}

std::string PythonForG::getOnDispatchEventCode() const {
	return _onDispatchEventCode;
}

void PythonForG::setOnDispatchEventCodeProperty(SourceCodeString onDispatchEventCode) {
	setOnDispatchEventCode(onDispatchEventCode.str());
}

SourceCodeString PythonForG::getOnDispatchEventCodeProperty() const {
	return SourceCodeString(_onDispatchEventCode);
}

void PythonForG::setForwardEntityOnError(bool forwardEntityOnError) {
	_forwardEntityOnError = forwardEntityOnError;
}

bool PythonForG::isForwardEntityOnError() const {
	return _forwardEntityOnError;
}

PythonRuntime* PythonForG::getPythonRuntime() const {
	return _pythonRuntime;
}

bool PythonForG::_validateInitCode(const std::string& candidate, std::string& errorMessage) const {
	if (_pythonRuntime == nullptr) {
		errorMessage = "PythonRuntime is not available.";
		return false;
	}
	return _pythonRuntime->validateHooks(candidate, _onDispatchEventCode, errorMessage);
}

bool PythonForG::_validateDispatchCode(const std::string& candidate, std::string& errorMessage) const {
	if (_pythonRuntime == nullptr) {
		errorMessage = "PythonRuntime is not available.";
		return false;
	}
	return _pythonRuntime->validateHooks(_initBetweenReplicationCode, candidate, errorMessage);
}

bool PythonForG::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_initBetweenReplicationCode = fields->loadField("initBetweenReplicationCode", DEFAULT.initBetweenReplicationCode);
		_onDispatchEventCode = fields->loadField("onDispatchEventCode", DEFAULT.onDispatchEventCode);
		_forwardEntityOnError = fields->loadField("forwardEntityOnError", DEFAULT.forwardEntityOnError);
		if (_pythonRuntime != nullptr) {
			_pythonRuntime->resetExecutionState();
		}
	}
	return res;
}

void PythonForG::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("initBetweenReplicationCode", _initBetweenReplicationCode, DEFAULT.initBetweenReplicationCode, saveDefaultValues);
	fields->saveField("onDispatchEventCode", _onDispatchEventCode, DEFAULT.onDispatchEventCode, saveDefaultValues);
	fields->saveField("forwardEntityOnError", _forwardEntityOnError, DEFAULT.forwardEntityOnError, saveDefaultValues);
}

bool PythonForG::_check(std::string& errorMessage) {
	bool resultAll = ModelDataDefinition::_check(errorMessage);
	_createNonEditableDataDefinitions();
	if (_pythonRuntime == nullptr) {
		errorMessage += "PythonRuntime internal data definition could not be created. ";
		return false;
	}
	std::string runtimeError;
	resultAll &= _pythonRuntime->validateHooks(_initBetweenReplicationCode, _onDispatchEventCode, runtimeError);
	if (!runtimeError.empty()) {
		errorMessage += runtimeError;
	}
	return resultAll;
}

void PythonForG::_initBetweenReplications() {
	if (_pythonRuntime == nullptr) {
		traceError("PythonRuntime is not available for " + getName());
		return;
	}
	std::string errorMessage;
	if (!_pythonRuntime->executeInitHook(this, _initBetweenReplicationCode, _onDispatchEventCode, errorMessage)) {
		traceError("PythonForG init hook failed in \"" + getName() + "\": " + errorMessage);
	}
	if (!_pythonRuntime->getLastStdout().empty()) {
		trace("PythonForG init stdout: " + _pythonRuntime->getLastStdout(), TraceManager::Level::L7_internal);
	}
	if (!_pythonRuntime->getLastStderr().empty()) {
		traceError("PythonForG init stderr: " + _pythonRuntime->getLastStderr(), TraceManager::Level::L3_errorRecover);
	}
}

void PythonForG::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;

	bool executionSucceeded = true;
	std::string errorMessage;
	if (_pythonRuntime == nullptr) {
		executionSucceeded = false;
		errorMessage = "PythonRuntime is not available.";
	} else {
		executionSucceeded = _pythonRuntime->executeOnDispatchHook(this, entity, _initBetweenReplicationCode, _onDispatchEventCode, errorMessage);
	}

	if (!executionSucceeded) {
		traceError("PythonForG dispatch hook failed in \"" + getName() + "\": " + errorMessage);
		if (_pythonRuntime != nullptr && !_pythonRuntime->getLastStderr().empty()) {
			traceError("PythonForG dispatch stderr: " + _pythonRuntime->getLastStderr(), TraceManager::Level::L3_errorRecover);
		}
		if (!_forwardEntityOnError) {
			return;
		}
	}

	if (_pythonRuntime != nullptr && !_pythonRuntime->getLastStdout().empty()) {
		trace("PythonForG dispatch stdout: " + _pythonRuntime->getLastStdout(), TraceManager::Level::L7_internal);
	}

	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceError("PythonForG dispatch skipped: invalid front connection.");
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void PythonForG::_createNonEditableDataDefinitions() {
	if (_pythonRuntime == nullptr) {
		_pythonRuntime = new PythonRuntime(_parentModel, getName() + ".PythonRuntime");
	}
	if (_pythonRuntime != nullptr) {
		_mandatoryNonEditableDataDefinitionInsert("PythonRuntime", _pythonRuntime);
	} else {
		_mandatoryNonEditableDataDefinitionRemove("PythonRuntime");
	}
}
