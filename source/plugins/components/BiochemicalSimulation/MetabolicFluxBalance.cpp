#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"
#include "tools/MetabolicFluxBalanceSolver.h"

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
		bool hasObjective = false;
		if (!selectedObjective.empty()) {
			auto* objectiveReaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), selectedObjective));
			if (objectiveReaction == nullptr) {
				errorMessage += "MetabolicFluxBalance \"" + getName() + "\" could not resolve objective MetabolicReaction \"" + selectedObjective + "\". ";
				resultAll = false;
			}
			hasObjective = true;
		}
		for (const std::string& reactionName : _metabolicNetwork->getReactionNames()) {
			auto* reaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), reactionName));
			if (reaction != nullptr && std::fabs(reaction->getObjectiveCoefficient()) > 1e-12) {
				hasObjective = true;
				break;
			}
		}
		if (!hasObjective) {
			errorMessage += "MetabolicFluxBalance \"" + getName() + "\" requires an objective reaction name or non-zero objective coefficients on MetabolicReaction definitions. ";
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

	const std::vector<std::string>& reactionNames = _metabolicNetwork->getReactionNames();
	const std::vector<std::string>& exchangeSpeciesNames = _metabolicNetwork->getExchangeSpeciesNames();
	const std::string selectedObjective = _objectiveReactionName.empty() ? _metabolicNetwork->getObjectiveReactionName() : _objectiveReactionName;

	std::vector<MetabolicReaction*> reactions;
	reactions.reserve(reactionNames.size());
	for (const std::string& reactionName : reactionNames) {
		auto* reaction = dynamic_cast<MetabolicReaction*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MetabolicReaction>(), reactionName));
		if (reaction == nullptr) {
			_lastSucceeded = false;
			_lastObjectiveValue = 0.0;
			_lastMessage = "MetabolicFluxBalance could not resolve MetabolicReaction \"" + reactionName + "\".";
			traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
			_forwardEntity(entity);
			return;
		}
		reactions.push_back(reaction);
	}

	std::vector<std::string> internalSpeciesNames;
	for (MetabolicReaction* reaction : reactions) {
		for (const MetabolicReaction::StoichiometricTerm& reactant : reaction->getReactants()) {
			if (std::find(exchangeSpeciesNames.begin(), exchangeSpeciesNames.end(), reactant.speciesName) == exchangeSpeciesNames.end() &&
			    std::find(internalSpeciesNames.begin(), internalSpeciesNames.end(), reactant.speciesName) == internalSpeciesNames.end()) {
				internalSpeciesNames.push_back(reactant.speciesName);
			}
		}
		for (const MetabolicReaction::StoichiometricTerm& product : reaction->getProducts()) {
			if (std::find(exchangeSpeciesNames.begin(), exchangeSpeciesNames.end(), product.speciesName) == exchangeSpeciesNames.end() &&
			    std::find(internalSpeciesNames.begin(), internalSpeciesNames.end(), product.speciesName) == internalSpeciesNames.end()) {
				internalSpeciesNames.push_back(product.speciesName);
			}
		}
	}

	MetabolicFluxBalanceSolver::Problem problem;
	problem.stoichiometry.assign(internalSpeciesNames.size(), std::vector<double>(reactions.size(), 0.0));
	problem.lowerBounds.reserve(reactions.size());
	problem.upperBounds.reserve(reactions.size());
	problem.objective.assign(reactions.size(), 0.0);
	problem.maximize = _metabolicNetwork->getObjectiveSense() != "Minimize";

	for (std::size_t j = 0; j < reactions.size(); ++j) {
		MetabolicReaction* reaction = reactions[j];
		problem.lowerBounds.push_back(reaction->getLowerBound());
		problem.upperBounds.push_back(reaction->getUpperBound());
		for (const MetabolicReaction::StoichiometricTerm& reactant : reaction->getReactants()) {
			auto it = std::find(internalSpeciesNames.begin(), internalSpeciesNames.end(), reactant.speciesName);
			if (it != internalSpeciesNames.end()) {
				problem.stoichiometry[std::distance(internalSpeciesNames.begin(), it)][j] -= reactant.stoichiometry;
			}
		}
		for (const MetabolicReaction::StoichiometricTerm& product : reaction->getProducts()) {
			auto it = std::find(internalSpeciesNames.begin(), internalSpeciesNames.end(), product.speciesName);
			if (it != internalSpeciesNames.end()) {
				problem.stoichiometry[std::distance(internalSpeciesNames.begin(), it)][j] += product.stoichiometry;
			}
		}
		if (!selectedObjective.empty() && reaction->getName() == selectedObjective) {
			problem.objective[j] = 1.0;
		} else if (selectedObjective.empty()) {
			problem.objective[j] = reaction->getObjectiveCoefficient();
		}
	}

	bool hasObjective = false;
	for (double coefficient : problem.objective) {
		if (std::fabs(coefficient) > 1e-12) {
			hasObjective = true;
			break;
		}
	}
	if (!hasObjective) {
		_lastSucceeded = false;
		_lastObjectiveValue = 0.0;
		_lastMessage = "MetabolicFluxBalance could not build a non-zero objective vector.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	MetabolicFluxBalanceSolver::Solution solution = MetabolicFluxBalanceSolver::solve(problem);
	if (!solution.feasible) {
		_lastSucceeded = false;
		_lastObjectiveValue = 0.0;
		_lastMessage = solution.errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "MetabolicFluxBalance failed for MetabolicNetwork \"" + _metabolicNetwork->getName() + "\": " + solution.errorMessage);
		_forwardEntity(entity);
		return;
	}

	_lastObjectiveValue = solution.objectiveValue;
	_lastSucceeded = true;
	std::ostringstream payload;
	payload << "{";
	payload << "\"objectiveValue\":" << Util::StrTruncIfInt(std::to_string(_lastObjectiveValue));
	payload << ",\"fluxes\":{";
	for (std::size_t i = 0; i < reactionNames.size(); ++i) {
		payload << "\"" << reactionNames[i] << "\":" << Util::StrTruncIfInt(std::to_string(solution.fluxes[i]));
		if (i + 1u < reactionNames.size()) {
			payload << ",";
		}
	}
	payload << "}}";
	_lastMessage = payload.str();
	traceSimulation(this, TraceManager::Level::L2_results,
	                "MetabolicFluxBalance solved objective \"" + (selectedObjective.empty() ? std::string("weighted-objective") : selectedObjective) +
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
