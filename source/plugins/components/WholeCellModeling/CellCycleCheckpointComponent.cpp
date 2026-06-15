#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"

#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellCycleCheckpointComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* CellCycleCheckpointComponent::NewInstance(Model* model, std::string name) {
	return new CellCycleCheckpointComponent(model, name);
}

CellCycleCheckpointComponent::CellCycleCheckpointComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<CellCycleCheckpointComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&CellCycleCheckpointComponent::getWholeCellState, this),
			std::bind(&CellCycleCheckpointComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "WholeCellState", "");
	auto* propDeltaT = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getDeltaT, this),
			std::bind(&CellCycleCheckpointComponent::setDeltaT, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "DeltaT", "");
	auto* propAdvanceClock = new SimulationControlBool(
			std::bind(&CellCycleCheckpointComponent::getAdvanceWholeCellClock, this),
			std::bind(&CellCycleCheckpointComponent::setAdvanceWholeCellClock, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "AdvanceWholeCellClock", "");
	auto* propEnergyKey = new SimulationControlString(
			std::bind(&CellCycleCheckpointComponent::getEnergyMetaboliteKey, this),
			std::bind(&CellCycleCheckpointComponent::setEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "EnergyMetaboliteKey", "");
	auto* propCompartmentRegion = new SimulationControlString(
			std::bind(&CellCycleCheckpointComponent::getCompartmentEnergyRegion, this),
			std::bind(&CellCycleCheckpointComponent::setCompartmentEnergyRegion, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "CompartmentEnergyRegion", "");
	auto* propCompartmentEnergyKey = new SimulationControlString(
			std::bind(&CellCycleCheckpointComponent::getCompartmentEnergyMetaboliteKey, this),
			std::bind(&CellCycleCheckpointComponent::setCompartmentEnergyMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "CompartmentEnergyMetaboliteKey", "");
	auto* propStarvationThreshold = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getStarvationAtpThreshold, this),
			std::bind(&CellCycleCheckpointComponent::setStarvationAtpThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "StarvationAtpThreshold", "");
	auto* propCompartmentStarvationThreshold = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getCompartmentStarvationThreshold, this),
			std::bind(&CellCycleCheckpointComponent::setCompartmentStarvationThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "CompartmentStarvationThreshold", "");
	auto* propLethalSteps = new SimulationControlUInt(
			std::bind(&CellCycleCheckpointComponent::getLethalStarvationSteps, this),
			std::bind(&CellCycleCheckpointComponent::setLethalStarvationSteps, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "LethalStarvationSteps", "");
	auto* propDivisionMass = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getDivisionMassThreshold, this),
			std::bind(&CellCycleCheckpointComponent::setDivisionMassThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "DivisionMassThreshold", "");
	auto* propFtsZKey = new SimulationControlString(
			std::bind(&CellCycleCheckpointComponent::getFtsZRingKey, this),
			std::bind(&CellCycleCheckpointComponent::setFtsZRingKey, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "FtsZRingKey", "");
	auto* propFtsZThreshold = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getFtsZThreshold, this),
			std::bind(&CellCycleCheckpointComponent::setFtsZThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "FtsZThreshold", "");
	auto* propCriticalPathwayKey = new SimulationControlString(
			std::bind(&CellCycleCheckpointComponent::getCriticalPathwayActivityKey, this),
			std::bind(&CellCycleCheckpointComponent::setCriticalPathwayActivityKey, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "CriticalPathwayActivityKey", "");
	auto* propCriticalPathwayThreshold = new SimulationControlDouble(
			std::bind(&CellCycleCheckpointComponent::getCriticalPathwayActivityThreshold, this),
			std::bind(&CellCycleCheckpointComponent::setCriticalPathwayActivityThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "CriticalPathwayActivityThreshold", "");
	auto* propStarvationStreak = new SimulationControlUInt(
			std::bind(&CellCycleCheckpointComponent::getStarvationStreak, this),
			nullptr,
			Util::TypeOf<CellCycleCheckpointComponent>(), getName(), "StarvationStreak", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propDeltaT);
	_parentModel->getControls()->insert(propAdvanceClock);
	_parentModel->getControls()->insert(propEnergyKey);
	_parentModel->getControls()->insert(propCompartmentRegion);
	_parentModel->getControls()->insert(propCompartmentEnergyKey);
	_parentModel->getControls()->insert(propStarvationThreshold);
	_parentModel->getControls()->insert(propCompartmentStarvationThreshold);
	_parentModel->getControls()->insert(propLethalSteps);
	_parentModel->getControls()->insert(propDivisionMass);
	_parentModel->getControls()->insert(propFtsZKey);
	_parentModel->getControls()->insert(propFtsZThreshold);
	_parentModel->getControls()->insert(propCriticalPathwayKey);
	_parentModel->getControls()->insert(propCriticalPathwayThreshold);
	_parentModel->getControls()->insert(propStarvationStreak);

	_addSimulationControl(propState);
	_addSimulationControl(propDeltaT);
	_addSimulationControl(propAdvanceClock);
	_addSimulationControl(propEnergyKey);
	_addSimulationControl(propCompartmentRegion);
	_addSimulationControl(propCompartmentEnergyKey);
	_addSimulationControl(propStarvationThreshold);
	_addSimulationControl(propCompartmentStarvationThreshold);
	_addSimulationControl(propLethalSteps);
	_addSimulationControl(propDivisionMass);
	_addSimulationControl(propFtsZKey);
	_addSimulationControl(propFtsZThreshold);
	_addSimulationControl(propCriticalPathwayKey);
	_addSimulationControl(propCriticalPathwayThreshold);
	_addSimulationControl(propStarvationStreak);
}

PluginInformation* CellCycleCheckpointComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellCycleCheckpointComponent>(), &CellCycleCheckpointComponent::LoadInstance, &CellCycleCheckpointComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Evaluates whole-cell lifecycle checkpoints and advances WholeCellState time by one event-window. "
		"Annotates lifecycle phase using ATP starvation and division-readiness thresholds without executing division directly.");
	return info;
}

ModelComponent* CellCycleCheckpointComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	CellCycleCheckpointComponent* newComponent = new CellCycleCheckpointComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CellCycleCheckpointComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string CellCycleCheckpointComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",deltaT=" + std::to_string(_deltaT) +
			",advanceWholeCellClock=" + std::string(_advanceWholeCellClock ? "true" : "false") +
			",energyMetaboliteKey=\"" + _energyMetaboliteKey + "\"" +
			",compartmentEnergyRegion=\"" + _compartmentEnergyRegion + "\"" +
			",compartmentEnergyMetaboliteKey=\"" + _compartmentEnergyMetaboliteKey + "\"" +
			",starvationAtpThreshold=" + std::to_string(_starvationAtpThreshold) +
			",compartmentStarvationThreshold=" + std::to_string(_compartmentStarvationThreshold) +
			",lethalStarvationSteps=" + std::to_string(_lethalStarvationSteps) +
			",divisionMassThreshold=" + std::to_string(_divisionMassThreshold) +
			",ftsZRingKey=\"" + _ftsZRingKey + "\"" +
			",ftsZThreshold=" + std::to_string(_ftsZThreshold) +
			",criticalPathwayActivityKey=\"" + _criticalPathwayActivityKey + "\"" +
			",criticalPathwayActivityThreshold=" + std::to_string(_criticalPathwayActivityThreshold) +
			",starvationStreak=" + std::to_string(_starvationStreak);
}

