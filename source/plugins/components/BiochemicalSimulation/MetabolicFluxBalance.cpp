#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MetabolicFluxBalance::GetPluginInformation;
}
#endif

ModelDataDefinition* MetabolicFluxBalance::NewInstance(Model* model, std::string name) {
	return new MetabolicFluxBalance(model, name);
}

MetabolicFluxBalance::MetabolicFluxBalance(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<MetabolicFluxBalance>(), name) {
	auto* propMetabolicNetwork = new SimulationControlGenericClass<MetabolicNetwork*, Model*, MetabolicNetwork>(
			_parentModel,
			std::bind(&MetabolicFluxBalance::getMetabolicNetwork, this),
			std::bind(&MetabolicFluxBalance::setMetabolicNetwork, this, std::placeholders::_1),
			Util::TypeOf<MetabolicFluxBalance>(), getName(), "MetabolicNetwork", "");
	auto* propObjectiveReactionName = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicFluxBalance::getObjectiveReactionName, this),
			std::bind(&MetabolicFluxBalance::setObjectiveReactionName, this, std::placeholders::_1),
			Util::TypeOf<MetabolicFluxBalance>(), getName(), "ObjectiveReactionName", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&MetabolicFluxBalance::getLastSucceeded, this),
			std::bind(&MetabolicFluxBalance::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<MetabolicFluxBalance>(), getName(), "LastSucceeded", "");
	auto* propLastObjectiveValue = new SimulationControlDouble(
			std::bind(&MetabolicFluxBalance::getLastObjectiveValue, this),
			std::bind(&MetabolicFluxBalance::setLastObjectiveValue, this, std::placeholders::_1),
			Util::TypeOf<MetabolicFluxBalance>(), getName(), "LastObjectiveValue", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicFluxBalance::getLastMessage, this),
			std::bind(&MetabolicFluxBalance::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<MetabolicFluxBalance>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propMetabolicNetwork);
	_parentModel->getControls()->insert(propObjectiveReactionName);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastObjectiveValue);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propMetabolicNetwork);
	_addSimulationControl(propObjectiveReactionName);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastObjectiveValue);
	_addSimulationControl(propLastMessage);
}

PluginInformation* MetabolicFluxBalance::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MetabolicFluxBalance>(), &MetabolicFluxBalance::LoadInstance, &MetabolicFluxBalance::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("metabolicnetwork.so");
	info->setDescriptionHelp("Evaluates a stub flux-balance objective over a MetabolicNetwork and stores the selected objective value for downstream logic.");
	return info;
}

