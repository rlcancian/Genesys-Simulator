#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "plugins/data/WholeCellModeling/MolecularSpecies.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &StochasticReactionComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* StochasticReactionComponent::NewInstance(Model* model, std::string name) {
	return new StochasticReactionComponent(model, name);
}

StochasticReactionComponent::StochasticReactionComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<StochasticReactionComponent>(), name) {
	auto* propWholeCellState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&StochasticReactionComponent::getWholeCellState, this),
			std::bind(&StochasticReactionComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionComponent>(), getName(), "WholeCellState", "");
	auto* propTimeWindow = new SimulationControlDouble(
			std::bind(&StochasticReactionComponent::getTimeWindow, this),
			std::bind(&StochasticReactionComponent::setTimeWindow, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionComponent>(), getName(), "TimeWindow", "");
	auto* propAdvanceWholeCellClock = new SimulationControlBool(
			std::bind(&StochasticReactionComponent::getAdvanceWholeCellClock, this),
			std::bind(&StochasticReactionComponent::setAdvanceWholeCellClock, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionComponent>(), getName(), "AdvanceWholeCellClock", "");
	auto* propRandomSeed = new SimulationControlUInt(
			std::bind(&StochasticReactionComponent::getRandomSeed, this),
			std::bind(&StochasticReactionComponent::setRandomSeed, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionComponent>(), getName(), "RandomSeed", "");
	auto* propLastReactionCount = new SimulationControlInt(
			std::bind(&StochasticReactionComponent::getLastReactionCount, this),
			nullptr,
			Util::TypeOf<StochasticReactionComponent>(), getName(), "LastReactionCount", "");
	auto* propLastSimulatedTime = new SimulationControlDouble(
			std::bind(&StochasticReactionComponent::getLastSimulatedTime, this),
			nullptr,
			Util::TypeOf<StochasticReactionComponent>(), getName(), "LastSimulatedTime", "");

	_parentModel->getControls()->insert(propWholeCellState);
	_parentModel->getControls()->insert(propTimeWindow);
	_parentModel->getControls()->insert(propAdvanceWholeCellClock);
	_parentModel->getControls()->insert(propRandomSeed);
	_parentModel->getControls()->insert(propLastReactionCount);
	_parentModel->getControls()->insert(propLastSimulatedTime);

	_addSimulationControl(propWholeCellState);
	_addSimulationControl(propTimeWindow);
	_addSimulationControl(propAdvanceWholeCellClock);
	_addSimulationControl(propRandomSeed);
	_addSimulationControl(propLastReactionCount);
	_addSimulationControl(propLastSimulatedTime);
}

PluginInformation* StochasticReactionComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<StochasticReactionComponent>(), &StochasticReactionComponent::LoadInstance, &StochasticReactionComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
			"Runs the Gillespie Direct Method SSA (Gillespie 1977) for a configurable time window "
			"using all StochasticReactionRule definitions in the model. Reads and writes molecule "
			"counts via the referenced WholeCellState. Optionally advances WholeCellState time when "
			"no external whole-cell checkpoint component is being used.");
	return info;
}

ModelComponent* StochasticReactionComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	StochasticReactionComponent* newComponent = new StochasticReactionComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load StochasticReactionComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string StochasticReactionComponent::show() {
	return ModelComponent::show() +
				",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
				",timeWindow=" + std::to_string(_timeWindow) +
				",advanceWholeCellClock=" + std::string(_advanceWholeCellClock ? "true" : "false") +
				",randomSeed=" + std::to_string(_randomSeed) +
				",lastReactionCount=" + std::to_string(_lastReactionCount) +
			",lastSimulatedTime=" + std::to_string(_lastSimulatedTime);
}

bool StochasticReactionComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
			}
			_timeWindow  = fields->loadField("timeWindow",  DEFAULT.timeWindow);
			_advanceWholeCellClock = fields->loadField("advanceWholeCellClock", DEFAULT.advanceWholeCellClock);
			_randomSeed  = fields->loadField("randomSeed",  DEFAULT.randomSeed);
			_rng.seed(_randomSeed);
	}
	return res;
}

void StochasticReactionComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("timeWindow",  _timeWindow,  DEFAULT.timeWindow,  saveDefaultValues);
	fields->saveField("advanceWholeCellClock", _advanceWholeCellClock, DEFAULT.advanceWholeCellClock, saveDefaultValues);
	fields->saveField("randomSeed",  _randomSeed,  DEFAULT.randomSeed,  saveDefaultValues);
}

bool StochasticReactionComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_timeWindow <= 0.0) {
		errorMessage += "StochasticReactionComponent \"" + getName() + "\" timeWindow must be > 0. ";
		resultAll = false;
	}
	const std::vector<StochasticReactionRule*> rules = _collectRules();
	if (rules.empty()) {
		errorMessage += "StochasticReactionComponent \"" + getName() + "\" requires at least one StochasticReactionRule in the model. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void StochasticReactionComponent::_initBetweenReplications() {
	_rng.seed(_randomSeed);
	_lastReactionCount = 0;
	_lastSimulatedTime = 0.0;
}

void StochasticReactionComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	const std::vector<StochasticReactionRule*> rules = _collectRules();
	if (rules.empty()) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"StochasticReactionComponent \"" + getName() + "\": no StochasticReactionRule found in model.");
		_forwardEntity(entity);
		return;
	}

	std::map<std::string, int> counts;
	_collectCountsFromState(counts);

	// Resource budget is intentionally empty — no hard floor constraint. Using the initial counts
	// as the budget would block every consuming reaction (count - stoich < initial). If a minimum
	// reserved pool per species is later needed, add a dedicated field to WholeCellState.
	std::map<std::string, int> resourceBudget;

	const GillespieState result = _runGillespie(counts, rules, resourceBudget, _timeWindow);

	_applyCountsToState(result.counts);
	_lastReactionCount = result.reactionCount;
	_lastSimulatedTime = result.simulatedTime;

	if (_wholeCellState != nullptr && _advanceWholeCellClock) {
		_wholeCellState->setCurrentTime(_wholeCellState->getCurrentTime() + _timeWindow);
		_wholeCellState->incrementStep();
	}

	traceSimulation(this, TraceManager::Level::L2_results,
		"StochasticReactionComponent \"" + getName() + "\": " +
		std::to_string(result.reactionCount) + " reactions fired in " +
		std::to_string(result.simulatedTime) + " s (window=" + std::to_string(_timeWindow) + " s)");

	_forwardEntity(entity);
}

StochasticReactionComponent::GillespieState StochasticReactionComponent::_runGillespie(
		const std::map<std::string, int>& initialCounts,
		const std::vector<StochasticReactionRule*>& rules,
		const std::map<std::string, int>& resourceBudget,
		double timeWindow) {

	GillespieState state;
	state.counts = initialCounts;
	state.simulatedTime = 0.0;
	state.reactionCount = 0;

	std::uniform_real_distribution<double> uniform(0.0, 1.0);

	while (state.simulatedTime < timeWindow) {
		// Step 1: compute propensities
		std::vector<double> propensities;
		propensities.reserve(rules.size());
		double totalPropensity = 0.0;
		for (StochasticReactionRule* rule : rules) {
			const double a = rule->computePropensity(state.counts);
			propensities.push_back(a);
			totalPropensity += a;
		}

		// Step 2: check for absorbing state
		if (totalPropensity <= 0.0) break;

		// Step 3: draw time to next reaction (exponential)
		const double u1 = uniform(_rng);
		const double tau = -std::log(u1 > 0.0 ? u1 : 1e-300) / totalPropensity;
		if (state.simulatedTime + tau > timeWindow) break;

		// Step 4: draw which reaction fires
		const double u2 = uniform(_rng) * totalPropensity;
		std::size_t reactionIndex = 0;
		double cumulative = 0.0;
		for (std::size_t j = 0; j < propensities.size(); ++j) {
			cumulative += propensities[j];
			if (cumulative > u2) {
				reactionIndex = j;
				break;
			}
		}

		// Step 5: check resource budget before firing
		const StochasticReactionRule* rule = rules[reactionIndex];
		bool budgetExceeded = false;
		for (const StochasticReactionRule::StoichiometricTerm& reactant : rule->getReactants()) {
			auto budgetIt = resourceBudget.find(reactant.speciesName);
			if (budgetIt != resourceBudget.end()) {
				auto countIt = state.counts.find(reactant.speciesName);
				const int current = (countIt != state.counts.end()) ? countIt->second : 0;
				if (current - reactant.stoichiometry < budgetIt->second) {
					budgetExceeded = true;
					break;
				}
			}
		}
		if (budgetExceeded) {
			// Advance time past this event but do not fire
			state.simulatedTime += tau;
			continue;
		}

		// Step 6: apply reaction
		for (const StochasticReactionRule::StoichiometricTerm& reactant : rule->getReactants()) {
			state.counts[reactant.speciesName] -= reactant.stoichiometry;
			if (state.counts[reactant.speciesName] < 0) state.counts[reactant.speciesName] = 0;
		}
		for (const StochasticReactionRule::StoichiometricTerm& product : rule->getProducts()) {
			state.counts[product.speciesName] += product.stoichiometry;
		}

		state.simulatedTime += tau;
		++state.reactionCount;
	}

	if (state.simulatedTime < timeWindow) {
		state.simulatedTime = timeWindow;
	}

	return state;
}