bool CellCycleCheckpointComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_deltaT = fields->loadField("deltaT", DEFAULT.deltaT);
		_advanceWholeCellClock = fields->loadField("advanceWholeCellClock", DEFAULT.advanceWholeCellClock);
		_energyMetaboliteKey = fields->loadField("energyMetaboliteKey", DEFAULT.energyMetaboliteKey);
		_compartmentEnergyRegion = fields->loadField("compartmentEnergyRegion", DEFAULT.compartmentEnergyRegion);
		_compartmentEnergyMetaboliteKey = fields->loadField("compartmentEnergyMetaboliteKey", DEFAULT.compartmentEnergyMetaboliteKey);
		_starvationAtpThreshold = fields->loadField("starvationAtpThreshold", DEFAULT.starvationAtpThreshold);
		_compartmentStarvationThreshold = fields->loadField("compartmentStarvationThreshold", DEFAULT.compartmentStarvationThreshold);
		_lethalStarvationSteps = fields->loadField("lethalStarvationSteps", DEFAULT.lethalStarvationSteps);
		_divisionMassThreshold = fields->loadField("divisionMassThreshold", DEFAULT.divisionMassThreshold);
		_ftsZRingKey = fields->loadField("ftsZRingKey", DEFAULT.ftsZRingKey);
		_ftsZThreshold = fields->loadField("ftsZThreshold", DEFAULT.ftsZThreshold);
		_criticalPathwayActivityKey = fields->loadField("criticalPathwayActivityKey", DEFAULT.criticalPathwayActivityKey);
		_criticalPathwayActivityThreshold = fields->loadField("criticalPathwayActivityThreshold", DEFAULT.criticalPathwayActivityThreshold);
	}
	return res;
}

void CellCycleCheckpointComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("deltaT", _deltaT, DEFAULT.deltaT, saveDefaultValues);
	fields->saveField("advanceWholeCellClock", _advanceWholeCellClock, DEFAULT.advanceWholeCellClock, saveDefaultValues);
	fields->saveField("energyMetaboliteKey", _energyMetaboliteKey, DEFAULT.energyMetaboliteKey, saveDefaultValues);
	fields->saveField("compartmentEnergyRegion", _compartmentEnergyRegion, DEFAULT.compartmentEnergyRegion, saveDefaultValues);
	fields->saveField("compartmentEnergyMetaboliteKey", _compartmentEnergyMetaboliteKey, DEFAULT.compartmentEnergyMetaboliteKey, saveDefaultValues);
	fields->saveField("starvationAtpThreshold", _starvationAtpThreshold, DEFAULT.starvationAtpThreshold, saveDefaultValues);
	fields->saveField("compartmentStarvationThreshold", _compartmentStarvationThreshold, DEFAULT.compartmentStarvationThreshold, saveDefaultValues);
	fields->saveField("lethalStarvationSteps", _lethalStarvationSteps, DEFAULT.lethalStarvationSteps, saveDefaultValues);
	fields->saveField("divisionMassThreshold", _divisionMassThreshold, DEFAULT.divisionMassThreshold, saveDefaultValues);
	fields->saveField("ftsZRingKey", _ftsZRingKey, DEFAULT.ftsZRingKey, saveDefaultValues);
	fields->saveField("ftsZThreshold", _ftsZThreshold, DEFAULT.ftsZThreshold, saveDefaultValues);
	fields->saveField("criticalPathwayActivityKey", _criticalPathwayActivityKey, DEFAULT.criticalPathwayActivityKey, saveDefaultValues);
	fields->saveField("criticalPathwayActivityThreshold", _criticalPathwayActivityThreshold, DEFAULT.criticalPathwayActivityThreshold, saveDefaultValues);
}

bool CellCycleCheckpointComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_deltaT <= 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" deltaT must be > 0. ";
		resultAll = false;
	}
	if (_starvationAtpThreshold < 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" starvationAtpThreshold must be >= 0. ";
		resultAll = false;
	}
	if (_compartmentStarvationThreshold < 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" compartmentStarvationThreshold must be >= 0. ";
		resultAll = false;
	}
	if (_divisionMassThreshold < 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" divisionMassThreshold must be >= 0. ";
		resultAll = false;
	}
	if (_ftsZThreshold < 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" ftsZThreshold must be >= 0. ";
		resultAll = false;
	}
	if (_criticalPathwayActivityThreshold < 0.0) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" criticalPathwayActivityThreshold must be >= 0. ";
		resultAll = false;
	}
	if ((!_compartmentEnergyRegion.empty() && _compartmentEnergyMetaboliteKey.empty()) ||
	    (_compartmentEnergyRegion.empty() && !_compartmentEnergyMetaboliteKey.empty())) {
		errorMessage += "CellCycleCheckpointComponent \"" + getName() + "\" requires both compartmentEnergyRegion and compartmentEnergyMetaboliteKey together. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void CellCycleCheckpointComponent::_initBetweenReplications() {
	_starvationStreak = 0u;
}

void CellCycleCheckpointComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"CellCycleCheckpointComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	if (_advanceWholeCellClock) {
		_wholeCellState->setCurrentTime(_wholeCellState->getCurrentTime() + _deltaT);
		_wholeCellState->incrementStep();
	}

	_updateLifecyclePhase();
	_forwardEntity(entity);
}

void CellCycleCheckpointComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

bool CellCycleCheckpointComponent::_isDivisionReady() const {
	const bool hasMassConstraint = _divisionMassThreshold > 0.0;
	const bool hasFtsZConstraint = _ftsZThreshold > 0.0;
	if (!hasMassConstraint && !hasFtsZConstraint) {
		return false;
	}

	bool massReady = true;
	if (hasMassConstraint) {
		massReady = _wholeCellState->getCellMass() >= _divisionMassThreshold;
	}

	bool ftsZReady = true;
	if (hasFtsZConstraint) {
		const int ftsZRaw = _wholeCellState->getMoleculeCount(_ftsZRingKey);
		const double completion = static_cast<double>(ftsZRaw) / 1000.0;
		ftsZReady = completion >= _ftsZThreshold;
	}

	return massReady && ftsZReady;
}

