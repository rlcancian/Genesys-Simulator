#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GeneticRegulation::GetPluginInformation;
}
#endif

ModelDataDefinition* GeneticRegulation::NewInstance(Model* model, std::string name) {
	return new GeneticRegulation(model, name);
}

GeneticRegulation::GeneticRegulation(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<GeneticRegulation>(), name) {
	auto* propRegulatorSpeciesName = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticRegulation::getRegulatorSpeciesName, this), std::bind(&GeneticRegulation::setRegulatorSpeciesName, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "RegulatorSpeciesName", "");
	auto* propTargetPartName = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticRegulation::getTargetPartName, this), std::bind(&GeneticRegulation::setTargetPartName, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "TargetPartName", "");
	auto* propRegulationType = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticRegulation::getRegulationType, this), std::bind(&GeneticRegulation::setRegulationType, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "RegulationType", "");
	auto* propHillCoefficient = new SimulationControlDouble(
			std::bind(&GeneticRegulation::getHillCoefficient, this), std::bind(&GeneticRegulation::setHillCoefficient, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "HillCoefficient", "");
	auto* propDissociationConstant = new SimulationControlDouble(
			std::bind(&GeneticRegulation::getDissociationConstant, this), std::bind(&GeneticRegulation::setDissociationConstant, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "DissociationConstant", "");
	auto* propMaxFoldChange = new SimulationControlDouble(
			std::bind(&GeneticRegulation::getMaxFoldChange, this), std::bind(&GeneticRegulation::setMaxFoldChange, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "MaxFoldChange", "");
	auto* propLeakiness = new SimulationControlDouble(
			std::bind(&GeneticRegulation::getLeakiness, this), std::bind(&GeneticRegulation::setLeakiness, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "Leakiness", "");
	auto* propEnabled = new SimulationControlGeneric<bool>(
			std::bind(&GeneticRegulation::isEnabled, this), std::bind(&GeneticRegulation::setEnabled, this, std::placeholders::_1),
			Util::TypeOf<GeneticRegulation>(), getName(), "Enabled", "");

	_parentModel->getControls()->insert(propRegulatorSpeciesName);
	_parentModel->getControls()->insert(propTargetPartName);
	_parentModel->getControls()->insert(propRegulationType);
	_parentModel->getControls()->insert(propHillCoefficient);
	_parentModel->getControls()->insert(propDissociationConstant);
	_parentModel->getControls()->insert(propMaxFoldChange);
	_parentModel->getControls()->insert(propLeakiness);
	_parentModel->getControls()->insert(propEnabled);

	_addSimulationControl(propRegulatorSpeciesName);
	_addSimulationControl(propTargetPartName);
	_addSimulationControl(propRegulationType);
	_addSimulationControl(propHillCoefficient);
	_addSimulationControl(propDissociationConstant);
	_addSimulationControl(propMaxFoldChange);
	_addSimulationControl(propLeakiness);
	_addSimulationControl(propEnabled);
}

PluginInformation* GeneticRegulation::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GeneticRegulation>(), &GeneticRegulation::LoadInstance, &GeneticRegulation::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Transcriptional regulation relation between a regulator BioSpecies and a target GeneticCircuitPart.");
	return info;
}

ModelDataDefinition* GeneticRegulation::LoadInstance(Model* model, PersistenceRecord* fields) {
	GeneticRegulation* newElement = new GeneticRegulation(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string GeneticRegulation::show() {
	return ModelDataDefinition::show() +
	       ",regulatorSpeciesName=\"" + _regulatorSpeciesName + "\"" +
	       ",targetPartName=\"" + _targetPartName + "\"" +
	       ",regulationType=\"" + _regulationType + "\"" +
	       ",hillCoefficient=" + Util::StrTruncIfInt(std::to_string(_hillCoefficient)) +
	       ",dissociationConstant=" + Util::StrTruncIfInt(std::to_string(_dissociationConstant)) +
	       ",maxFoldChange=" + Util::StrTruncIfInt(std::to_string(_maxFoldChange)) +
	       ",leakiness=" + Util::StrTruncIfInt(std::to_string(_leakiness)) +
	       ",enabled=" + std::to_string(_enabled ? 1 : 0);
}

bool GeneticRegulation::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_regulatorSpeciesName = fields->loadField("regulatorSpeciesName", DEFAULT.regulatorSpeciesName);
		_targetPartName = fields->loadField("targetPartName", DEFAULT.targetPartName);
		_regulationType = fields->loadField("regulationType", DEFAULT.regulationType);
		_hillCoefficient = fields->loadField("hillCoefficient", DEFAULT.hillCoefficient);
		_dissociationConstant = fields->loadField("dissociationConstant", DEFAULT.dissociationConstant);
		_maxFoldChange = fields->loadField("maxFoldChange", DEFAULT.maxFoldChange);
		_leakiness = fields->loadField("leakiness", DEFAULT.leakiness);
		_enabled = fields->loadField("enabled", DEFAULT.enabled ? 1u : 0u) != 0u;
	}
	return res;
}

void GeneticRegulation::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("regulatorSpeciesName", _regulatorSpeciesName, DEFAULT.regulatorSpeciesName, saveDefaultValues);
	fields->saveField("targetPartName", _targetPartName, DEFAULT.targetPartName, saveDefaultValues);
	fields->saveField("regulationType", _regulationType, DEFAULT.regulationType, saveDefaultValues);
	fields->saveField("hillCoefficient", _hillCoefficient, DEFAULT.hillCoefficient, saveDefaultValues);
	fields->saveField("dissociationConstant", _dissociationConstant, DEFAULT.dissociationConstant, saveDefaultValues);
	fields->saveField("maxFoldChange", _maxFoldChange, DEFAULT.maxFoldChange, saveDefaultValues);
	fields->saveField("leakiness", _leakiness, DEFAULT.leakiness, saveDefaultValues);
	fields->saveField("enabled", _enabled ? 1u : 0u, DEFAULT.enabled ? 1u : 0u, saveDefaultValues);
}

bool GeneticRegulation::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "GeneticRegulation must define a non-empty name. ";
		resultAll = false;
	}
	if (_regulatorSpeciesName.empty()) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define regulatorSpeciesName. ";
		resultAll = false;
	} else {
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), _regulatorSpeciesName));
		if (species == nullptr) {
			errorMessage += "GeneticRegulation \"" + getName() + "\" references missing BioSpecies \"" + _regulatorSpeciesName + "\". ";
			resultAll = false;
		}
	}
	if (_targetPartName.empty()) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define targetPartName. ";
		resultAll = false;
	} else {
		auto* part = dynamic_cast<GeneticCircuitPart*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), _targetPartName));
		if (part == nullptr) {
			errorMessage += "GeneticRegulation \"" + getName() + "\" references missing GeneticCircuitPart \"" + _targetPartName + "\". ";
			resultAll = false;
		}
	}
	if (_regulationType != "Activation" && _regulationType != "Repression" && _regulationType != "Dual") {
		errorMessage += "GeneticRegulation \"" + getName() + "\" regulationType must be Activation, Repression, or Dual. ";
		resultAll = false;
	}
	if (_hillCoefficient <= 0.0) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define hillCoefficient > 0. ";
		resultAll = false;
	}
	if (_dissociationConstant <= 0.0) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define dissociationConstant > 0. ";
		resultAll = false;
	}
	if (_maxFoldChange < 0.0) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define maxFoldChange >= 0. ";
		resultAll = false;
	}
	if (_leakiness < 0.0 || _leakiness > 1.0) {
		errorMessage += "GeneticRegulation \"" + getName() + "\" must define leakiness in [0,1]. ";
		resultAll = false;
	}
	return resultAll;
}

