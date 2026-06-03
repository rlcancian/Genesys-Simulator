#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellGrowthComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* CellGrowthComponent::NewInstance(Model* model, std::string name) {
	return new CellGrowthComponent(model, name);
}

CellGrowthComponent::CellGrowthComponent(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<CellGrowthComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&CellGrowthComponent::getWholeCellState, this),
			std::bind(&CellGrowthComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<CellGrowthComponent>(), getName(), "WholeCellState", "");
	auto* propGrowthRate = new SimulationControlDouble(
			std::bind(&CellGrowthComponent::getGrowthRate, this),
			std::bind(&CellGrowthComponent::setGrowthRate, this, std::placeholders::_1),
			Util::TypeOf<CellGrowthComponent>(), getName(), "GrowthRate", "");
	auto* propDensity = new SimulationControlDouble(
			std::bind(&CellGrowthComponent::getDensity, this),
			std::bind(&CellGrowthComponent::setDensity, this, std::placeholders::_1),
			Util::TypeOf<CellGrowthComponent>(), getName(), "Density", "");
	auto* propDeltaT = new SimulationControlDouble(
			std::bind(&CellGrowthComponent::getDeltaT, this),
			std::bind(&CellGrowthComponent::setDeltaT, this, std::placeholders::_1),
			Util::TypeOf<CellGrowthComponent>(), getName(), "DeltaT", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propGrowthRate);
	_parentModel->getControls()->insert(propDensity);
	_parentModel->getControls()->insert(propDeltaT);

	_addSimulationControl(propState);
	_addSimulationControl(propGrowthRate);
	_addSimulationControl(propDensity);
	_addSimulationControl(propDeltaT);
}

PluginInformation* CellGrowthComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellGrowthComponent>(), &CellGrowthComponent::LoadInstance, &CellGrowthComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Continuous cell growth applied at each simulation step. "
		"Applies exponential mass growth: mass += mass * growthRate * deltaT. "
		"Updates cell volume from mass and cytoplasm density. "
		"Default growthRate=2.1393e-05/s and density=1100 kg/m3 from M. genitalium parameters.json.");
	return info;
}

ModelComponent* CellGrowthComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	CellGrowthComponent* newComponent = new CellGrowthComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CellGrowthComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string CellGrowthComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",growthRate=" + std::to_string(_growthRate) +
			",density=" + std::to_string(_density) +
			",deltaT=" + std::to_string(_deltaT);
}

bool CellGrowthComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_growthRate = fields->loadField("growthRate", DEFAULT.growthRate);
		_density    = fields->loadField("density",    DEFAULT.density);
		_deltaT     = fields->loadField("deltaT",     DEFAULT.deltaT);
	}
	return res;
}

void CellGrowthComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("growthRate", _growthRate, DEFAULT.growthRate, saveDefaultValues);
	fields->saveField("density",    _density,    DEFAULT.density,    saveDefaultValues);
	fields->saveField("deltaT",     _deltaT,     DEFAULT.deltaT,     saveDefaultValues);
}

bool CellGrowthComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "CellGrowthComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_growthRate < 0.0) {
		errorMessage += "CellGrowthComponent \"" + getName() + "\" growthRate must be >= 0. ";
		resultAll = false;
	}
	if (_density <= 0.0) {
		errorMessage += "CellGrowthComponent \"" + getName() + "\" density must be > 0. ";
		resultAll = false;
	}
	if (_deltaT <= 0.0) {
		errorMessage += "CellGrowthComponent \"" + getName() + "\" deltaT must be > 0. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void CellGrowthComponent::_initBetweenReplications() {
	// WholeCellState restores cell mass/volume in its own _initBetweenReplications.
}

void CellGrowthComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"CellGrowthComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	const double mass = _wholeCellState->getCellMass();
	const double newMass = mass + mass * _growthRate * _deltaT;
	const double newVolume = (newMass * 1e-3) / _density * 1e15;  // g→kg; m³→fL

	_wholeCellState->setCellMass(newMass);
	_wholeCellState->setCellVolume(newVolume);

	_forwardEntity(entity);
}

void CellGrowthComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void CellGrowthComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "CellGrowthComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void CellGrowthComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* CellGrowthComponent::getWholeCellState() const      { return _wholeCellState; }
void CellGrowthComponent::setGrowthRate(double rate)                 { _growthRate = rate; }
double CellGrowthComponent::getGrowthRate() const                    { return _growthRate; }
void CellGrowthComponent::setDensity(double density)                 { _density = density; }
double CellGrowthComponent::getDensity() const                       { return _density; }
void CellGrowthComponent::setDeltaT(double dt)                       { _deltaT = dt; }
double CellGrowthComponent::getDeltaT() const                        { return _deltaT; }
