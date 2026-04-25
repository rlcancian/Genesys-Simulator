#include "plugins/components/BiochemicalSimulation/GeneticCircuitSimulate.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuitPart.h"
#include "plugins/data/BiochemicalSimulation/GeneticRegulation.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &GeneticCircuitSimulate::GetPluginInformation;
}
#endif

ModelDataDefinition* GeneticCircuitSimulate::NewInstance(Model* model, std::string name) {
	return new GeneticCircuitSimulate(model, name);
}

GeneticCircuitSimulate::GeneticCircuitSimulate(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<GeneticCircuitSimulate>(), name) {
	auto* propGeneticCircuit = new SimulationControlGenericClass<GeneticCircuit*, Model*, GeneticCircuit>(
			_parentModel,
			std::bind(&GeneticCircuitSimulate::getGeneticCircuit, this),
			std::bind(&GeneticCircuitSimulate::setGeneticCircuit, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "GeneticCircuit", "");
	auto* propStartTime = new SimulationControlDouble(
			std::bind(&GeneticCircuitSimulate::getStartTime, this),
			std::bind(&GeneticCircuitSimulate::setStartTime, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "StartTime", "");
	auto* propStopTime = new SimulationControlDouble(
			std::bind(&GeneticCircuitSimulate::getStopTime, this),
			std::bind(&GeneticCircuitSimulate::setStopTime, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "StopTime", "");
	auto* propStepSize = new SimulationControlDouble(
			std::bind(&GeneticCircuitSimulate::getStepSize, this),
			std::bind(&GeneticCircuitSimulate::setStepSize, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "StepSize", "");
	auto* propApplyRegulation = new SimulationControlGeneric<bool>(
			std::bind(&GeneticCircuitSimulate::getApplyRegulation, this),
			std::bind(&GeneticCircuitSimulate::setApplyRegulation, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "ApplyRegulation", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&GeneticCircuitSimulate::getLastSucceeded, this),
			std::bind(&GeneticCircuitSimulate::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "LastSucceeded", "");
	auto* propLastSampleCount = new SimulationControlGeneric<unsigned int>(
			std::bind(&GeneticCircuitSimulate::getLastSampleCount, this),
			std::bind(&GeneticCircuitSimulate::setLastSampleCount, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "LastSampleCount", "");
	auto* propLastTotalExpression = new SimulationControlDouble(
			std::bind(&GeneticCircuitSimulate::getLastTotalExpression, this),
			std::bind(&GeneticCircuitSimulate::setLastTotalExpression, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "LastTotalExpression", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticCircuitSimulate::getLastMessage, this),
			std::bind(&GeneticCircuitSimulate::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<GeneticCircuitSimulate>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propGeneticCircuit);
	_parentModel->getControls()->insert(propStartTime);
	_parentModel->getControls()->insert(propStopTime);
	_parentModel->getControls()->insert(propStepSize);
	_parentModel->getControls()->insert(propApplyRegulation);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastSampleCount);
	_parentModel->getControls()->insert(propLastTotalExpression);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propGeneticCircuit);
	_addSimulationControl(propStartTime);
	_addSimulationControl(propStopTime);
	_addSimulationControl(propStepSize);
	_addSimulationControl(propApplyRegulation);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastSampleCount);
	_addSimulationControl(propLastTotalExpression);
	_addSimulationControl(propLastMessage);
}

PluginInformation* GeneticCircuitSimulate::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GeneticCircuitSimulate>(), &GeneticCircuitSimulate::LoadInstance, &GeneticCircuitSimulate::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("geneticcircuit.so");
	info->setDescriptionHelp("Simulates a GeneticCircuit over a configurable time window using repeated expression steps and updates referenced BioSpecies.");
	return info;
}

ModelComponent* GeneticCircuitSimulate::LoadInstance(Model* model, PersistenceRecord* fields) {
	GeneticCircuitSimulate* newComponent = new GeneticCircuitSimulate(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string GeneticCircuitSimulate::show() {
	return ModelComponent::show() +
	       ",geneticCircuit=\"" + (_geneticCircuit != nullptr ? _geneticCircuit->getName() : std::string()) + "\"" +
	       ",startTime=" + Util::StrTruncIfInt(std::to_string(_startTime)) +
	       ",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
	       ",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
	       ",applyRegulation=" + std::to_string(_applyRegulation ? 1 : 0) +
	       ",lastSampleCount=" + std::to_string(_lastSampleCount) +
	       ",lastTotalExpression=" + Util::StrTruncIfInt(std::to_string(_lastTotalExpression));
}

bool GeneticCircuitSimulate::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string geneticCircuitName = fields->loadField("geneticCircuit", DEFAULT.geneticCircuitName);
		_geneticCircuit = nullptr;
		if (!geneticCircuitName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuit>(), geneticCircuitName);
			_geneticCircuit = dynamic_cast<GeneticCircuit*>(definition);
		}
		_startTime = fields->loadField("startTime", DEFAULT.startTime);
		_stopTime = fields->loadField("stopTime", DEFAULT.stopTime);
		_stepSize = fields->loadField("stepSize", DEFAULT.stepSize);
		_applyRegulation = fields->loadField("applyRegulation", DEFAULT.applyRegulation ? 1u : 0u) != 0u;
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastSampleCount = fields->loadField("lastSampleCount", DEFAULT.lastSampleCount);
		_lastTotalExpression = fields->loadField("lastTotalExpression", DEFAULT.lastTotalExpression);
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void GeneticCircuitSimulate::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("geneticCircuit", _geneticCircuit != nullptr ? _geneticCircuit->getName() : DEFAULT.geneticCircuitName, DEFAULT.geneticCircuitName, saveDefaultValues);
	fields->saveField("startTime", _startTime, DEFAULT.startTime, saveDefaultValues);
	fields->saveField("stopTime", _stopTime, DEFAULT.stopTime, saveDefaultValues);
	fields->saveField("stepSize", _stepSize, DEFAULT.stepSize, saveDefaultValues);
	fields->saveField("applyRegulation", _applyRegulation ? 1u : 0u, DEFAULT.applyRegulation ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastSampleCount", _lastSampleCount, DEFAULT.lastSampleCount, saveDefaultValues);
	fields->saveField("lastTotalExpression", _lastTotalExpression, DEFAULT.lastTotalExpression, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool GeneticCircuitSimulate::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<GeneticCircuit>(), _geneticCircuit, "GeneticCircuit", errorMessage);
	if (_stepSize <= 0.0) {
		errorMessage += "GeneticCircuitSimulate \"" + getName() + "\" must define stepSize > 0. ";
		resultAll = false;
	}
	if (_stopTime < _startTime) {
		errorMessage += "GeneticCircuitSimulate \"" + getName() + "\" must define stopTime >= startTime. ";
		resultAll = false;
	}
	return resultAll;
}

void GeneticCircuitSimulate::_createInternalAndAttachedData() {
	if (_geneticCircuit != nullptr) {
		_attachedDataInsert("GeneticCircuit", _geneticCircuit);
	} else {
		_attachedDataRemove("GeneticCircuit");
	}
}

double GeneticCircuitSimulate::_computeRegulationMultiplier(const GeneticCircuitPart* part) const {
	if (!_applyRegulation || _geneticCircuit == nullptr || part == nullptr) {
		return 1.0;
	}

	double multiplier = 1.0;
	for (const std::string& regulationName : _geneticCircuit->getRegulationNames()) {
		auto* regulation = dynamic_cast<GeneticRegulation*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticRegulation>(), regulationName));
		if (regulation == nullptr || !regulation->isEnabled() || regulation->getTargetPartName() != part->getName()) {
			continue;
		}
		auto* regulator = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), regulation->getRegulatorSpeciesName()));
		if (regulator == nullptr) {
			continue;
		}

		const double regulatorAmount = std::max(0.0, regulator->getAmount());
		const double hill = regulation->getHillCoefficient();
		const double kd = regulation->getDissociationConstant();
		const double numerator = std::pow(regulatorAmount, hill);
		const double kdPow = std::pow(kd, hill);
		const double saturation = numerator / (kdPow + numerator);
		double factor = 1.0;
		if (regulation->getRegulationType() == "Activation" || regulation->getRegulationType() == "Dual") {
			factor = regulation->getLeakiness() + regulation->getMaxFoldChange() * saturation;
		} else if (regulation->getRegulationType() == "Repression") {
			factor = regulation->getLeakiness() + regulation->getMaxFoldChange() * (1.0 - saturation);
		}
		multiplier *= factor;
	}
	return multiplier;
}

