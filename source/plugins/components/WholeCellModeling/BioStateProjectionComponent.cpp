#include "plugins/components/WholeCellModeling/BioStateProjectionComponent.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <set>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioStateProjectionComponent::GetPluginInformation;
}
#endif

namespace {

std::string projectionKindToString(BioStateProjectionComponent::ProjectionKind kind) {
	switch (kind) {
		case BioStateProjectionComponent::ProjectionKind::MoleculeCount:
			return "MoleculeCount";
		case BioStateProjectionComponent::ProjectionKind::MetaboliteAmount:
			return "MetaboliteAmount";
	}
	return "MoleculeCount";
}

BioStateProjectionComponent::ProjectionKind projectionKindFromString(const std::string& value) {
	if (value == "MetaboliteAmount") {
		return BioStateProjectionComponent::ProjectionKind::MetaboliteAmount;
	}
	return BioStateProjectionComponent::ProjectionKind::MoleculeCount;
}

} // namespace

ModelDataDefinition* BioStateProjectionComponent::NewInstance(Model* model, std::string name) {
	return new BioStateProjectionComponent(model, name);
}

BioStateProjectionComponent::BioStateProjectionComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<BioStateProjectionComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&BioStateProjectionComponent::getWholeCellState, this),
			std::bind(&BioStateProjectionComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<BioStateProjectionComponent>(), getName(), "WholeCellState", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&BioStateProjectionComponent::getLastSucceeded, this),
			std::bind(&BioStateProjectionComponent::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<BioStateProjectionComponent>(), getName(), "LastSucceeded", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioStateProjectionComponent::getLastMessage, this),
			std::bind(&BioStateProjectionComponent::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<BioStateProjectionComponent>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propState);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastMessage);
}

PluginInformation* BioStateProjectionComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioStateProjectionComponent>(), &BioStateProjectionComponent::LoadInstance, &BioStateProjectionComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("wholecellstate.so");
	info->insertDynamicLibFileDependence("biospecies.so");
	info->setDescriptionHelp(
		"Projects selected BioSpecies values into a WholeCellState after biochemical simulation. "
		"Each projection can write either a molecule count or a metabolite amount using a linear "
		"scale/offset transform. This component is the runtime bridge between deterministic "
		"BioNetwork output and whole-cell stochastic components.");
	return info;
}

ModelComponent* BioStateProjectionComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	BioStateProjectionComponent* newComponent = new BioStateProjectionComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load BioStateProjectionComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string BioStateProjectionComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",projectionCount=" + std::to_string(_projections.size()) +
			",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0) +
			",lastMessage=\"" + _lastMessage + "\"";
}

bool BioStateProjectionComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
		_projections.clear();
		const unsigned int projectionCount = fields->loadField("projectionCount", 0u);
		for (unsigned int i = 0; i < projectionCount; ++i) {
			Projection projection;
			projection.sourceSpeciesName = fields->loadField("projectionSource" + Util::StrIndex(i), "");
			projection.targetKey = fields->loadField("projectionTarget" + Util::StrIndex(i), "");
			projection.kind = projectionKindFromString(fields->loadField("projectionKind" + Util::StrIndex(i), "MoleculeCount"));
			projection.scale = fields->loadField("projectionScale" + Util::StrIndex(i), 1.0);
			projection.offset = fields->loadField("projectionOffset" + Util::StrIndex(i), 0.0);
			_projections.push_back(std::move(projection));
		}
	}
	_syncEditableDataDefinitions();
	return res;
}

void BioStateProjectionComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("projectionCount", static_cast<unsigned int>(_projections.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _projections.size(); ++i) {
		const Projection& projection = _projections[i];
		fields->saveField("projectionSource" + Util::StrIndex(i), projection.sourceSpeciesName, "", saveDefaultValues);
		fields->saveField("projectionTarget" + Util::StrIndex(i), projection.targetKey, "", saveDefaultValues);
		fields->saveField("projectionKind" + Util::StrIndex(i), projectionKindToString(projection.kind), "MoleculeCount", saveDefaultValues);
		fields->saveField("projectionScale" + Util::StrIndex(i), projection.scale, 1.0, saveDefaultValues);
		fields->saveField("projectionOffset" + Util::StrIndex(i), projection.offset, 0.0, saveDefaultValues);
	}
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool BioStateProjectionComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "BioStateProjectionComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_projections.empty()) {
		errorMessage += "BioStateProjectionComponent \"" + getName() + "\" requires at least one projection. ";
		resultAll = false;
	}
	for (const Projection& projection : _projections) {
		if (projection.sourceSpeciesName.empty()) {
			errorMessage += "BioStateProjectionComponent \"" + getName() + "\" has a projection with an empty source species name. ";
			resultAll = false;
			continue;
		}
		ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), projection.sourceSpeciesName);
		if (def == nullptr) {
			errorMessage += "BioStateProjectionComponent \"" + getName() + "\" references missing BioSpecies \"" + projection.sourceSpeciesName + "\". ";
			resultAll = false;
		}
	}
	_syncEditableDataDefinitions();
	return resultAll;
}

void BioStateProjectionComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		_lastSucceeded = false;
		_lastMessage = "BioStateProjectionComponent requires a WholeCellState reference.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}
	if (_projections.empty()) {
		_lastSucceeded = false;
		_lastMessage = "BioStateProjectionComponent requires at least one projection.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	ModelDataManager* dataManager = _parentModel->getDataManager();
	for (const Projection& projection : _projections) {
		auto* source = dynamic_cast<BioSpecies*>(dataManager->getDataDefinition(Util::TypeOf<BioSpecies>(), projection.sourceSpeciesName));
		if (source == nullptr) {
			_lastSucceeded = false;
			_lastMessage = "BioStateProjectionComponent missing BioSpecies \"" + projection.sourceSpeciesName + "\".";
			traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
			_forwardEntity(entity);
			return;
		}
		const double rawValue = source->getAmount() * projection.scale + projection.offset;
		const double projectedValue = std::max(0.0, rawValue);
		const std::string targetKey = projection.targetKey.empty() ? projection.sourceSpeciesName : projection.targetKey;
		if (projection.kind == ProjectionKind::MoleculeCount) {
			const int count = static_cast<int>(std::llround(projectedValue));
			_wholeCellState->setMoleculeCount(targetKey, std::max(0, count));
		} else {
			_wholeCellState->setMetaboliteAmount(targetKey, projectedValue);
		}
	}

	_lastSucceeded = true;
	_lastMessage = "BioStateProjectionComponent projected " + std::to_string(_projections.size()) + " values.";
	traceSimulation(this, TraceManager::Level::L2_results, _lastMessage);
	_forwardEntity(entity);
}

void BioStateProjectionComponent::_createEditableDataDefinitions() {
	_syncEditableDataDefinitions();
}

void BioStateProjectionComponent::setWholeCellState(WholeCellState* state) {
	_wholeCellState = state;
	_syncEditableDataDefinitions();
}

WholeCellState* BioStateProjectionComponent::getWholeCellState() const {
	return _wholeCellState;
}

void BioStateProjectionComponent::clearProjections() {
	_projections.clear();
	_syncEditableDataDefinitions();
}

void BioStateProjectionComponent::addMoleculeProjection(const std::string& sourceSpeciesName, const std::string& targetKey,
		double scale, double offset) {
	Projection projection;
	projection.sourceSpeciesName = sourceSpeciesName;
	projection.targetKey = targetKey.empty() ? sourceSpeciesName : targetKey;
	projection.kind = ProjectionKind::MoleculeCount;
	projection.scale = scale;
	projection.offset = offset;
	_projections.push_back(std::move(projection));
	_syncEditableDataDefinitions();
}

void BioStateProjectionComponent::addMetaboliteProjection(const std::string& sourceSpeciesName, const std::string& targetKey,
		double scale, double offset) {
	Projection projection;
	projection.sourceSpeciesName = sourceSpeciesName;
	projection.targetKey = targetKey.empty() ? sourceSpeciesName : targetKey;
	projection.kind = ProjectionKind::MetaboliteAmount;
	projection.scale = scale;
	projection.offset = offset;
	_projections.push_back(std::move(projection));
	_syncEditableDataDefinitions();
}

unsigned int BioStateProjectionComponent::getProjectionCount() const {
	return static_cast<unsigned int>(_projections.size());
}

void BioStateProjectionComponent::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool BioStateProjectionComponent::getLastSucceeded() const {
	return _lastSucceeded;
}

void BioStateProjectionComponent::setLastMessage(std::string lastMessage) {
	_lastMessage = std::move(lastMessage);
}

std::string BioStateProjectionComponent::getLastMessage() const {
	return _lastMessage;
}

void BioStateProjectionComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* conn = this->getConnectionManager()->getFrontConnection();
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "BioStateProjectionComponent dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void BioStateProjectionComponent::_syncEditableDataDefinitions() {
	_optionalEditableDataDefinitionsClear();
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	}
	std::set<std::string> uniqueSources;
	for (const Projection& projection : _projections) {
		if (!projection.sourceSpeciesName.empty()) {
			uniqueSources.insert(projection.sourceSpeciesName);
		}
	}
	unsigned int index = 0;
	ModelDataManager* dataManager = _parentModel != nullptr ? _parentModel->getDataManager() : nullptr;
	if (dataManager != nullptr) {
		for (const std::string& sourceName : uniqueSources) {
			ModelDataDefinition* def = dataManager->getDataDefinition(Util::TypeOf<BioSpecies>(), sourceName);
			if (def != nullptr) {
				_optionalEditableDataDefinitionInsert("SourceSpecies" + Util::StrIndex(index), def);
			}
			++index;
		}
	}
}
