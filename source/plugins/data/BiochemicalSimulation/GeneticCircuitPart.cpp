#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GeneticCircuitPart::GetPluginInformation;
}
#endif

ModelDataDefinition* GeneticCircuitPart::NewInstance(Model* model, std::string name) {
	return new GeneticCircuitPart(model, name);
}

GeneticCircuitPart::GeneticCircuitPart(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<GeneticCircuitPart>(), name) {
	auto* propPartType = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuitPart::getPartType, this), std::bind(&GeneticCircuitPart::setPartType, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "PartType", "");
	auto* propSequence = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuitPart::getSequence, this), std::bind(&GeneticCircuitPart::setSequence, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "Sequence", "");
	auto* propProductSpeciesName = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuitPart::getProductSpeciesName, this), std::bind(&GeneticCircuitPart::setProductSpeciesName, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "ProductSpeciesName", "");
	auto* propCopyNumber = new SimulationControlDouble(
			std::bind(&GeneticCircuitPart::getCopyNumber, this), std::bind(&GeneticCircuitPart::setCopyNumber, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "CopyNumber", "");
	auto* propBasalExpressionRate = new SimulationControlDouble(
			std::bind(&GeneticCircuitPart::getBasalExpressionRate, this), std::bind(&GeneticCircuitPart::setBasalExpressionRate, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "BasalExpressionRate", "");
	auto* propDegradationRate = new SimulationControlDouble(
			std::bind(&GeneticCircuitPart::getDegradationRate, this), std::bind(&GeneticCircuitPart::setDegradationRate, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "DegradationRate", "");
	auto* propEnabled = new SimulationControlGeneric<bool>(
			std::bind(&GeneticCircuitPart::isEnabled, this), std::bind(&GeneticCircuitPart::setEnabled, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitPart>(), getName(), "Enabled", "");

	_parentModel->getControls()->insert(propPartType);
	_parentModel->getControls()->insert(propSequence);
	_parentModel->getControls()->insert(propProductSpeciesName);
	_parentModel->getControls()->insert(propCopyNumber);
	_parentModel->getControls()->insert(propBasalExpressionRate);
	_parentModel->getControls()->insert(propDegradationRate);
	_parentModel->getControls()->insert(propEnabled);

	_addSimulationControl(propPartType);
	_addSimulationControl(propSequence);
	_addSimulationControl(propProductSpeciesName);
	_addSimulationControl(propCopyNumber);
	_addSimulationControl(propBasalExpressionRate);
	_addSimulationControl(propDegradationRate);
	_addSimulationControl(propEnabled);
}

PluginInformation* GeneticCircuitPart::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GeneticCircuitPart>(), &GeneticCircuitPart::LoadInstance, &GeneticCircuitPart::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Genetic circuit part definition (promoter/RBS/CDS/terminator, sequence, copy number, and expression defaults).");
	return info;
}

ModelDataDefinition* GeneticCircuitPart::LoadInstance(Model* model, PersistenceRecord* fields) {
	GeneticCircuitPart* newElement = new GeneticCircuitPart(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string GeneticCircuitPart::show() {
	return ModelDataDefinition::show() +
	       ",partType=\"" + _partType + "\"" +
	       ",sequence=\"" + _sequence + "\"" +
	       ",productSpeciesName=\"" + _productSpeciesName + "\"" +
	       ",copyNumber=" + Util::StrTruncIfInt(std::to_string(_copyNumber)) +
	       ",basalExpressionRate=" + Util::StrTruncIfInt(std::to_string(_basalExpressionRate)) +
	       ",degradationRate=" + Util::StrTruncIfInt(std::to_string(_degradationRate)) +
	       ",enabled=" + std::to_string(_enabled ? 1 : 0);
}

bool GeneticCircuitPart::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_partType = fields->loadField("partType", DEFAULT.partType);
		_sequence = fields->loadField("sequence", DEFAULT.sequence);
		_productSpeciesName = fields->loadField("productSpeciesName", DEFAULT.productSpeciesName);
		_copyNumber = fields->loadField("copyNumber", DEFAULT.copyNumber);
		_basalExpressionRate = fields->loadField("basalExpressionRate", DEFAULT.basalExpressionRate);
		_degradationRate = fields->loadField("degradationRate", DEFAULT.degradationRate);
		_enabled = fields->loadField("enabled", DEFAULT.enabled ? 1u : 0u) != 0u;
	}
	return res;
}

void GeneticCircuitPart::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("partType", _partType, DEFAULT.partType, saveDefaultValues);
	fields->saveField("sequence", _sequence, DEFAULT.sequence, saveDefaultValues);
	fields->saveField("productSpeciesName", _productSpeciesName, DEFAULT.productSpeciesName, saveDefaultValues);
	fields->saveField("copyNumber", _copyNumber, DEFAULT.copyNumber, saveDefaultValues);
	fields->saveField("basalExpressionRate", _basalExpressionRate, DEFAULT.basalExpressionRate, saveDefaultValues);
	fields->saveField("degradationRate", _degradationRate, DEFAULT.degradationRate, saveDefaultValues);
	fields->saveField("enabled", _enabled ? 1u : 0u, DEFAULT.enabled ? 1u : 0u, saveDefaultValues);
}

bool GeneticCircuitPart::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "GeneticCircuitPart must define a non-empty name. ";
		resultAll = false;
	}
	if (_partType.empty()) {
		errorMessage += "GeneticCircuitPart \"" + getName() + "\" must define partType. ";
		resultAll = false;
	}
	if (_copyNumber < 0.0) {
		errorMessage += "GeneticCircuitPart \"" + getName() + "\" must define copyNumber >= 0. ";
		resultAll = false;
	}
	if (_basalExpressionRate < 0.0) {
		errorMessage += "GeneticCircuitPart \"" + getName() + "\" must define basalExpressionRate >= 0. ";
		resultAll = false;
	}
	if (_degradationRate < 0.0) {
		errorMessage += "GeneticCircuitPart \"" + getName() + "\" must define degradationRate >= 0. ";
		resultAll = false;
	}
	if (!_productSpeciesName.empty()) {
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), _productSpeciesName));
		if (species == nullptr) {
			errorMessage += "GeneticCircuitPart \"" + getName() + "\" references missing BioSpecies \"" + _productSpeciesName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

void GeneticCircuitPart::setPartType(std::string partType) {
	_partType = partType;
}

std::string GeneticCircuitPart::getPartType() const {
	return _partType;
}

void GeneticCircuitPart::setSequence(std::string sequence) {
	_sequence = sequence;
}

std::string GeneticCircuitPart::getSequence() const {
	return _sequence;
}

void GeneticCircuitPart::setProductSpeciesName(std::string productSpeciesName) {
	_productSpeciesName = productSpeciesName;
}

std::string GeneticCircuitPart::getProductSpeciesName() const {
	return _productSpeciesName;
}

void GeneticCircuitPart::setCopyNumber(double copyNumber) {
	_copyNumber = copyNumber;
}

double GeneticCircuitPart::getCopyNumber() const {
	return _copyNumber;
}

void GeneticCircuitPart::setBasalExpressionRate(double basalExpressionRate) {
	_basalExpressionRate = basalExpressionRate;
}

double GeneticCircuitPart::getBasalExpressionRate() const {
	return _basalExpressionRate;
}

void GeneticCircuitPart::setDegradationRate(double degradationRate) {
	_degradationRate = degradationRate;
}

double GeneticCircuitPart::getDegradationRate() const {
	return _degradationRate;
}

void GeneticCircuitPart::setEnabled(bool enabled) {
	_enabled = enabled;
}

bool GeneticCircuitPart::isEnabled() const {
	return _enabled;
}

void GeneticCircuitPart::_createReportStatisticsDataDefinitions() {
}

void GeneticCircuitPart::_createEditableDataDefinitions() {
}

void GeneticCircuitPart::_createOthersDataDefinitions() {
}