void GeneticCircuitSimulate::_executeStep(double timeStep) {
	for (const std::string& partName : _geneticCircuit->getPartNames()) {
		auto* part = dynamic_cast<GeneticCircuitPart*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuitPart>(), partName));
		if (part == nullptr || !part->isEnabled() || part->getProductSpeciesName().empty()) {
			continue;
		}
		auto* product = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), part->getProductSpeciesName()));
		if (product == nullptr) {
			continue;
		}

		const double regulationMultiplier = _computeRegulationMultiplier(part);
		const double expressionIncrement = part->getBasalExpressionRate() * part->getCopyNumber() * regulationMultiplier * timeStep;
		const double degradationLoss = part->getDegradationRate() * product->getAmount() * timeStep;
		const double nextAmount = std::max(0.0, product->getAmount() + expressionIncrement - degradationLoss);
		product->setAmount(nextAmount);
		_lastTotalExpression += expressionIncrement;
	}
}

void GeneticCircuitSimulate::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_geneticCircuit == nullptr) {
		_lastSucceeded = false;
		_lastSampleCount = 0;
		_lastTotalExpression = 0.0;
		_lastMessage = "GeneticCircuitSimulate requires a referenced GeneticCircuit.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	_lastSampleCount = 0;
	_lastTotalExpression = 0.0;
	for (double time = _startTime; time <= _stopTime + 1e-9; time += _stepSize) {
		_executeStep(_stepSize);
		++_lastSampleCount;
	}

	_lastSucceeded = true;
	_lastMessage = "GeneticCircuitSimulate executed GeneticCircuit \"" + _geneticCircuit->getName() + "\".";
	traceSimulation(this, TraceManager::Level::L2_results,
	                "GeneticCircuitSimulate executed GeneticCircuit \"" + _geneticCircuit->getName() +
	                "\" samples=" + std::to_string(_lastSampleCount) +
	                ", totalExpression=" + Util::StrTruncIfInt(std::to_string(_lastTotalExpression)));
	_forwardEntity(entity);
}

