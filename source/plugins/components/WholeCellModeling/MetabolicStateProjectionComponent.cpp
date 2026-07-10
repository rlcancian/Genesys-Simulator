#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelComponentManager.h"
#include "kernel/simulator/ConnectionManager.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MetabolicStateProjectionComponent::GetPluginInformation;
}
#endif

namespace {

std::string projectionKindToString(MetabolicStateProjectionComponent::ProjectionKind kind) {
	switch (kind) {
		case MetabolicStateProjectionComponent::ProjectionKind::MetaboliteAmount:
			return "MetaboliteAmount";
		case MetabolicStateProjectionComponent::ProjectionKind::CompartmentMetaboliteAmount:
			return "CompartmentMetaboliteAmount";
		case MetabolicStateProjectionComponent::ProjectionKind::PathwayActivity:
			return "PathwayActivity";
	}
	return "MetaboliteAmount";
}

MetabolicStateProjectionComponent::ProjectionKind projectionKindFromString(const std::string& value) {
	if (value == "CompartmentMetaboliteAmount") {
		return MetabolicStateProjectionComponent::ProjectionKind::CompartmentMetaboliteAmount;
	}
	if (value == "PathwayActivity") {
		return MetabolicStateProjectionComponent::ProjectionKind::PathwayActivity;
	}
	return MetabolicStateProjectionComponent::ProjectionKind::MetaboliteAmount;
}

std::string projectionUpdateModeToString(MetabolicStateProjectionComponent::ProjectionUpdateMode mode) {
	switch (mode) {
		case MetabolicStateProjectionComponent::ProjectionUpdateMode::Overwrite:
			return "Overwrite";
		case MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate:
			return "Accumulate";
		case MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover:
			return "Turnover";
	}
	return "Overwrite";
}

MetabolicStateProjectionComponent::ProjectionUpdateMode projectionUpdateModeFromString(const std::string& value) {
	if (value == "Accumulate") {
		return MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate;
	}
	if (value == "Turnover") {
		return MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover;
	}
	return MetabolicStateProjectionComponent::ProjectionUpdateMode::Overwrite;
}

} // namespace

ModelDataDefinition* MetabolicStateProjectionComponent::NewInstance(Model* model, std::string name) {
	return new MetabolicStateProjectionComponent(model, name);
}

MetabolicStateProjectionComponent::MetabolicStateProjectionComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<MetabolicStateProjectionComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&MetabolicStateProjectionComponent::getWholeCellState, this),
			std::bind(&MetabolicStateProjectionComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<MetabolicStateProjectionComponent>(), getName(), "WholeCellState", "");
	auto* propFluxBalanceComponentName = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicStateProjectionComponent::getFluxBalanceComponentName, this),
			std::bind(&MetabolicStateProjectionComponent::setFluxBalanceComponentName, this, std::placeholders::_1),
			Util::TypeOf<MetabolicStateProjectionComponent>(), getName(), "FluxBalanceComponentName", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&MetabolicStateProjectionComponent::getLastSucceeded, this),
			std::bind(&MetabolicStateProjectionComponent::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<MetabolicStateProjectionComponent>(), getName(), "LastSucceeded", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicStateProjectionComponent::getLastMessage, this),
			std::bind(&MetabolicStateProjectionComponent::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<MetabolicStateProjectionComponent>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propFluxBalanceComponentName);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propState);
	_addSimulationControl(propFluxBalanceComponentName);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastMessage);
}

PluginInformation* MetabolicStateProjectionComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MetabolicStateProjectionComponent>(), &MetabolicStateProjectionComponent::LoadInstance, &MetabolicStateProjectionComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("wholecellstate.so");
	info->insertDynamicLibFileDependence("metabolicfluxbalance.so");
	info->setDescriptionHelp(
		"Projects selected MetabolicFluxBalance objective and flux values into WholeCellState as metabolite proxies, "
		"compartment-specific metabolite proxies, or pathway activity summaries.");
	return info;
}

ModelComponent* MetabolicStateProjectionComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	MetabolicStateProjectionComponent* newComponent = new MetabolicStateProjectionComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load MetabolicStateProjectionComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string MetabolicStateProjectionComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",fluxBalanceComponent=\"" + (_fluxBalanceComponent != nullptr ? _fluxBalanceComponent->getName() : _fluxBalanceComponentName) + "\"" +
			",objectiveProjection=" + std::string(_objectiveProjectionEnabled ? "true" : "false") +
			",projectionCount=" + std::to_string(_projections.size()) +
			",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0) +
			",lastMessage=\"" + _lastMessage + "\"";
}

