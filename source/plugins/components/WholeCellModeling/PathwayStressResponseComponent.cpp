#include "plugins/components/WholeCellModeling/PathwayStressResponseComponent.h"

#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PathwayStressResponseComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* PathwayStressResponseComponent::NewInstance(Model* model, std::string name) {
	return new PathwayStressResponseComponent(model, name);
}

PathwayStressResponseComponent::PathwayStressResponseComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<PathwayStressResponseComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&PathwayStressResponseComponent::getWholeCellState, this),
			std::bind(&PathwayStressResponseComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "WholeCellState", "");
	auto* propPathwayKey = new SimulationControlString(
			std::bind(&PathwayStressResponseComponent::getMonitoredPathwayKey, this),
			std::bind(&PathwayStressResponseComponent::setMonitoredPathwayKey, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "MonitoredPathwayKey", "");
	auto* propThreshold = new SimulationControlDouble(
			std::bind(&PathwayStressResponseComponent::getStressThreshold, this),
			std::bind(&PathwayStressResponseComponent::setStressThreshold, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "StressThreshold", "");
	auto* propArrestAfter = new SimulationControlUInt(
			std::bind(&PathwayStressResponseComponent::getArrestAfterSteps, this),
			std::bind(&PathwayStressResponseComponent::setArrestAfterSteps, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "ArrestAfterSteps", "");
	auto* propDeathAfter = new SimulationControlUInt(
			std::bind(&PathwayStressResponseComponent::getDeathAfterSteps, this),
			std::bind(&PathwayStressResponseComponent::setDeathAfterSteps, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "DeathAfterSteps", "");
	auto* propArrestPhase = new SimulationControlString(
			std::bind(&PathwayStressResponseComponent::getArrestPhase, this),
			std::bind(&PathwayStressResponseComponent::setArrestPhase, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "ArrestPhase", "");
	auto* propDeadPhase = new SimulationControlString(
			std::bind(&PathwayStressResponseComponent::getDeadPhase, this),
			std::bind(&PathwayStressResponseComponent::setDeadPhase, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "DeadPhase", "");
	auto* propRecoveryPhase = new SimulationControlString(
			std::bind(&PathwayStressResponseComponent::getRecoveryPhase, this),
			std::bind(&PathwayStressResponseComponent::setRecoveryPhase, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "RecoveryPhase", "");
	auto* propResetRecovery = new SimulationControlBool(
			std::bind(&PathwayStressResponseComponent::getResetStreakOnRecovery, this),
			std::bind(&PathwayStressResponseComponent::setResetStreakOnRecovery, this, std::placeholders::_1),
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "ResetStreakOnRecovery", "");
	auto* propStressStreak = new SimulationControlUInt(
			std::bind(&PathwayStressResponseComponent::getStressStreak, this),
			nullptr,
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "StressStreak", "");
	auto* propLastActivity = new SimulationControlDouble(
			std::bind(&PathwayStressResponseComponent::getLastObservedActivity, this),
			nullptr,
			Util::TypeOf<PathwayStressResponseComponent>(), getName(), "LastObservedActivity", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propPathwayKey);
	_parentModel->getControls()->insert(propThreshold);
	_parentModel->getControls()->insert(propArrestAfter);
	_parentModel->getControls()->insert(propDeathAfter);
	_parentModel->getControls()->insert(propArrestPhase);
	_parentModel->getControls()->insert(propDeadPhase);
	_parentModel->getControls()->insert(propRecoveryPhase);
	_parentModel->getControls()->insert(propResetRecovery);
	_parentModel->getControls()->insert(propStressStreak);
	_parentModel->getControls()->insert(propLastActivity);

	_addSimulationControl(propState);
	_addSimulationControl(propPathwayKey);
	_addSimulationControl(propThreshold);
	_addSimulationControl(propArrestAfter);
	_addSimulationControl(propDeathAfter);
	_addSimulationControl(propArrestPhase);
	_addSimulationControl(propDeadPhase);
	_addSimulationControl(propRecoveryPhase);
	_addSimulationControl(propResetRecovery);
	_addSimulationControl(propStressStreak);
	_addSimulationControl(propLastActivity);
}

PluginInformation* PathwayStressResponseComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PathwayStressResponseComponent>(), &PathwayStressResponseComponent::LoadInstance, &PathwayStressResponseComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("wholecellstate.so");
	info->setDescriptionHelp(
		"Applies sustained pathway-collapse responses to WholeCellState, "
		"including arrest and death after configurable streak lengths.");
	return info;
}

ModelComponent* PathwayStressResponseComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	PathwayStressResponseComponent* newComponent = new PathwayStressResponseComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load PathwayStressResponseComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string PathwayStressResponseComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",monitoredPathwayKey=\"" + _monitoredPathwayKey + "\"" +
			",stressThreshold=" + std::to_string(_stressThreshold) +
			",arrestAfterSteps=" + std::to_string(_arrestAfterSteps) +
			",deathAfterSteps=" + std::to_string(_deathAfterSteps) +
			",arrestPhase=\"" + _arrestPhase + "\"" +
			",deadPhase=\"" + _deadPhase + "\"" +
			",recoveryPhase=\"" + _recoveryPhase + "\"" +
			",resetStreakOnRecovery=" + std::string(_resetStreakOnRecovery ? "true" : "false") +
			",stressStreak=" + std::to_string(_stressStreak) +
			",lastObservedActivity=" + std::to_string(_lastObservedActivity);
}

bool PathwayStressResponseComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_monitoredPathwayKey = fields->loadField("monitoredPathwayKey", DEFAULT.monitoredPathwayKey);
		_stressThreshold = fields->loadField("stressThreshold", DEFAULT.stressThreshold);
		_arrestAfterSteps = fields->loadField("arrestAfterSteps", DEFAULT.arrestAfterSteps);
		_deathAfterSteps = fields->loadField("deathAfterSteps", DEFAULT.deathAfterSteps);
		_arrestPhase = fields->loadField("arrestPhase", DEFAULT.arrestPhase);
		_deadPhase = fields->loadField("deadPhase", DEFAULT.deadPhase);
		_recoveryPhase = fields->loadField("recoveryPhase", DEFAULT.recoveryPhase);
		_resetStreakOnRecovery = fields->loadField("resetStreakOnRecovery", DEFAULT.resetStreakOnRecovery);
		_stressStreak = fields->loadField("stressStreak", DEFAULT.stressStreak);
		_lastObservedActivity = fields->loadField("lastObservedActivity", DEFAULT.lastObservedActivity);
	}
	_createEditableDataDefinitions();
	return res;
}

void PathwayStressResponseComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("monitoredPathwayKey", _monitoredPathwayKey, DEFAULT.monitoredPathwayKey, saveDefaultValues);
	fields->saveField("stressThreshold", _stressThreshold, DEFAULT.stressThreshold, saveDefaultValues);
	fields->saveField("arrestAfterSteps", _arrestAfterSteps, DEFAULT.arrestAfterSteps, saveDefaultValues);
	fields->saveField("deathAfterSteps", _deathAfterSteps, DEFAULT.deathAfterSteps, saveDefaultValues);
	fields->saveField("arrestPhase", _arrestPhase, DEFAULT.arrestPhase, saveDefaultValues);
	fields->saveField("deadPhase", _deadPhase, DEFAULT.deadPhase, saveDefaultValues);
	fields->saveField("recoveryPhase", _recoveryPhase, DEFAULT.recoveryPhase, saveDefaultValues);
	fields->saveField("resetStreakOnRecovery", _resetStreakOnRecovery, DEFAULT.resetStreakOnRecovery, saveDefaultValues);
	fields->saveField("stressStreak", _stressStreak, DEFAULT.stressStreak, saveDefaultValues);
	fields->saveField("lastObservedActivity", _lastObservedActivity, DEFAULT.lastObservedActivity, saveDefaultValues);
}

bool PathwayStressResponseComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_monitoredPathwayKey.empty()) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" monitoredPathwayKey must be non-empty. ";
		resultAll = false;
	}
	if (_stressThreshold < 0.0) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" stressThreshold must be >= 0. ";
		resultAll = false;
	}
	if (_arrestAfterSteps > 0u && _arrestPhase.empty()) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" arrestPhase must be non-empty when arrestAfterSteps > 0. ";
		resultAll = false;
	}
	if (_deathAfterSteps > 0u && _deadPhase.empty()) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" deadPhase must be non-empty when deathAfterSteps > 0. ";
		resultAll = false;
	}
	if (_arrestAfterSteps > 0u && _deathAfterSteps > 0u && _deathAfterSteps < _arrestAfterSteps) {
		errorMessage += "PathwayStressResponseComponent \"" + getName() + "\" deathAfterSteps must be >= arrestAfterSteps when both are configured. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void PathwayStressResponseComponent::_initBetweenReplications() {
	_stressStreak = DEFAULT.stressStreak;
	_lastObservedActivity = DEFAULT.lastObservedActivity;
}

void PathwayStressResponseComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"PathwayStressResponseComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	_lastObservedActivity = _wholeCellState->getPathwayActivity(_monitoredPathwayKey);
	const bool underStress = _lastObservedActivity < _stressThreshold;

	if (underStress) {
		++_stressStreak;
		if (_deathAfterSteps > 0u && _stressStreak >= _deathAfterSteps) {
			_wholeCellState->setViable(false);
			_wholeCellState->setLifecyclePhase(_deadPhase);
		} else if (_arrestAfterSteps > 0u && _stressStreak >= _arrestAfterSteps && _wholeCellState->isViable()) {
			_wholeCellState->setLifecyclePhase(_arrestPhase);
		}
	} else {
		if (_resetStreakOnRecovery) {
			_stressStreak = 0u;
		}
		if (_wholeCellState->isViable() && !_recoveryPhase.empty() && _wholeCellState->getLifecyclePhase() == _arrestPhase) {
			_wholeCellState->setLifecyclePhase(_recoveryPhase);
		}
	}

	_forwardEntity(entity);
}

void PathwayStressResponseComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void PathwayStressResponseComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getFrontConnection();
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "PathwayStressResponseComponent dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void PathwayStressResponseComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; _createEditableDataDefinitions(); }
WholeCellState* PathwayStressResponseComponent::getWholeCellState() const { return _wholeCellState; }
void PathwayStressResponseComponent::setMonitoredPathwayKey(std::string key) { _monitoredPathwayKey = std::move(key); }
std::string PathwayStressResponseComponent::getMonitoredPathwayKey() const { return _monitoredPathwayKey; }
void PathwayStressResponseComponent::setStressThreshold(double threshold) { _stressThreshold = threshold; }
double PathwayStressResponseComponent::getStressThreshold() const { return _stressThreshold; }
void PathwayStressResponseComponent::setArrestAfterSteps(unsigned int steps) { _arrestAfterSteps = steps; }
unsigned int PathwayStressResponseComponent::getArrestAfterSteps() const { return _arrestAfterSteps; }
void PathwayStressResponseComponent::setDeathAfterSteps(unsigned int steps) { _deathAfterSteps = steps; }
unsigned int PathwayStressResponseComponent::getDeathAfterSteps() const { return _deathAfterSteps; }
void PathwayStressResponseComponent::setArrestPhase(std::string phase) { _arrestPhase = std::move(phase); }
std::string PathwayStressResponseComponent::getArrestPhase() const { return _arrestPhase; }
void PathwayStressResponseComponent::setDeadPhase(std::string phase) { _deadPhase = std::move(phase); }
std::string PathwayStressResponseComponent::getDeadPhase() const { return _deadPhase; }
void PathwayStressResponseComponent::setRecoveryPhase(std::string phase) { _recoveryPhase = std::move(phase); }
std::string PathwayStressResponseComponent::getRecoveryPhase() const { return _recoveryPhase; }
void PathwayStressResponseComponent::setResetStreakOnRecovery(bool reset) { _resetStreakOnRecovery = reset; }
bool PathwayStressResponseComponent::getResetStreakOnRecovery() const { return _resetStreakOnRecovery; }
unsigned int PathwayStressResponseComponent::getStressStreak() const { return _stressStreak; }
double PathwayStressResponseComponent::getLastObservedActivity() const { return _lastObservedActivity; }