ModelComponent* MetabolicFluxBalance::LoadInstance(Model* model, PersistenceRecord* fields) {
	MetabolicFluxBalance* newComponent = new MetabolicFluxBalance(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string MetabolicFluxBalance::show() {
	return ModelComponent::show() +
	       ",metabolicNetwork=\"" + (_metabolicNetwork != nullptr ? _metabolicNetwork->getName() : std::string()) + "\"" +
	       ",objectiveReactionName=\"" + _objectiveReactionName + "\"" +
	       ",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0) +
	       ",lastObjectiveValue=" + Util::StrTruncIfInt(std::to_string(_lastObjectiveValue));
}

bool MetabolicFluxBalance::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string metabolicNetworkName = fields->loadField("metabolicNetwork", DEFAULT.metabolicNetworkName);
		_metabolicNetwork = nullptr;
		if (!metabolicNetworkName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicNetwork>(), metabolicNetworkName);
			_metabolicNetwork = dynamic_cast<MetabolicNetwork*>(definition);
		}
		_objectiveReactionName = fields->loadField("objectiveReactionName", DEFAULT.objectiveReactionName);
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastObjectiveValue = fields->loadField("lastObjectiveValue", DEFAULT.lastObjectiveValue);
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void MetabolicFluxBalance::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("metabolicNetwork", _metabolicNetwork != nullptr ? _metabolicNetwork->getName() : DEFAULT.metabolicNetworkName, DEFAULT.metabolicNetworkName, saveDefaultValues);
	fields->saveField("objectiveReactionName", _objectiveReactionName, DEFAULT.objectiveReactionName, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastObjectiveValue", _lastObjectiveValue, DEFAULT.lastObjectiveValue, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool MetabolicFluxBalance::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<MetabolicNetwork>(), _metabolicNetwork, "MetabolicNetwork", errorMessage);
	if (_metabolicNetwork != nullptr) {
		const std::string selectedObjective = _objectiveReactionName.empty() ? _metabolicNetwork->getObjectiveReactionName() : _objectiveReactionName;
		if (selectedObjective.empty()) {
			errorMessage += "MetabolicFluxBalance \"" + getName() + "\" requires an objective reaction name either on the component or MetabolicNetwork. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void MetabolicFluxBalance::_createInternalAndAttachedData() {
	if (_metabolicNetwork != nullptr) {
		_attachedDataInsert("MetabolicNetwork", _metabolicNetwork);
	} else {
		_attachedDataRemove("MetabolicNetwork");
	}
}

void MetabolicFluxBalance::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_metabolicNetwork == nullptr) {
		_lastSucceeded = false;
		_lastObjectiveValue = 0.0;
		_lastMessage = "MetabolicFluxBalance requires a referenced MetabolicNetwork.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	const std::string selectedObjective = _objectiveReactionName.empty() ? _metabolicNetwork->getObjectiveReactionName() : _objectiveReactionName;
	auto* objectiveReaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), selectedObjective));
	if (objectiveReaction == nullptr) {
		_lastSucceeded = false;
		_lastObjectiveValue = 0.0;
		_lastMessage = "MetabolicFluxBalance could not resolve objective MetabolicReaction \"" + selectedObjective + "\".";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	if (_metabolicNetwork->getObjectiveSense() == "Minimize") {
		_lastObjectiveValue = objectiveReaction->getLowerBound();
	} else {
		_lastObjectiveValue = objectiveReaction->getUpperBound();
	}

	_lastSucceeded = true;
	_lastMessage = "MetabolicFluxBalance evaluated MetabolicNetwork \"" + _metabolicNetwork->getName() + "\".";
	traceSimulation(this, TraceManager::Level::L2_results,
	                "MetabolicFluxBalance evaluated objective \"" + selectedObjective +
	                "\" value=" + Util::StrTruncIfInt(std::to_string(_lastObjectiveValue)));
	_forwardEntity(entity);
}

void MetabolicFluxBalance::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "MetabolicFluxBalance dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void MetabolicFluxBalance::setMetabolicNetwork(MetabolicNetwork* metabolicNetwork) {
	_metabolicNetwork = metabolicNetwork;
}

MetabolicNetwork* MetabolicFluxBalance::getMetabolicNetwork() const {
	return _metabolicNetwork;
}

void MetabolicFluxBalance::setObjectiveReactionName(std::string objectiveReactionName) {
	_objectiveReactionName = objectiveReactionName;
}

std::string MetabolicFluxBalance::getObjectiveReactionName() const {
	return _objectiveReactionName;
}

void MetabolicFluxBalance::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool MetabolicFluxBalance::getLastSucceeded() const {
	return _lastSucceeded;
}

void MetabolicFluxBalance::setLastObjectiveValue(double lastObjectiveValue) {
	_lastObjectiveValue = lastObjectiveValue;
}

double MetabolicFluxBalance::getLastObjectiveValue() const {
	return _lastObjectiveValue;
}

void MetabolicFluxBalance::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string MetabolicFluxBalance::getLastMessage() const {
	return _lastMessage;
}

void MetabolicFluxBalance::_createReportStatisticsDataDefinitions() {
}

void MetabolicFluxBalance::_createEditableDataDefinitions() {
}

void MetabolicFluxBalance::_createOthersDataDefinitions() {
}
