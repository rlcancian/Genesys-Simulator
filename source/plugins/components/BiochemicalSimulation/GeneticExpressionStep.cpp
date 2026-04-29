#include "plugins/components/BiochemicalSimulation/GeneticExpressionStep.h"

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
	return &GeneticExpressionStep::GetPluginInformation;
}
#endif

ModelDataDefinition* GeneticExpressionStep::NewInstance(Model* model, std::string name) {
	return new GeneticExpressionStep(model, name);
}

GeneticExpressionStep::GeneticExpressionStep(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<GeneticExpressionStep>(), name) {
	auto* propGeneticCircuit = new SimulationControlGenericClass<GeneticCircuit*, Model*, GeneticCircuit>(
			_parentModel,
			std::bind(&GeneticExpressionStep::getGeneticCircuit, this),
			std::bind(&GeneticExpressionStep::setGeneticCircuit, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "GeneticCircuit", "");
	auto* propTimeStep = new SimulationControlDouble(
			std::bind(&GeneticExpressionStep::getTimeStep, this),
			std::bind(&GeneticExpressionStep::setTimeStep, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "TimeStep", "");
	auto* propApplyRegulation = new SimulationControlGeneric<bool>(
			std::bind(&GeneticExpressionStep::getApplyRegulation, this),
			std::bind(&GeneticExpressionStep::setApplyRegulation, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "ApplyRegulation", "");
	auto* propLastSucceeded = new SimulationControlGeneric<bool>(
			std::bind(&GeneticExpressionStep::getLastSucceeded, this),
			std::bind(&GeneticExpressionStep::setLastSucceeded, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "LastSucceeded", "");
	auto* propLastTotalExpression = new SimulationControlDouble(
			std::bind(&GeneticExpressionStep::getLastTotalExpression, this),
			std::bind(&GeneticExpressionStep::setLastTotalExpression, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "LastTotalExpression", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&GeneticExpressionStep::getLastMessage, this),
			std::bind(&GeneticExpressionStep::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<GeneticExpressionStep>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propGeneticCircuit);
	_parentModel->getControls()->insert(propTimeStep);
	_parentModel->getControls()->insert(propApplyRegulation);
	_parentModel->getControls()->insert(propLastSucceeded);
	_parentModel->getControls()->insert(propLastTotalExpression);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propGeneticCircuit);
	_addSimulationControl(propTimeStep);
	_addSimulationControl(propApplyRegulation);
	_addSimulationControl(propLastSucceeded);
	_addSimulationControl(propLastTotalExpression);
	_addSimulationControl(propLastMessage);
}

PluginInformation* GeneticExpressionStep::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<GeneticExpressionStep>(), &GeneticExpressionStep::LoadInstance, &GeneticExpressionStep::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("geneticcircuit.so");
	info->setDescriptionHelp("Executes one genetic expression update step over a GeneticCircuit and writes the resulting expression into referenced BioSpecies.");
	return info;
}

ModelComponent* GeneticExpressionStep::LoadInstance(Model* model, PersistenceRecord* fields) {
	GeneticExpressionStep* newComponent = new GeneticExpressionStep(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string GeneticExpressionStep::show() {
	return ModelComponent::show() +
	       ",geneticCircuit=\"" + (_geneticCircuit != nullptr ? _geneticCircuit->getName() : std::string()) + "\"" +
	       ",timeStep=" + Util::StrTruncIfInt(std::to_string(_timeStep)) +
	       ",applyRegulation=" + std::to_string(_applyRegulation ? 1 : 0) +
	       ",lastSucceeded=" + std::to_string(_lastSucceeded ? 1 : 0) +
	       ",lastTotalExpression=" + Util::StrTruncIfInt(std::to_string(_lastTotalExpression));
}

bool GeneticExpressionStep::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string geneticCircuitName = fields->loadField("geneticCircuit", DEFAULT.geneticCircuitName);
		_geneticCircuit = nullptr;
		if (!geneticCircuitName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GeneticCircuit>(), geneticCircuitName);
			_geneticCircuit = dynamic_cast<GeneticCircuit*>(definition);
		}
		_timeStep = fields->loadField("timeStep", DEFAULT.timeStep);
		_applyRegulation = fields->loadField("applyRegulation", DEFAULT.applyRegulation ? 1u : 0u) != 0u;
		_lastSucceeded = fields->loadField("lastSucceeded", DEFAULT.lastSucceeded ? 1u : 0u) != 0u;
		_lastTotalExpression = fields->loadField("lastTotalExpression", DEFAULT.lastTotalExpression);
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void GeneticExpressionStep::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("geneticCircuit", _geneticCircuit != nullptr ? _geneticCircuit->getName() : DEFAULT.geneticCircuitName, DEFAULT.geneticCircuitName, saveDefaultValues);
	fields->saveField("timeStep", _timeStep, DEFAULT.timeStep, saveDefaultValues);
	fields->saveField("applyRegulation", _applyRegulation ? 1u : 0u, DEFAULT.applyRegulation ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastSucceeded", _lastSucceeded ? 1u : 0u, DEFAULT.lastSucceeded ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastTotalExpression", _lastTotalExpression, DEFAULT.lastTotalExpression, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool GeneticExpressionStep::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<GeneticCircuit>(), _geneticCircuit, "GeneticCircuit", errorMessage);
	if (_timeStep <= 0.0) {
		errorMessage += "GeneticExpressionStep \"" + getName() + "\" must define timeStep > 0. ";
		resultAll = false;
	}
	return resultAll;
}

void GeneticExpressionStep::_createInternalAndAttachedData() {
	if (_geneticCircuit != nullptr) {
		_attachedDataInsert("GeneticCircuit", _geneticCircuit);
	} else {
		_attachedDataRemove("GeneticCircuit");
	}
}

double GeneticExpressionStep::_computeRegulationMultiplier(const GeneticCircuitPart* part) const {
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

void GeneticExpressionStep::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_geneticCircuit == nullptr) {
		_lastSucceeded = false;
		_lastTotalExpression = 0.0;
		_lastMessage = "GeneticExpressionStep requires a referenced GeneticCircuit.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	_lastTotalExpression = 0.0;
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
		const double expressionIncrement = part->getBasalExpressionRate() * part->getCopyNumber() * regulationMultiplier * _timeStep;
		const double degradationLoss = part->getDegradationRate() * product->getAmount() * _timeStep;
		const double nextAmount = std::max(0.0, product->getAmount() + expressionIncrement - degradationLoss);
		product->setAmount(nextAmount);
		_lastTotalExpression += expressionIncrement;
	}

	_lastSucceeded = true;
	_lastMessage = "GeneticExpressionStep updated GeneticCircuit \"" + _geneticCircuit->getName() + "\".";
	traceSimulation(this, TraceManager::Level::L2_results,
	                "GeneticExpressionStep updated GeneticCircuit \"" + _geneticCircuit->getName() +
	                "\" totalExpression=" + Util::StrTruncIfInt(std::to_string(_lastTotalExpression)));
	_forwardEntity(entity);
}

void GeneticExpressionStep::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "GeneticExpressionStep dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void GeneticExpressionStep::setGeneticCircuit(GeneticCircuit* geneticCircuit) {
	_geneticCircuit = geneticCircuit;
}

GeneticCircuit* GeneticExpressionStep::getGeneticCircuit() const {
	return _geneticCircuit;
}

void GeneticExpressionStep::setTimeStep(double timeStep) {
	_timeStep = timeStep;
}

double GeneticExpressionStep::getTimeStep() const {
	return _timeStep;
}

void GeneticExpressionStep::setApplyRegulation(bool applyRegulation) {
	_applyRegulation = applyRegulation;
}

bool GeneticExpressionStep::getApplyRegulation() const {
	return _applyRegulation;
}

void GeneticExpressionStep::setLastSucceeded(bool lastSucceeded) {
	_lastSucceeded = lastSucceeded;
}

bool GeneticExpressionStep::getLastSucceeded() const {
	return _lastSucceeded;
}

void GeneticExpressionStep::setLastTotalExpression(double lastTotalExpression) {
	_lastTotalExpression = lastTotalExpression;
}

double GeneticExpressionStep::getLastTotalExpression() const {
	return _lastTotalExpression;
}

void GeneticExpressionStep::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string GeneticExpressionStep::getLastMessage() const {
	return _lastMessage;
}

void GeneticExpressionStep::_createReportStatisticsDataDefinitions() {
}

void GeneticExpressionStep::_createEditableDataDefinitions() {
}

void GeneticExpressionStep::_createOthersDataDefinitions() {
}
