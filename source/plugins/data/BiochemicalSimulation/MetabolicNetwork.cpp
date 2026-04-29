#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MetabolicNetwork::GetPluginInformation;
}
#endif

namespace {

std::string namesToString(const std::vector<std::string>& names) {
	std::string text = "{";
	for (const std::string& name : names) {
		text += name + ",";
	}
	if (!names.empty()) {
		text.pop_back();
	}
	text += "}";
	return text;
}

} // namespace

ModelDataDefinition* MetabolicNetwork::NewInstance(Model* model, std::string name) {
	return new MetabolicNetwork(model, name);
}

MetabolicNetwork::MetabolicNetwork(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<MetabolicNetwork>(), name) {
	auto* propObjectiveReactionName = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicNetwork::getObjectiveReactionName, this), std::bind(&MetabolicNetwork::setObjectiveReactionName, this, std::placeholders::_1),
			Util::TypeOf<MetabolicNetwork>(), getName(), "ObjectiveReactionName", "");
	auto* propObjectiveSense = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicNetwork::getObjectiveSense, this), std::bind(&MetabolicNetwork::setObjectiveSense, this, std::placeholders::_1),
			Util::TypeOf<MetabolicNetwork>(), getName(), "ObjectiveSense", "");
	auto* propCompartment = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicNetwork::getCompartment, this), std::bind(&MetabolicNetwork::setCompartment, this, std::placeholders::_1),
			Util::TypeOf<MetabolicNetwork>(), getName(), "Compartment", "");
	auto* propEnabled = new SimulationControlGeneric<bool>(
			std::bind(&MetabolicNetwork::isEnabled, this), std::bind(&MetabolicNetwork::setEnabled, this, std::placeholders::_1),
			Util::TypeOf<MetabolicNetwork>(), getName(), "Enabled", "");

	_parentModel->getControls()->insert(propObjectiveReactionName);
	_parentModel->getControls()->insert(propObjectiveSense);
	_parentModel->getControls()->insert(propCompartment);
	_parentModel->getControls()->insert(propEnabled);

	_addSimulationControl(propObjectiveReactionName);
	_addSimulationControl(propObjectiveSense);
	_addSimulationControl(propCompartment);
	_addSimulationControl(propEnabled);
}

PluginInformation* MetabolicNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MetabolicNetwork>(), &MetabolicNetwork::LoadInstance, &MetabolicNetwork::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Aggregate metabolic network definition with reaction membership, exchange species, and objective metadata.");
	return info;
}

ModelDataDefinition* MetabolicNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	MetabolicNetwork* newElement = new MetabolicNetwork(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string MetabolicNetwork::show() {
	return ModelDataDefinition::show() +
	       ",reactions=" + namesToString(_reactionNames) +
	       ",exchangeSpecies=" + namesToString(_exchangeSpeciesNames) +
	       ",objectiveReactionName=\"" + _objectiveReactionName + "\"" +
	       ",objectiveSense=\"" + _objectiveSense + "\"" +
	       ",compartment=\"" + _compartment + "\"" +
	       ",enabled=" + std::to_string(_enabled ? 1 : 0);
}

bool MetabolicNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_reactionNames.clear();
		_exchangeSpeciesNames.clear();
		const unsigned int reactionCount = fields->loadField("reactions", 0u);
		for (unsigned int i = 0; i < reactionCount; ++i) {
			addReaction(fields->loadField("reactionName" + Util::StrIndex(i), ""));
		}
		const unsigned int exchangeCount = fields->loadField("exchangeSpecies", 0u);
		for (unsigned int i = 0; i < exchangeCount; ++i) {
			addExchangeSpecies(fields->loadField("exchangeSpeciesName" + Util::StrIndex(i), ""));
		}
		_objectiveReactionName = fields->loadField("objectiveReactionName", DEFAULT.objectiveReactionName);
		_objectiveSense = fields->loadField("objectiveSense", DEFAULT.objectiveSense);
		_compartment = fields->loadField("compartment", DEFAULT.compartment);
		_enabled = fields->loadField("enabled", DEFAULT.enabled ? 1u : 0u) != 0u;
	}
	return res;
}

void MetabolicNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("reactions", static_cast<unsigned int>(_reactionNames.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _reactionNames.size(); ++i) {
		fields->saveField("reactionName" + Util::StrIndex(i), _reactionNames[i], "", saveDefaultValues);
	}
	fields->saveField("exchangeSpecies", static_cast<unsigned int>(_exchangeSpeciesNames.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _exchangeSpeciesNames.size(); ++i) {
		fields->saveField("exchangeSpeciesName" + Util::StrIndex(i), _exchangeSpeciesNames[i], "", saveDefaultValues);
	}
	fields->saveField("objectiveReactionName", _objectiveReactionName, DEFAULT.objectiveReactionName, saveDefaultValues);
	fields->saveField("objectiveSense", _objectiveSense, DEFAULT.objectiveSense, saveDefaultValues);
	fields->saveField("compartment", _compartment, DEFAULT.compartment, saveDefaultValues);
	fields->saveField("enabled", _enabled ? 1u : 0u, DEFAULT.enabled ? 1u : 0u, saveDefaultValues);
}

bool MetabolicNetwork::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "MetabolicNetwork must define a non-empty name. ";
		resultAll = false;
	}
	if (_reactionNames.empty()) {
		errorMessage += "MetabolicNetwork \"" + getName() + "\" must define at least one reaction. ";
		resultAll = false;
	}
	resultAll = _checkReactionNames(errorMessage) && resultAll;
	resultAll = _checkExchangeSpeciesNames(errorMessage) && resultAll;
	if (!_objectiveReactionName.empty()) {
		auto* reaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), _objectiveReactionName));
		if (reaction == nullptr) {
			errorMessage += "MetabolicNetwork \"" + getName() + "\" references missing objective MetabolicReaction \"" + _objectiveReactionName + "\". ";
			resultAll = false;
		}
	}
	if (_objectiveSense != "Maximize" && _objectiveSense != "Minimize") {
		errorMessage += "MetabolicNetwork \"" + getName() + "\" objectiveSense must be Maximize or Minimize. ";
		resultAll = false;
	}
	return resultAll;
}

bool MetabolicNetwork::_checkReactionNames(std::string& errorMessage) const {
	bool resultAll = true;
	for (const std::string& reactionName : _reactionNames) {
		if (reactionName.empty()) {
			errorMessage += "MetabolicNetwork \"" + getName() + "\" has an empty reaction name. ";
			resultAll = false;
			continue;
		}
		auto* reaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), reactionName));
		if (reaction == nullptr) {
			errorMessage += "MetabolicNetwork \"" + getName() + "\" references missing MetabolicReaction \"" + reactionName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

bool MetabolicNetwork::_checkExchangeSpeciesNames(std::string& errorMessage) const {
	bool resultAll = true;
	for (const std::string& speciesName : _exchangeSpeciesNames) {
		if (speciesName.empty()) {
			errorMessage += "MetabolicNetwork \"" + getName() + "\" has an empty exchange species name. ";
			resultAll = false;
			continue;
		}
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName));
		if (species == nullptr) {
			errorMessage += "MetabolicNetwork \"" + getName() + "\" references missing exchange BioSpecies \"" + speciesName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

void MetabolicNetwork::addReaction(std::string reactionName) {
	_reactionNames.push_back(reactionName);
}

void MetabolicNetwork::addExchangeSpecies(std::string speciesName) {
	_exchangeSpeciesNames.push_back(speciesName);
}

void MetabolicNetwork::clearReactions() {
	_reactionNames.clear();
}

void MetabolicNetwork::clearExchangeSpecies() {
	_exchangeSpeciesNames.clear();
}

const std::vector<std::string>& MetabolicNetwork::getReactionNames() const {
	return _reactionNames;
}

const std::vector<std::string>& MetabolicNetwork::getExchangeSpeciesNames() const {
	return _exchangeSpeciesNames;
}

void MetabolicNetwork::setObjectiveReactionName(std::string objectiveReactionName) {
	_objectiveReactionName = objectiveReactionName;
}

std::string MetabolicNetwork::getObjectiveReactionName() const {
	return _objectiveReactionName;
}

void MetabolicNetwork::setObjectiveSense(std::string objectiveSense) {
	_objectiveSense = objectiveSense;
}

std::string MetabolicNetwork::getObjectiveSense() const {
	return _objectiveSense;
}

void MetabolicNetwork::setCompartment(std::string compartment) {
	_compartment = compartment;
}

std::string MetabolicNetwork::getCompartment() const {
	return _compartment;
}

void MetabolicNetwork::setEnabled(bool enabled) {
	_enabled = enabled;
}

bool MetabolicNetwork::isEnabled() const {
	return _enabled;
}

void MetabolicNetwork::_createReportStatisticsDataDefinitions() {
}

void MetabolicNetwork::_createEditableDataDefinitions() {
}

void MetabolicNetwork::_createOthersDataDefinitions() {
}