void StochasticReactionComponent::_collectCountsFromState(std::map<std::string, int>& counts) const {
	if (_wholeCellState != nullptr) {
		counts = _wholeCellState->getMoleculeCounts();
		return;
	}
	// Fall back to individual MolecularSpecies in model
	List<ModelDataDefinition*>* defs = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<MolecularSpecies>());
	if (defs != nullptr) {
		for (ModelDataDefinition* def : *defs->list()) {
			if (auto* species = dynamic_cast<MolecularSpecies*>(def)) {
				counts[species->getName()] = species->getCount();
			}
		}
	}
}

void StochasticReactionComponent::_applyCountsToState(const std::map<std::string, int>& counts) {
	if (_wholeCellState != nullptr) {
		for (const auto& [name, count] : counts) {
			_wholeCellState->setMoleculeCount(name, count);
		}
		return;
	}
	// Fall back to individual MolecularSpecies in model
	for (const auto& [name, count] : counts) {
		auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<MolecularSpecies>(), name);
		if (auto* species = dynamic_cast<MolecularSpecies*>(def)) {
			species->setCount(count);
		}
	}
}

std::vector<StochasticReactionRule*> StochasticReactionComponent::_collectRules() const {
	std::vector<StochasticReactionRule*> rules;
	List<ModelDataDefinition*>* defs = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<StochasticReactionRule>());
	if (defs == nullptr) return rules;
	for (ModelDataDefinition* def : *defs->list()) {
		if (auto* rule = dynamic_cast<StochasticReactionRule*>(def)) {
			rules.push_back(rule);
		}
	}
	return rules;
}

void StochasticReactionComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
	// Register all StochasticReactionRule instances from the model as optional editable (attached)
	// data. "Editable"/attached references do not transfer lifecycle ownership to this component —
	// the rules remain independently managed in the model data manager. This protects them from
	// orphan cleanup without risking double-deletion: attachedDataRemove just removes the map
	// entry, unlike internalDataRemove which calls _deleteOwnedInternalData and deletes the object.
	for (StochasticReactionRule* rule : _collectRules()) {
		_optionalEditableDataDefinitionInsert(rule->getName(), rule);
	}
}

void StochasticReactionComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "StochasticReactionComponent: invalid front connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void StochasticReactionComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* StochasticReactionComponent::getWholeCellState() const     { return _wholeCellState; }
void StochasticReactionComponent::setTimeWindow(double timeWindow)          { _timeWindow = timeWindow; }
double StochasticReactionComponent::getTimeWindow() const                   { return _timeWindow; }
void StochasticReactionComponent::setAdvanceWholeCellClock(bool advance)    { _advanceWholeCellClock = advance; }
bool StochasticReactionComponent::getAdvanceWholeCellClock() const          { return _advanceWholeCellClock; }
void StochasticReactionComponent::setRandomSeed(unsigned int seed)          { _randomSeed = seed; _rng.seed(seed); }
unsigned int StochasticReactionComponent::getRandomSeed() const             { return _randomSeed; }
int StochasticReactionComponent::getLastReactionCount() const               { return _lastReactionCount; }
double StochasticReactionComponent::getLastSimulatedTime() const            { return _lastSimulatedTime; }
