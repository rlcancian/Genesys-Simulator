#include "plugins/data/BiochemicalSimulation/GeneticCircuit.h"

#include <cctype>
#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"
#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

// #ifdef PLUGINCONNECT_DYNAMIC

// extern "C" StaticGetPluginInformation GetPluginInformation() {
// 	return &GeneticCircuit::GetPluginInformation;
// }
// #endif

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

std::string toLowerCopy(const std::string& text) {
	std::string lowered;
	lowered.reserve(text.size());
	for (unsigned char ch : text) {
		lowered += static_cast<char>(std::tolower(ch));
	}
	return lowered;
}

std::string classifyPartRole(const std::string& partType) {
	const std::string lowered = toLowerCopy(partType);
	if (lowered.find("promoter") != std::string::npos || lowered.find("tata") != std::string::npos) {
		return "Pre-gene regulatory";
	}
	if (lowered.find("operator") != std::string::npos || lowered.find("enhancer") != std::string::npos ||
	    lowered.find("insulator") != std::string::npos || lowered.find("silencer") != std::string::npos ||
	    lowered.find("regulatory") != std::string::npos) {
		return "Regulatory";
	}
	if (lowered.find("rbs") != std::string::npos || lowered.find("ribosome") != std::string::npos) {
		return "Translation initiation";
	}
	if (lowered.find("cds") != std::string::npos || lowered.find("orf") != std::string::npos ||
	    lowered.find("coding") != std::string::npos || lowered.find("gene") != std::string::npos) {
		return "Coding";
	}
	if (lowered.find("terminator") != std::string::npos) {
		return "Termination";
	}
	if (lowered.find("region") != std::string::npos || lowered.find("utr") != std::string::npos ||
	    lowered.find("leader") != std::string::npos || lowered.find("trailer") != std::string::npos ||
	    lowered.find("pre gene") != std::string::npos || lowered.find("pre-gene") != std::string::npos ||
	    lowered.find("pregene") != std::string::npos) {
		return "Pre-gene region";
	}
	return "Other";
}

std::string summarizePartRoles(Model* model, const std::vector<std::string>& partNames) {
	std::vector<std::pair<std::string, unsigned int>> counts = {
		{"Pre-gene regulatory", 0u},
		{"Regulatory", 0u},
		{"Translation initiation", 0u},
		{"Coding", 0u},
		{"Termination", 0u},
		{"Pre-gene region", 0u},
		{"Other", 0u},
		{"Missing", 0u},
	};

	auto incrementCount = [&counts](const std::string& label) {
		for (std::pair<std::string, unsigned int>& count : counts) {
			if (count.first == label) {
				++count.second;
				return;
			}
		}
		counts.push_back({label, 1u});
	};

	if (model == nullptr || model->getDataManager() == nullptr) {
		incrementCount("Missing");
	} else {
		for (const std::string& partName : partNames) {
			auto* part = dynamic_cast<GeneticCircuitPart*>(model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), partName));
			if (part == nullptr) {
				incrementCount("Missing");
				continue;
			}
			incrementCount(classifyPartRole(part->getPartType()));
		}
	}

	std::string summary = "{";
	for (const std::pair<std::string, unsigned int>& count : counts) {
		if (count.second == 0u) {
			continue;
		}
		summary += count.first + "=" + std::to_string(count.second) + ",";
	}
	if (summary.back() == ',') {
		summary.pop_back();
	}
	summary += "}";
	return summary;
}

std::string summarizeRegulatoryLinks(Model* model, const std::vector<std::string>& regulationNames) {
	if (model == nullptr || model->getDataManager() == nullptr) {
		return "{}";
	}

	std::string summary = "{";
	for (const std::string& regulationName : regulationNames) {
		auto* regulation = dynamic_cast<GeneticRegulation*>(model->getDataManager()->getDataDefinition(Util::TypeOf<GeneticRegulation>(), regulationName));
		if (regulation == nullptr) {
			continue;
		}
		summary += regulation->getRegulatorSpeciesName() + "->" + regulation->getTargetPartName();
		if (!regulation->getRegulationType().empty()) {
			summary += ":" + regulation->getRegulationType();
		}
		summary += ",";
	}
	if (summary.back() == ',') {
		summary.pop_back();
	}
	summary += "}";
	return summary;
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
	info->setCategory("Biologic/Biochemical/Genetic");
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
	       ",partCount=" + std::to_string(_partNames.size()) +
	       ",regulationCount=" + std::to_string(_regulationNames.size()) +
	       ",parts=" + namesToString(_partNames) +
	       ",regulations=" + namesToString(_regulationNames) +
	       ",partsByRole=" + summarizePartRoles(_parentModel, _partNames) +
	       ",regulatoryLinks=" + summarizeRegulatoryLinks(_parentModel, _regulationNames) +
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

// void GeneticCircuit::_createInternalStatisticReporters() { }

// void GeneticCircuit::_createEditableDataDefinitions() { }

// void GeneticCircuit::_createAttachedAttributes() { }