bool MetabolicStateProjectionComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			ModelDataDefinition* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}

		_fluxBalanceComponentName = fields->loadField("fluxBalanceComponentName", DEFAULT.fluxBalanceComponentName);
		_fluxBalanceComponent = nullptr;
		_resolveFluxBalanceComponent();

		_objectiveProjectionEnabled = fields->loadField("objectiveProjectionEnabled", DEFAULT.objectiveProjectionEnabled ? 1u : 0u) != 0u;
		_objectiveProjection.targetKey = fields->loadField("objectiveTargetKey", DEFAULT.objectiveTargetKey);
		_objectiveProjection.kind = projectionKindFromString(fields->loadField("objectiveKind", DEFAULT.objectiveKind));
		_objectiveProjection.updateMode = projectionUpdateModeFromString(fields->loadField("objectiveUpdateMode", DEFAULT.objectiveUpdateMode));
		_objectiveProjection.scale = fields->loadField("objectiveScale", DEFAULT.objectiveScale);
		_objectiveProjection.offset = fields->loadField("objectiveOffset", DEFAULT.objectiveOffset);
		_objectiveProjection.turnoverFraction = fields->loadField("objectiveTurnoverFraction", DEFAULT.objectiveTurnoverFraction);
		_objectiveProjection.compartmentName = fields->loadField("objectiveCompartmentName", DEFAULT.objectiveCompartmentName);

		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);

		_projections.clear();
		const unsigned int projectionCount = fields->loadField("projectionCount", 0u);
		for (unsigned int i = 0; i < projectionCount; ++i) {
			Projection projection;
			projection.reactionName = fields->loadField("projectionReaction" + Util::StrIndex(i), "");
			projection.targetKey = fields->loadField("projectionTarget" + Util::StrIndex(i), "");
			projection.kind = projectionKindFromString(fields->loadField("projectionKind" + Util::StrIndex(i), "MetaboliteAmount"));
			projection.updateMode = projectionUpdateModeFromString(fields->loadField("projectionUpdateMode" + Util::StrIndex(i), "Overwrite"));
			projection.scale = fields->loadField("projectionScale" + Util::StrIndex(i), 1.0);
			projection.offset = fields->loadField("projectionOffset" + Util::StrIndex(i), 0.0);
			projection.turnoverFraction = fields->loadField("projectionTurnoverFraction" + Util::StrIndex(i), 0.0);
			projection.compartmentName = fields->loadField("projectionCompartment" + Util::StrIndex(i), "");
			_projections.push_back(std::move(projection));
		}
	}
	_syncEditableDataDefinitions();
	return res;
}

void MetabolicStateProjectionComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("fluxBalanceComponentName", _fluxBalanceComponent != nullptr ? _fluxBalanceComponent->getName() : _fluxBalanceComponentName, DEFAULT.fluxBalanceComponentName, saveDefaultValues);
	fields->saveField("objectiveProjectionEnabled", _objectiveProjectionEnabled ? 1u : 0u, DEFAULT.objectiveProjectionEnabled ? 1u : 0u, saveDefaultValues);
	fields->saveField("objectiveTargetKey", _objectiveProjection.targetKey, DEFAULT.objectiveTargetKey, saveDefaultValues);
	fields->saveField("objectiveKind", projectionKindToString(_objectiveProjection.kind), DEFAULT.objectiveKind, saveDefaultValues);
	fields->saveField("objectiveUpdateMode", projectionUpdateModeToString(_objectiveProjection.updateMode), DEFAULT.objectiveUpdateMode, saveDefaultValues);
	fields->saveField("objectiveScale", _objectiveProjection.scale, DEFAULT.objectiveScale, saveDefaultValues);
	fields->saveField("objectiveOffset", _objectiveProjection.offset, DEFAULT.objectiveOffset, saveDefaultValues);
	fields->saveField("objectiveTurnoverFraction", _objectiveProjection.turnoverFraction, DEFAULT.objectiveTurnoverFraction, saveDefaultValues);
	fields->saveField("objectiveCompartmentName", _objectiveProjection.compartmentName, DEFAULT.objectiveCompartmentName, saveDefaultValues);
	fields->saveField("projectionCount", static_cast<unsigned int>(_projections.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _projections.size(); ++i) {
		const Projection& projection = _projections[i];
		fields->saveField("projectionReaction" + Util::StrIndex(i), projection.reactionName, "", saveDefaultValues);
		fields->saveField("projectionTarget" + Util::StrIndex(i), projection.targetKey, "", saveDefaultValues);
		fields->saveField("projectionKind" + Util::StrIndex(i), projectionKindToString(projection.kind), "MetaboliteAmount", saveDefaultValues);
		fields->saveField("projectionUpdateMode" + Util::StrIndex(i), projectionUpdateModeToString(projection.updateMode), "Overwrite", saveDefaultValues);
		fields->saveField("projectionScale" + Util::StrIndex(i), projection.scale, 1.0, saveDefaultValues);
		fields->saveField("projectionOffset" + Util::StrIndex(i), projection.offset, 0.0, saveDefaultValues);
		fields->saveField("projectionTurnoverFraction" + Util::StrIndex(i), projection.turnoverFraction, 0.0, saveDefaultValues);
		fields->saveField("projectionCompartment" + Util::StrIndex(i), projection.compartmentName, "", saveDefaultValues);
	}
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool MetabolicStateProjectionComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	_resolveFluxBalanceComponent();
	if (_wholeCellState == nullptr) {
		errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_fluxBalanceComponent == nullptr) {
		errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" requires a MetabolicFluxBalance component reference. ";
		resultAll = false;
	}
	if (!_objectiveProjectionEnabled && _projections.empty()) {
		errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" requires an objective projection or at least one reaction flux projection. ";
		resultAll = false;
	}
	if (_objectiveProjectionEnabled && _objectiveProjection.targetKey.empty()) {
		errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" objective projection requires a non-empty target key. ";
		resultAll = false;
	}
	if (_objectiveProjectionEnabled && _objectiveProjection.updateMode == ProjectionUpdateMode::Turnover &&
	    (_objectiveProjection.turnoverFraction < 0.0 || _objectiveProjection.turnoverFraction > 1.0)) {
		errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" objective turnoverFraction must be within [0,1]. ";
		resultAll = false;
	}
	for (const Projection& projection : _projections) {
		if (projection.reactionName.empty()) {
			errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" has a flux projection with an empty reaction name. ";
			resultAll = false;
		}
		if (projection.targetKey.empty()) {
			errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" has a flux projection with an empty target key. ";
			resultAll = false;
		}
		if (projection.updateMode == ProjectionUpdateMode::Turnover &&
		    (projection.turnoverFraction < 0.0 || projection.turnoverFraction > 1.0)) {
			errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" has a flux projection with turnoverFraction outside [0,1]. ";
			resultAll = false;
		}
	}
	if (_fluxBalanceComponent != nullptr && _fluxBalanceComponent->getMetabolicNetwork() != nullptr) {
		const std::vector<std::string>& reactionNames = _fluxBalanceComponent->getMetabolicNetwork()->getReactionNames();
		for (const Projection& projection : _projections) {
			if (!projection.reactionName.empty() &&
			    std::find(reactionNames.begin(), reactionNames.end(), projection.reactionName) == reactionNames.end()) {
				errorMessage += "MetabolicStateProjectionComponent \"" + getName() + "\" references missing MetabolicReaction \"" + projection.reactionName + "\". ";
				resultAll = false;
			}
		}
	}
	_syncEditableDataDefinitions();
	return resultAll;
}

void MetabolicStateProjectionComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;

	_resolveFluxBalanceComponent();
	if (_wholeCellState == nullptr) {
		_lastSucceeded = false;
		_lastMessage = "MetabolicStateProjectionComponent requires a WholeCellState reference.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}
	if (_fluxBalanceComponent == nullptr) {
		_lastSucceeded = false;
		_lastMessage = "MetabolicStateProjectionComponent requires a MetabolicFluxBalance component reference.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}
	if (!_fluxBalanceComponent->getLastSucceeded()) {
		_lastSucceeded = false;
		_lastMessage = "MetabolicStateProjectionComponent requires a successful MetabolicFluxBalance result before projection.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	if (_objectiveProjectionEnabled) {
		_applyProjection(_objectiveProjection, _fluxBalanceComponent->getLastObjectiveValue());
	}

	const std::map<std::string, double>& fluxes = _fluxBalanceComponent->getLastFluxes();
	for (const Projection& projection : _projections) {
		auto it = fluxes.find(projection.reactionName);
		if (it == fluxes.end()) {
			_lastSucceeded = false;
			_lastMessage = "MetabolicStateProjectionComponent could not find projected flux for reaction \"" + projection.reactionName + "\".";
			traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
			_forwardEntity(entity);
			return;
		}
		_applyProjection(projection, it->second);
	}

	_lastSucceeded = true;
	_lastMessage = "MetabolicStateProjectionComponent projected " +
	               std::to_string(_projections.size() + (_objectiveProjectionEnabled ? 1u : 0u)) +
	               " FBA values.";
	traceSimulation(this, TraceManager::Level::L2_results, _lastMessage);
	_forwardEntity(entity);
}

void MetabolicStateProjectionComponent::_createEditableDataDefinitions() {
	_syncEditableDataDefinitions();
}

void MetabolicStateProjectionComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "MetabolicStateProjectionComponent dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void MetabolicStateProjectionComponent::_syncEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void MetabolicStateProjectionComponent::_resolveFluxBalanceComponent() {
	if (_fluxBalanceComponent == nullptr && !_fluxBalanceComponentName.empty()) {
		ModelComponent* component = _parentModel->getComponentManager()->find(_fluxBalanceComponentName);
		_fluxBalanceComponent = dynamic_cast<MetabolicFluxBalance*>(component);
	}
	if (_fluxBalanceComponent != nullptr) {
		_fluxBalanceComponentName = _fluxBalanceComponent->getName();
	}
}

void MetabolicStateProjectionComponent::_configureObjectiveProjection(ProjectionKind kind, const std::string& targetKey,
                                                                      const std::string& compartmentName, double scale, double offset,
                                                                      ProjectionUpdateMode updateMode, double turnoverFraction) {
	_objectiveProjectionEnabled = true;
	_objectiveProjection.reactionName.clear();
	_objectiveProjection.kind = kind;
	_objectiveProjection.targetKey = targetKey;
	_objectiveProjection.updateMode = updateMode;
	_objectiveProjection.scale = scale;
	_objectiveProjection.offset = offset;
	_objectiveProjection.turnoverFraction = turnoverFraction;
	_objectiveProjection.compartmentName = compartmentName;
}

void MetabolicStateProjectionComponent::_appendProjection(const Projection& projection) {
	_projections.push_back(projection);
}

void MetabolicStateProjectionComponent::_applyProjection(const Projection& projection, double value) {
	const double projectedValue = value * projection.scale + projection.offset;
	const double currentValue = _resolveCurrentValue(projection);
	double nextValue = projectedValue;
	switch (projection.updateMode) {
		case ProjectionUpdateMode::Overwrite:
			nextValue = projectedValue;
			break;
		case ProjectionUpdateMode::Accumulate:
			nextValue = currentValue + projectedValue;
			break;
		case ProjectionUpdateMode::Turnover:
			nextValue = currentValue * (1.0 - projection.turnoverFraction) + projectedValue;
			break;
	}
	switch (projection.kind) {
		case ProjectionKind::MetaboliteAmount:
			_wholeCellState->setMetaboliteAmount(projection.targetKey, std::max(0.0, nextValue));
			break;
		case ProjectionKind::CompartmentMetaboliteAmount:
			_wholeCellState->setCompartmentMetaboliteAmount(_resolveCompartmentName(projection), projection.targetKey, std::max(0.0, nextValue));
			break;
		case ProjectionKind::PathwayActivity:
			_wholeCellState->setPathwayActivity(projection.targetKey, nextValue);
			break;
	}
}

std::string MetabolicStateProjectionComponent::_resolveCompartmentName(const Projection& projection) const {
	if (!projection.compartmentName.empty()) {
		return projection.compartmentName;
	}
	if (_fluxBalanceComponent != nullptr && _fluxBalanceComponent->getMetabolicNetwork() != nullptr) {
		const std::string compartmentName = _fluxBalanceComponent->getMetabolicNetwork()->getCompartment();
		if (!compartmentName.empty()) {
			return compartmentName;
		}
	}
	return "default";
}

double MetabolicStateProjectionComponent::_resolveCurrentValue(const Projection& projection) const {
	switch (projection.kind) {
		case ProjectionKind::MetaboliteAmount:
			return _wholeCellState->getMetaboliteAmount(projection.targetKey);
		case ProjectionKind::CompartmentMetaboliteAmount:
			return _wholeCellState->getCompartmentMetaboliteAmount(_resolveCompartmentName(projection), projection.targetKey);
		case ProjectionKind::PathwayActivity:
			return _wholeCellState->getPathwayActivity(projection.targetKey);
	}
	return 0.0;
}