void GeneticCircuitSimulate::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "GeneticCircuitSimulate dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void GeneticCircuitSimulate::setGeneticCircuit(GeneticCircuit* geneticCircuit) {
	_geneticCircuit = geneticCircuit;
}

GeneticCircuit* GeneticCircuitSimulate::getGeneticCircuit() const {
	return _geneticCircuit;
}

void GeneticCircuitSimulate::setStartTime(double startTime) {
	_startTime = startTime;
}

double GeneticCircuitSimulate::getStartTime() const {
	return _startTime;
}

void GeneticCircuitSimulate::setStopTime(double stopTime) {
	_stopTime = stopTime;
}

double GeneticCircuitSimulate::getStopTime() const {
	return _stopTime;
}

void GeneticCircuitSimulate::setStepSize(double stepSize) {
	_stepSize = stepSize;
}

double GeneticCircuitSimulate::getStepSize() const {
	return _stepSize;
}

void GeneticCircuitSimulate::setApplyRegulation(bool applyRegulation) {
	_applyRegulation = applyRegulation;
}

bool GeneticCircuitSimulate::getApplyRegulation() const {
	return _applyRegulation;
}

void GeneticCircuitSimulate::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool GeneticCircuitSimulate::getLastSucceeded() const {
	return _lastSucceeded;
}

void GeneticCircuitSimulate::setLastSampleCount(unsigned int lastSampleCount) {
	_lastSampleCount = lastSampleCount;
}

unsigned int GeneticCircuitSimulate::getLastSampleCount() const {
	return _lastSampleCount;
}

void GeneticCircuitSimulate::setLastTotalExpression(double lastTotalExpression) {
	_lastTotalExpression = lastTotalExpression;
}

double GeneticCircuitSimulate::getLastTotalExpression() const {
	return _lastTotalExpression;
}

void GeneticCircuitSimulate::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string GeneticCircuitSimulate::getLastMessage() const {
	return _lastMessage;
}

void GeneticCircuitSimulate::_createReportStatisticsDataDefinitions() {
}

void GeneticCircuitSimulate::_createEditableDataDefinitions() {
}

void GeneticCircuitSimulate::_createOthersDataDefinitions() {
}