void GeneticRegulation::setRegulatorSpeciesName(std::string regulatorSpeciesName) {
	_regulatorSpeciesName = regulatorSpeciesName;
}

std::string GeneticRegulation::getRegulatorSpeciesName() const {
	return _regulatorSpeciesName;
}

void GeneticRegulation::setTargetPartName(std::string targetPartName) {
	_targetPartName = targetPartName;
}

std::string GeneticRegulation::getTargetPartName() const {
	return _targetPartName;
}

void GeneticRegulation::setRegulationType(std::string regulationType) {
	_regulationType = regulationType;
}

std::string GeneticRegulation::getRegulationType() const {
	return _regulationType;
}

void GeneticRegulation::setHillCoefficient(double hillCoefficient) {
	_hillCoefficient = hillCoefficient;
}

double GeneticRegulation::getHillCoefficient() const {
	return _hillCoefficient;
}

void GeneticRegulation::setDissociationConstant(double dissociationConstant) {
	_dissociationConstant = dissociationConstant;
}

double GeneticRegulation::getDissociationConstant() const {
	return _dissociationConstant;
}

void GeneticRegulation::setMaxFoldChange(double maxFoldChange) {
	_maxFoldChange = maxFoldChange;
}

double GeneticRegulation::getMaxFoldChange() const {
	return _maxFoldChange;
}

void GeneticRegulation::setLeakiness(double leakiness) {
	_leakiness = leakiness;
}

double GeneticRegulation::getLeakiness() const {
	return _leakiness;
}

void GeneticRegulation::setEnabled(bool enabled) {
	_enabled = enabled;
}

bool GeneticRegulation::isEnabled() const {
	return _enabled;
}

void GeneticRegulation::_createReportStatisticsDataDefinitions() {
}

void GeneticRegulation::_createEditableDataDefinitions() {
}

void GeneticRegulation::_createOthersDataDefinitions() {
}
