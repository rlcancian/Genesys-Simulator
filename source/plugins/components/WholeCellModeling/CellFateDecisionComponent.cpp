#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"

#include <functional>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellFateDecisionComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* CellFateDecisionComponent::NewInstance(Model* model, std::string name) {
	return new CellFateDecisionComponent(model, name);
}

CellFateDecisionComponent::CellFateDecisionComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<CellFateDecisionComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&CellFateDecisionComponent::getWholeCellState, this),
			std::bind(&CellFateDecisionComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "WholeCellState", "");
	auto* propDivisionReadyPhase = new SimulationControlString(
			std::bind(&CellFateDecisionComponent::getDivisionReadyPhase, this),
			std::bind(&CellFateDecisionComponent::setDivisionReadyPhase, this, std::placeholders::_1),
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "DivisionReadyPhase", "");
	auto* propStarvedPhase = new SimulationControlString(
			std::bind(&CellFateDecisionComponent::getStarvedPhase, this),
			std::bind(&CellFateDecisionComponent::setStarvedPhase, this, std::placeholders::_1),
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "StarvedPhase", "");
	auto* propArrestedPhase = new SimulationControlString(
			std::bind(&CellFateDecisionComponent::getArrestedPhase, this),
			std::bind(&CellFateDecisionComponent::setArrestedPhase, this, std::placeholders::_1),
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "ArrestedPhase", "");
	auto* propDeadPhase = new SimulationControlString(
			std::bind(&CellFateDecisionComponent::getDeadPhase, this),
			std::bind(&CellFateDecisionComponent::setDeadPhase, this, std::placeholders::_1),
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "DeadPhase", "");
	auto* propLastRoutedPort = new SimulationControlUInt(
			std::bind(&CellFateDecisionComponent::getLastRoutedPort, this),
			nullptr,
			Util::TypeOf<CellFateDecisionComponent>(), getName(), "LastRoutedPort", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propDivisionReadyPhase);
	_parentModel->getControls()->insert(propStarvedPhase);
	_parentModel->getControls()->insert(propArrestedPhase);
	_parentModel->getControls()->insert(propDeadPhase);
	_parentModel->getControls()->insert(propLastRoutedPort);

	_addSimulationControl(propState);
	_addSimulationControl(propDivisionReadyPhase);
	_addSimulationControl(propStarvedPhase);
	_addSimulationControl(propArrestedPhase);
	_addSimulationControl(propDeadPhase);
	_addSimulationControl(propLastRoutedPort);
}

PluginInformation* CellFateDecisionComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellFateDecisionComponent>(), &CellFateDecisionComponent::LoadInstance, &CellFateDecisionComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(4);
	info->setDescriptionHelp(
		"Routes entities according to whole-cell lifecycle state. "
		"Port 0: viable default flow; port 1: division_ready; port 2: starved; port 3: dead.");
	return info;
}

ModelComponent* CellFateDecisionComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	CellFateDecisionComponent* newComponent = new CellFateDecisionComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CellFateDecisionComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string CellFateDecisionComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",divisionReadyPhase=\"" + _divisionReadyPhase + "\"" +
			",starvedPhase=\"" + _starvedPhase + "\"" +
			",arrestedPhase=\"" + _arrestedPhase + "\"" +
			",deadPhase=\"" + _deadPhase + "\"" +
			",lastRoutedPort=" + std::to_string(_lastRoutedPort);
}

bool CellFateDecisionComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_divisionReadyPhase = fields->loadField("divisionReadyPhase", DEFAULT.divisionReadyPhase);
		_starvedPhase = fields->loadField("starvedPhase", DEFAULT.starvedPhase);
		_arrestedPhase = fields->loadField("arrestedPhase", DEFAULT.arrestedPhase);
		_deadPhase = fields->loadField("deadPhase", DEFAULT.deadPhase);
	}
	return res;
}

void CellFateDecisionComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("divisionReadyPhase", _divisionReadyPhase, DEFAULT.divisionReadyPhase, saveDefaultValues);
	fields->saveField("starvedPhase", _starvedPhase, DEFAULT.starvedPhase, saveDefaultValues);
	fields->saveField("arrestedPhase", _arrestedPhase, DEFAULT.arrestedPhase, saveDefaultValues);
	fields->saveField("deadPhase", _deadPhase, DEFAULT.deadPhase, saveDefaultValues);
}

bool CellFateDecisionComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "CellFateDecisionComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_divisionReadyPhase.empty()) {
		errorMessage += "CellFateDecisionComponent \"" + getName() + "\" divisionReadyPhase must be non-empty. ";
		resultAll = false;
	}
	if (_starvedPhase.empty()) {
		errorMessage += "CellFateDecisionComponent \"" + getName() + "\" starvedPhase must be non-empty. ";
		resultAll = false;
	}
	if (_arrestedPhase.empty()) {
		errorMessage += "CellFateDecisionComponent \"" + getName() + "\" arrestedPhase must be non-empty. ";
		resultAll = false;
	}
	if (_deadPhase.empty()) {
		errorMessage += "CellFateDecisionComponent \"" + getName() + "\" deadPhase must be non-empty. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void CellFateDecisionComponent::_initBetweenReplications() {
	_lastRoutedPort = DEFAULT.lastRoutedPort;
}

void CellFateDecisionComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"CellFateDecisionComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity, 0u);
		return;
	}

	_lastRoutedPort = _selectOutputPort();
	_forwardEntity(entity, _lastRoutedPort);
}

void CellFateDecisionComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

unsigned int CellFateDecisionComponent::_selectOutputPort() const {
	if (!_wholeCellState->isViable() || _wholeCellState->getLifecyclePhase() == _deadPhase) {
		return 3u;
	}
	if (_wholeCellState->getLifecyclePhase() == _divisionReadyPhase) {
		return 1u;
	}
	if (_wholeCellState->getLifecyclePhase() == _starvedPhase || _wholeCellState->getLifecyclePhase() == _arrestedPhase) {
		return 2u;
	}
	return 0u;
}

void CellFateDecisionComponent::_forwardEntity(Entity* entity, unsigned int port) {
	if (entity == nullptr) return;

	Connection* conn = this->getConnectionManager()->getConnectionAtPort(port);
	if (conn == nullptr || conn->component == nullptr) {
		conn = this->getConnectionManager()->getConnectionAtPort(0u);
		if (conn == nullptr || conn->component == nullptr) {
			traceSimulation(this, "CellFateDecisionComponent: no valid output connection, entity removed.");
			_parentModel->removeEntity(entity);
			return;
		}
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void CellFateDecisionComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* CellFateDecisionComponent::getWholeCellState() const { return _wholeCellState; }
void CellFateDecisionComponent::setDivisionReadyPhase(std::string phase) { _divisionReadyPhase = std::move(phase); }
std::string CellFateDecisionComponent::getDivisionReadyPhase() const { return _divisionReadyPhase; }
void CellFateDecisionComponent::setStarvedPhase(std::string phase) { _starvedPhase = std::move(phase); }
std::string CellFateDecisionComponent::getStarvedPhase() const { return _starvedPhase; }
void CellFateDecisionComponent::setArrestedPhase(std::string phase) { _arrestedPhase = std::move(phase); }
std::string CellFateDecisionComponent::getArrestedPhase() const { return _arrestedPhase; }
void CellFateDecisionComponent::setDeadPhase(std::string phase) { _deadPhase = std::move(phase); }
std::string CellFateDecisionComponent::getDeadPhase() const { return _deadPhase; }
unsigned int CellFateDecisionComponent::getLastRoutedPort() const { return _lastRoutedPort; }