void CellCycleCheckpointComponent::_updateLifecyclePhase() {
	if (!_wholeCellState->isViable()) {
		_wholeCellState->setLifecyclePhase("dead");
		return;
	}

	bool starved = false;
	if (!_energyMetaboliteKey.empty()) {
		const double energy = _wholeCellState->getMetaboliteAmount(_energyMetaboliteKey);
		starved = energy < _starvationAtpThreshold;
	}
	if (!starved && !_compartmentEnergyRegion.empty() && !_compartmentEnergyMetaboliteKey.empty()) {
		const double compartmentEnergy = _wholeCellState->getCompartmentMetaboliteAmount(_compartmentEnergyRegion, _compartmentEnergyMetaboliteKey);
		starved = compartmentEnergy < _compartmentStarvationThreshold;
	}
	if (!starved && !_criticalPathwayActivityKey.empty()) {
		const double pathwayActivity = _wholeCellState->getPathwayActivity(_criticalPathwayActivityKey);
		starved = pathwayActivity < _criticalPathwayActivityThreshold;
	}

	if (starved) {
		++_starvationStreak;
		if (_lethalStarvationSteps > 0u && _starvationStreak >= _lethalStarvationSteps) {
			_wholeCellState->setViable(false);
			_wholeCellState->setLifecyclePhase("dead");
			return;
		}
		_wholeCellState->setLifecyclePhase("starved");
		return;
	}

	_starvationStreak = 0u;
	if (_isDivisionReady()) {
		_wholeCellState->setLifecyclePhase("division_ready");
		return;
	}

	if (_wholeCellState->getGenerationCount() == 0 && _wholeCellState->getStepCount() <= 1) {
		_wholeCellState->setLifecyclePhase("newborn");
		return;
	}

	_wholeCellState->setLifecyclePhase("growth");
}

void CellCycleCheckpointComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "CellCycleCheckpointComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void CellCycleCheckpointComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* CellCycleCheckpointComponent::getWholeCellState() const { return _wholeCellState; }
void CellCycleCheckpointComponent::setDeltaT(double deltaT) { _deltaT = deltaT; }
double CellCycleCheckpointComponent::getDeltaT() const { return _deltaT; }
void CellCycleCheckpointComponent::setAdvanceWholeCellClock(bool advance) { _advanceWholeCellClock = advance; }
bool CellCycleCheckpointComponent::getAdvanceWholeCellClock() const { return _advanceWholeCellClock; }
void CellCycleCheckpointComponent::setEnergyMetaboliteKey(std::string key) { _energyMetaboliteKey = std::move(key); }
std::string CellCycleCheckpointComponent::getEnergyMetaboliteKey() const { return _energyMetaboliteKey; }
void CellCycleCheckpointComponent::setCompartmentEnergyRegion(std::string region) { _compartmentEnergyRegion = std::move(region); }
std::string CellCycleCheckpointComponent::getCompartmentEnergyRegion() const { return _compartmentEnergyRegion; }
void CellCycleCheckpointComponent::setCompartmentEnergyMetaboliteKey(std::string key) { _compartmentEnergyMetaboliteKey = std::move(key); }
std::string CellCycleCheckpointComponent::getCompartmentEnergyMetaboliteKey() const { return _compartmentEnergyMetaboliteKey; }
void CellCycleCheckpointComponent::setStarvationAtpThreshold(double threshold) { _starvationAtpThreshold = threshold; }
double CellCycleCheckpointComponent::getStarvationAtpThreshold() const { return _starvationAtpThreshold; }
void CellCycleCheckpointComponent::setCompartmentStarvationThreshold(double threshold) { _compartmentStarvationThreshold = threshold; }
double CellCycleCheckpointComponent::getCompartmentStarvationThreshold() const { return _compartmentStarvationThreshold; }
void CellCycleCheckpointComponent::setLethalStarvationSteps(unsigned int steps) { _lethalStarvationSteps = steps; }
unsigned int CellCycleCheckpointComponent::getLethalStarvationSteps() const { return _lethalStarvationSteps; }
void CellCycleCheckpointComponent::setDivisionMassThreshold(double threshold) { _divisionMassThreshold = threshold; }
double CellCycleCheckpointComponent::getDivisionMassThreshold() const { return _divisionMassThreshold; }
void CellCycleCheckpointComponent::setFtsZRingKey(std::string key) { _ftsZRingKey = std::move(key); }
std::string CellCycleCheckpointComponent::getFtsZRingKey() const { return _ftsZRingKey; }
void CellCycleCheckpointComponent::setFtsZThreshold(double threshold) { _ftsZThreshold = threshold; }
double CellCycleCheckpointComponent::getFtsZThreshold() const { return _ftsZThreshold; }
void CellCycleCheckpointComponent::setCriticalPathwayActivityKey(std::string key) { _criticalPathwayActivityKey = std::move(key); }
std::string CellCycleCheckpointComponent::getCriticalPathwayActivityKey() const { return _criticalPathwayActivityKey; }
void CellCycleCheckpointComponent::setCriticalPathwayActivityThreshold(double threshold) { _criticalPathwayActivityThreshold = threshold; }
double CellCycleCheckpointComponent::getCriticalPathwayActivityThreshold() const { return _criticalPathwayActivityThreshold; }
unsigned int CellCycleCheckpointComponent::getStarvationStreak() const { return _starvationStreak; }
