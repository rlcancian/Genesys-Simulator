#include "plugins/data/BiochemicalSimulation/GeneticCircuit.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"
#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GeneticCircuit::GetPluginInformation;
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

ModelDataDefinition* GeneticCircuit::NewInstance(Model* model, std::string name) {
	return new GeneticCircuit(model, name);
}

GeneticCircuit::GeneticCircuit(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<GeneticCircuit>(), name) {
	auto* propHostOrganism = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuit::getHostOrganism, this), std::bind(&GeneticCircuit::setHostOrganism, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuit>(), getName(), "HostOrganism", "");
	auto* propCompartment = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuit::getCompartment, this), std::bind(&GeneticCircuit::setCompartment, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuit>(), getName(), "Compartment", "");
	auto* propEnabled = new SimulationControlGeneric<bool>(
			std::bind(&GeneticCircuit::isEnabled, this), std::bind(&GeneticCircuit::setEnabled, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuit>(), getName(), "Enabled", "");

	_parentModel->getControls()->insert(propHostOrganism);
	_parentModel->getControls()->insert(propCompartment);
	_parentModel->getControls()->insert(propEnabled);

	_addSimulationControl(propHostOrganism);
	_addSimulationControl(propCompartment);
	_addSimulationControl(propEnabled);
}

PluginInformation* GeneticCircuit::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GeneticCircuit>(), &GeneticCircuit::LoadInstance, &GeneticCircuit::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Aggregate genetic circuit definition with ordered parts, regulations, host metadata, and enable flag.");
	return info;
}

ModelDataDefinition* GeneticCircuit::LoadInstance(Model* model, PersistenceRecord* fields) {
	GeneticCircuit* newElement = new GeneticCircuit(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string GeneticCircuit::show() {
	return ModelDataDefinition::show() +
	       ",parts=" + namesToString(_partNames) +
	       ",regulations=" + namesToString(_regulationNames) +
	       ",hostOrganism=\"" + _hostOrganism + "\"" +
	       ",compartment=\"" + _compartment + "\"" +
	       ",enabled=" + std::to_string(_enabled ? 1 : 0);
}

bool GeneticCircuit::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_partNames.clear();
		_regulationNames.clear();
		const unsigned int partCount = fields->loadField("parts", 0u);
		for (unsigned int i = 0; i < partCount; ++i) {
			addPart(fields->loadField("partName" + Util::StrIndex(i), ""));
		}
		const unsigned int regulationCount = fields->loadField("regulations", 0u);
		for (unsigned int i = 0; i < regulationCount; ++i) {
			addRegulation(fields->loadField("regulationName" + Util::StrIndex(i), ""));
		}
		_hostOrganism = fields->loadField("hostOrganism", DEFAULT.hostOrganism);
		_compartment = fields->loadField("compartment", DEFAULT.compartment);
		_enabled = fields->loadField("enabled", DEFAULT.enabled ? 1u : 0u) != 0u;
	}
	return res;
}

void GeneticCircuit::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("parts", static_cast<unsigned int>(_partNames.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _partNames.size(); ++i) {
		fields->saveField("partName" + Util::StrIndex(i), _partNames[i], "", saveDefaultValues);
	}
	fields->saveField("regulations", static_cast<unsigned int>(_regulationNames.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _regulationNames.size(); ++i) {
		fields->saveField("regulationName" + Util::StrIndex(i), _regulationNames[i], "", saveDefaultValues);
	}
	fields->saveField("hostOrganism", _hostOrganism, DEFAULT.hostOrganism, saveDefaultValues);
	fields->saveField("compartment", _compartment, DEFAULT.compartment, saveDefaultValues);
	fields->saveField("enabled", _enabled ? 1u : 0u, DEFAULT.enabled ? 1u : 0u, saveDefaultValues);
}

bool GeneticCircuit::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "GeneticCircuit must define a non-empty name. ";
		resultAll = false;
	}
	if (_partNames.empty()) {
		errorMessage += "GeneticCircuit \"" + getName() + "\" must define at least one part. ";
		resultAll = false;
	}
	resultAll = _checkPartNames(errorMessage) && resultAll;
	resultAll = _checkRegulationNames(errorMessage) && resultAll;
	return resultAll;
}

bool GeneticCircuit::_checkPartNames(std::string& errorMessage) const {
	bool resultAll = true;
	for (const std::string& partName : _partNames) {
		if (partName.empty()) {
			errorMessage += "GeneticCircuit \"" + getName() + "\" has an empty part name. ";
			resultAll = false;
			continue;
		}
		auto* part = dynamic_cast<GeneticCircuitPart*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), partName));
		if (part == nullptr) {
			errorMessage += "GeneticCircuit \"" + getName() + "\" references missing GeneticCircuitPart \"" + partName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

bool GeneticCircuit::_checkRegulationNames(std::string& errorMessage) const {
	bool resultAll = true;
	for (const std::string& regulationName : _regulationNames) {
		if (regulationName.empty()) {
			errorMessage += "GeneticCircuit \"" + getName() + "\" has an empty regulation name. ";
			resultAll = false;
			continue;
		}
		auto* regulation = dynamic_cast<GeneticRegulation*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticRegulation>(), regulationName));
		if (regulation == nullptr) {
			errorMessage += "GeneticCircuit \"" + getName() + "\" references missing GeneticRegulation \"" + regulationName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

void GeneticCircuit::addPart(std::string partName) {
	_partNames.push_back(partName);
}

void GeneticCircuit::addRegulation(std::string regulationName) {
	_regulationNames.push_back(regulationName);
}

void GeneticCircuit::clearParts() {
	_partNames.clear();
}

void GeneticCircuit::clearRegulations() {
	_regulationNames.clear();
}

const std::vector<std::string>& GeneticCircuit::getPartNames() const {
	return _partNames;
}

const std::vector<std::string>& GeneticCircuit::getRegulationNames() const {
	return _regulationNames;
}

void GeneticCircuit::setHostOrganism(std::string hostOrganism) {
	_hostOrganism = hostOrganism;
}

std::string GeneticCircuit::getHostOrganism() const {
	return _hostOrganism;
}

void GeneticCircuit::setCompartment(std::string compartment) {
	_compartment = compartment;
}

std::string GeneticCircuit::getCompartment() const {
	return _compartment;
}

void GeneticCircuit::setEnabled(bool enabled) {
	_enabled = enabled;
}

bool GeneticCircuit::isEnabled() const {
	return _enabled;
}

void GeneticCircuit::_createReportStatisticsDataDefinitions() {
}

void GeneticCircuit::_createEditableDataDefinitions() {
}

void GeneticCircuit::_createOthersDataDefinitions() {
}