void MetabolicStateProjectionComponent::setWholeCellState(WholeCellState* state) {
	_wholeCellState = state;
	_syncEditableDataDefinitions();
}

WholeCellState* MetabolicStateProjectionComponent::getWholeCellState() const {
	return _wholeCellState;
}

void MetabolicStateProjectionComponent::setFluxBalanceComponent(MetabolicFluxBalance* component) {
	_fluxBalanceComponent = component;
	_fluxBalanceComponentName = component != nullptr ? component->getName() : std::string();
}

MetabolicFluxBalance* MetabolicStateProjectionComponent::getFluxBalanceComponent() const {
	return _fluxBalanceComponent;
}

void MetabolicStateProjectionComponent::setFluxBalanceComponentName(std::string componentName) {
	_fluxBalanceComponentName = std::move(componentName);
	_fluxBalanceComponent = nullptr;
	_resolveFluxBalanceComponent();
}

std::string MetabolicStateProjectionComponent::getFluxBalanceComponentName() const {
	return _fluxBalanceComponent != nullptr ? _fluxBalanceComponent->getName() : _fluxBalanceComponentName;
}

void MetabolicStateProjectionComponent::clearObjectiveProjection() {
	_objectiveProjectionEnabled = false;
	_objectiveProjection = Projection{};
}

void MetabolicStateProjectionComponent::setObjectiveAsMetabolite(const std::string& targetKey, double scale, double offset,
                                                                 ProjectionUpdateMode updateMode, double turnoverFraction) {
	_configureObjectiveProjection(ProjectionKind::MetaboliteAmount, targetKey, "", scale, offset, updateMode, turnoverFraction);
}

void MetabolicStateProjectionComponent::setObjectiveAsCompartmentMetabolite(const std::string& compartmentName,
                                                                            const std::string& targetKey, double scale, double offset,
                                                                            ProjectionUpdateMode updateMode, double turnoverFraction) {
	_configureObjectiveProjection(ProjectionKind::CompartmentMetaboliteAmount, targetKey, compartmentName, scale, offset, updateMode, turnoverFraction);
}

void MetabolicStateProjectionComponent::setObjectiveAsPathwayActivity(const std::string& targetKey, double scale, double offset,
                                                                      ProjectionUpdateMode updateMode, double turnoverFraction) {
	_configureObjectiveProjection(ProjectionKind::PathwayActivity, targetKey, "", scale, offset, updateMode, turnoverFraction);
}

bool MetabolicStateProjectionComponent::hasObjectiveProjection() const {
	return _objectiveProjectionEnabled;
}

void MetabolicStateProjectionComponent::clearFluxProjections() {
	_projections.clear();
}

void MetabolicStateProjectionComponent::addMetaboliteProjection(const std::string& reactionName, const std::string& targetKey,
                                                                double scale, double offset, ProjectionUpdateMode updateMode,
                                                                double turnoverFraction) {
	_appendProjection(Projection{reactionName, targetKey, ProjectionKind::MetaboliteAmount, updateMode, scale, offset, turnoverFraction, ""});
}

void MetabolicStateProjectionComponent::addCompartmentMetaboliteProjection(const std::string& reactionName,
                                                                           const std::string& compartmentName, const std::string& targetKey,
                                                                           double scale, double offset, ProjectionUpdateMode updateMode,
                                                                           double turnoverFraction) {
	_appendProjection(Projection{reactionName, targetKey, ProjectionKind::CompartmentMetaboliteAmount, updateMode, scale, offset, turnoverFraction, compartmentName});
}

void MetabolicStateProjectionComponent::addPathwayProjection(const std::string& reactionName, const std::string& targetKey,
                                                             double scale, double offset, ProjectionUpdateMode updateMode,
                                                             double turnoverFraction) {
	_appendProjection(Projection{reactionName, targetKey, ProjectionKind::PathwayActivity, updateMode, scale, offset, turnoverFraction, ""});
}

unsigned int MetabolicStateProjectionComponent::getProjectionCount() const {
	return static_cast<unsigned int>(_projections.size());
}

void MetabolicStateProjectionComponent::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool MetabolicStateProjectionComponent::getLastSucceeded() const {
	return _lastSucceeded;
}

void MetabolicStateProjectionComponent::setLastMessage(std::string lastMessage) {
	_lastMessage = std::move(lastMessage);
}

std::string MetabolicStateProjectionComponent::getLastMessage() const {
	return _lastMessage;
}
