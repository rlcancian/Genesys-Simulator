#include "plugins/components/WholeCellModeling/MetabolicSubmodelComponent.h"

#include <algorithm>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MetabolicSubmodelComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* MetabolicSubmodelComponent::NewInstance(Model* model, std::string name) {
	return new MetabolicSubmodelComponent(model, name);
}

MetabolicSubmodelComponent::MetabolicSubmodelComponent(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<MetabolicSubmodelComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&MetabolicSubmodelComponent::getWholeCellState, this),
			std::bind(&MetabolicSubmodelComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<MetabolicSubmodelComponent>(), getName(), "WholeCellState", "");
	auto* propYield = new SimulationControlDouble(
			std::bind(&MetabolicSubmodelComponent::getAtpYieldRate, this),
			std::bind(&MetabolicSubmodelComponent::setAtpYieldRate, this, std::placeholders::_1),
			Util::TypeOf<MetabolicSubmodelComponent>(), getName(), "AtpYieldRate", "");
	auto* propConsumption = new SimulationControlDouble(
			std::bind(&MetabolicSubmodelComponent::getConsumptionFraction, this),
			std::bind(&MetabolicSubmodelComponent::setConsumptionFraction, this, std::placeholders::_1),
			Util::TypeOf<MetabolicSubmodelComponent>(), getName(), "ConsumptionFraction", "");
	auto* propMax = new SimulationControlDouble(
			std::bind(&MetabolicSubmodelComponent::getAtpPoolMax, this),
			std::bind(&MetabolicSubmodelComponent::setAtpPoolMax, this, std::placeholders::_1),
			Util::TypeOf<MetabolicSubmodelComponent>(), getName(), "AtpPoolMax", "");
	auto* propDeltaT = new SimulationControlDouble(
			std::bind(&MetabolicSubmodelComponent::getDeltaT, this),
			std::bind(&MetabolicSubmodelComponent::setDeltaT, this, std::placeholders::_1),
			Util::TypeOf<MetabolicSubmodelComponent>(), getName(), "DeltaT", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propYield);
	_parentModel->getControls()->insert(propConsumption);
	_parentModel->getControls()->insert(propMax);
	_parentModel->getControls()->insert(propDeltaT);

	_addSimulationControl(propState);
	_addSimulationControl(propYield);
	_addSimulationControl(propConsumption);
	_addSimulationControl(propMax);
	_addSimulationControl(propDeltaT);
}

PluginInformation* MetabolicSubmodelComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MetabolicSubmodelComponent>(), &MetabolicSubmodelComponent::LoadInstance, &MetabolicSubmodelComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Minimal metabolic submodel that produces and consumes ATP each step. "
		"ATP production is proportional to cell volume and deltaT. "
		"Results are written to WholeCellState metabolite pool keys 'ATP' and 'ADP'. "
		"Replaces full FBA for Phase 4 integration; extend with MetabolicFluxBalance for full accuracy.");
	return info;
}

ModelComponent* MetabolicSubmodelComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	MetabolicSubmodelComponent* newComponent = new MetabolicSubmodelComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load MetabolicSubmodelComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string MetabolicSubmodelComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",atpYieldRate=" + std::to_string(_atpYieldRate) +
			",consumptionFraction=" + std::to_string(_consumptionFraction) +
			",atpPoolMax=" + std::to_string(_atpPoolMax) +
			",deltaT=" + std::to_string(_deltaT);
}

bool MetabolicSubmodelComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_atpYieldRate       = fields->loadField("atpYieldRate",       DEFAULT.atpYieldRate);
		_consumptionFraction = fields->loadField("consumptionFraction", DEFAULT.consumptionFraction);
		_atpPoolMax         = fields->loadField("atpPoolMax",         DEFAULT.atpPoolMax);
		_deltaT             = fields->loadField("deltaT",             DEFAULT.deltaT);
	}
	return res;
}

void MetabolicSubmodelComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState",     _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("atpYieldRate",       _atpYieldRate,       DEFAULT.atpYieldRate,       saveDefaultValues);
	fields->saveField("consumptionFraction", _consumptionFraction, DEFAULT.consumptionFraction, saveDefaultValues);
	fields->saveField("atpPoolMax",         _atpPoolMax,         DEFAULT.atpPoolMax,         saveDefaultValues);
	fields->saveField("deltaT",             _deltaT,             DEFAULT.deltaT,             saveDefaultValues);
}

bool MetabolicSubmodelComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "MetabolicSubmodelComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_atpYieldRate < 0.0) {
		errorMessage += "MetabolicSubmodelComponent \"" + getName() + "\" atpYieldRate must be >= 0. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void MetabolicSubmodelComponent::_initBetweenReplications() {
	// ATP pool is reset through WholeCellState::_initBetweenReplications.
}

void MetabolicSubmodelComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"MetabolicSubmodelComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	const double volume  = _wholeCellState->getCellVolume();  // fL
	const double curATP  = _wholeCellState->getMetaboliteAmount("ATP");
	const double curADP  = _wholeCellState->getMetaboliteAmount("ADP");

	const double produced  = _atpYieldRate * volume * _deltaT;
	const double consumed  = _consumptionFraction * curATP;

	const double newATP = std::min(_atpPoolMax, std::max(0.0, curATP + produced - consumed));
	const double newADP = std::max(0.0, curADP + consumed - produced);

	_wholeCellState->setMetaboliteAmount("ATP", newATP);
	_wholeCellState->setMetaboliteAmount("ADP", newADP);

	_forwardEntity(entity);
}

void MetabolicSubmodelComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void MetabolicSubmodelComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "MetabolicSubmodelComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void MetabolicSubmodelComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* MetabolicSubmodelComponent::getWholeCellState() const      { return _wholeCellState; }
void MetabolicSubmodelComponent::setAtpYieldRate(double rate)              { _atpYieldRate = rate; }
double MetabolicSubmodelComponent::getAtpYieldRate() const                 { return _atpYieldRate; }
void MetabolicSubmodelComponent::setConsumptionFraction(double frac)       { _consumptionFraction = frac; }
double MetabolicSubmodelComponent::getConsumptionFraction() const          { return _consumptionFraction; }
void MetabolicSubmodelComponent::setAtpPoolMax(double maxPool)             { _atpPoolMax = maxPool; }
double MetabolicSubmodelComponent::getAtpPoolMax() const                   { return _atpPoolMax; }
void MetabolicSubmodelComponent::setDeltaT(double dt)                      { _deltaT = dt; }
double MetabolicSubmodelComponent::getDeltaT() const                       { return _deltaT; }
