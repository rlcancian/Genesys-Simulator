#include "plugins/components/WholeCellModeling/StochasticTranscription.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &StochasticTranscription::GetPluginInformation;
}
#endif

ModelDataDefinition* StochasticTranscription::NewInstance(Model* model, std::string name) {
	return new StochasticTranscription(model, name);
}

StochasticTranscription::StochasticTranscription(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<StochasticTranscription>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&StochasticTranscription::getWholeCellState, this),
			std::bind(&StochasticTranscription::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "WholeCellState", "");
	auto* propElong = new SimulationControlDouble(
			std::bind(&StochasticTranscription::getElongationRate, this),
			std::bind(&StochasticTranscription::setElongationRate, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "ElongationRate", "");
	auto* propGeneLen = new SimulationControlDouble(
			std::bind(&StochasticTranscription::getMeanGeneLength, this),
			std::bind(&StochasticTranscription::setMeanGeneLength, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "MeanGeneLength", "");
	auto* propBindProb = new SimulationControlDouble(
			std::bind(&StochasticTranscription::getBindingProbability, this),
			std::bind(&StochasticTranscription::setBindingProbability, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "BindingProbability", "");
	auto* propWindow = new SimulationControlDouble(
			std::bind(&StochasticTranscription::getTimeWindow, this),
			std::bind(&StochasticTranscription::setTimeWindow, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "TimeWindow", "");
	auto* propPrefix = new SimulationControlString(
			std::bind(&StochasticTranscription::getMRNASpeciesPrefix, this),
			std::bind(&StochasticTranscription::setMRNASpeciesPrefix, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "MRNASpeciesPrefix", "");
	auto* propRnapKey = new SimulationControlString(
			std::bind(&StochasticTranscription::getRnapCountKey, this),
			std::bind(&StochasticTranscription::setRnapCountKey, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "RnapCountKey", "");
	auto* propSeed = new SimulationControlUInt(
			std::bind(&StochasticTranscription::getRandomSeed, this),
			std::bind(&StochasticTranscription::setRandomSeed, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranscription>(), getName(), "RandomSeed", "");
	auto* propLastSynth = new SimulationControlInt(
			std::bind(&StochasticTranscription::getLastSynthesizedCount, this),
			nullptr,
			Util::TypeOf<StochasticTranscription>(), getName(), "LastSynthesizedCount", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propElong);
	_parentModel->getControls()->insert(propGeneLen);
	_parentModel->getControls()->insert(propBindProb);
	_parentModel->getControls()->insert(propWindow);
	_parentModel->getControls()->insert(propPrefix);
	_parentModel->getControls()->insert(propRnapKey);
	_parentModel->getControls()->insert(propSeed);
	_parentModel->getControls()->insert(propLastSynth);

	_addSimulationControl(propState);
	_addSimulationControl(propElong);
	_addSimulationControl(propGeneLen);
	_addSimulationControl(propBindProb);
	_addSimulationControl(propWindow);
	_addSimulationControl(propPrefix);
	_addSimulationControl(propRnapKey);
	_addSimulationControl(propSeed);
	_addSimulationControl(propLastSynth);
}

PluginInformation* StochasticTranscription::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<StochasticTranscription>(), &StochasticTranscription::LoadInstance, &StochasticTranscription::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Stochastic mRNA synthesis via Poisson tau-leaping (Gillespie 2001) for whole-cell simulations. "
		"Models the M. genitalium Transcription process (Karr et al. 2012). "
		"Synthesized mRNA counts per gene are Poisson draws with mean proportional to "
		"RNAP elongation rate, gene length, binding probability, and free RNAP count.");
	return info;
}

ModelComponent* StochasticTranscription::LoadInstance(Model* model, PersistenceRecord* fields) {
	StochasticTranscription* newComponent = new StochasticTranscription(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load StochasticTranscription: " + std::string(e.what()));
	}
	return newComponent;
}

std::string StochasticTranscription::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",elongationRate=" + std::to_string(_elongationRate) +
			",meanGeneLength=" + std::to_string(_meanGeneLength) +
			",bindingProbability=" + std::to_string(_bindingProbability) +
			",timeWindow=" + std::to_string(_timeWindow) +
			",mRNASpeciesPrefix=\"" + _mRNASpeciesPrefix + "\"" +
			",rnapCountKey=\"" + _rnapCountKey + "\"" +
			",randomSeed=" + std::to_string(_randomSeed) +
			",lastSynthesizedCount=" + std::to_string(_lastSynthesizedCount);
}

bool StochasticTranscription::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_elongationRate     = fields->loadField("elongationRate",     DEFAULT.elongationRate);
		_meanGeneLength     = fields->loadField("meanGeneLength",     DEFAULT.meanGeneLength);
		_bindingProbability = fields->loadField("bindingProbability", DEFAULT.bindingProbability);
		_timeWindow         = fields->loadField("timeWindow",         DEFAULT.timeWindow);
		_mRNASpeciesPrefix  = fields->loadField("mRNASpeciesPrefix",  DEFAULT.mRNASpeciesPrefix);
		_rnapCountKey       = fields->loadField("rnapCountKey",       DEFAULT.rnapCountKey);
		_randomSeed         = fields->loadField("randomSeed",         DEFAULT.randomSeed);
		_rng.seed(_randomSeed);
	}
	return res;
}

void StochasticTranscription::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState",    _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("elongationRate",    _elongationRate,     DEFAULT.elongationRate,     saveDefaultValues);
	fields->saveField("meanGeneLength",    _meanGeneLength,     DEFAULT.meanGeneLength,     saveDefaultValues);
	fields->saveField("bindingProbability",_bindingProbability, DEFAULT.bindingProbability, saveDefaultValues);
	fields->saveField("timeWindow",        _timeWindow,         DEFAULT.timeWindow,         saveDefaultValues);
	fields->saveField("mRNASpeciesPrefix", _mRNASpeciesPrefix,  DEFAULT.mRNASpeciesPrefix,  saveDefaultValues);
	fields->saveField("rnapCountKey",      _rnapCountKey,       DEFAULT.rnapCountKey,       saveDefaultValues);
	fields->saveField("randomSeed",        _randomSeed,         DEFAULT.randomSeed,         saveDefaultValues);
}

bool StochasticTranscription::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_elongationRate <= 0.0) {
		errorMessage += "StochasticTranscription \"" + getName() + "\" elongationRate must be > 0. ";
		resultAll = false;
	}
	if (_meanGeneLength <= 0.0) {
		errorMessage += "StochasticTranscription \"" + getName() + "\" meanGeneLength must be > 0. ";
		resultAll = false;
	}
	if (_bindingProbability < 0.0 || _bindingProbability > 1.0) {
		errorMessage += "StochasticTranscription \"" + getName() + "\" bindingProbability must be in [0, 1]. ";
		resultAll = false;
	}
	if (_timeWindow <= 0.0) {
		errorMessage += "StochasticTranscription \"" + getName() + "\" timeWindow must be > 0. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void StochasticTranscription::_initBetweenReplications() {
	_rng.seed(_randomSeed);
	_lastSynthesizedCount = 0;
}

void StochasticTranscription::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"StochasticTranscription \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	// Free RNAP count from WholeCellState
	const int rnapFree = _wholeCellState->getMoleculeCount(_rnapCountKey);
	if (rnapFree <= 0) {
		_lastSynthesizedCount = 0;
		_forwardEntity(entity);
		return;
	}

	// Collect mRNA species from WholeCellState (identified by prefix)
	std::vector<std::string> mrnaSpecies;
	for (const auto& [species, _count] : _wholeCellState->getMoleculeCounts()) {
		if (species.rfind(_mRNASpeciesPrefix, 0u) == 0u) {
			mrnaSpecies.push_back(species);
		}
	}

	if (mrnaSpecies.empty()) {
		_lastSynthesizedCount = 0;
		_forwardEntity(entity);
		return;
	}

	// Poisson tau-leaping: λ_i = (elongRate / geneLen) × bindProb × RNAP_free × Δt / nGenes
	// λ total is distributed uniformly over genes (simplified; full model weighs by gene expression)
	const double lambdaPerGene =
		(_elongationRate / _meanGeneLength) * _bindingProbability *
		static_cast<double>(rnapFree) * _timeWindow /
		static_cast<double>(mrnaSpecies.size());

	std::poisson_distribution<int> poisson(std::max(lambdaPerGene, 1e-300));

	int totalSynthesized = 0;
	for (const std::string& species : mrnaSpecies) {
		const int nNew = poisson(_rng);
		if (nNew > 0) {
			_wholeCellState->setMoleculeCount(species, _wholeCellState->getMoleculeCount(species) + nNew);
			totalSynthesized += nNew;
		}
	}
	_lastSynthesizedCount = totalSynthesized;

	traceSimulation(this, TraceManager::Level::L2_results,
		"StochasticTranscription \"" + getName() + "\": synthesized " +
		std::to_string(totalSynthesized) + " mRNA molecules across " +
		std::to_string(mrnaSpecies.size()) + " species (window=" + std::to_string(_timeWindow) + " s)");

	_forwardEntity(entity);
}

void StochasticTranscription::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void StochasticTranscription::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getFrontConnection();
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "StochasticTranscription: invalid front connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void StochasticTranscription::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* StochasticTranscription::getWholeCellState() const      { return _wholeCellState; }
void StochasticTranscription::setElongationRate(double rate)            { _elongationRate = rate; }
double StochasticTranscription::getElongationRate() const               { return _elongationRate; }
void StochasticTranscription::setMeanGeneLength(double length)          { _meanGeneLength = length; }
double StochasticTranscription::getMeanGeneLength() const               { return _meanGeneLength; }
void StochasticTranscription::setBindingProbability(double prob)        { _bindingProbability = prob; }
double StochasticTranscription::getBindingProbability() const           { return _bindingProbability; }
void StochasticTranscription::setTimeWindow(double window)              { _timeWindow = window; }
double StochasticTranscription::getTimeWindow() const                   { return _timeWindow; }
void StochasticTranscription::setMRNASpeciesPrefix(std::string prefix)  { _mRNASpeciesPrefix = std::move(prefix); }
std::string StochasticTranscription::getMRNASpeciesPrefix() const       { return _mRNASpeciesPrefix; }
void StochasticTranscription::setRnapCountKey(std::string key)          { _rnapCountKey = std::move(key); }
std::string StochasticTranscription::getRnapCountKey() const            { return _rnapCountKey; }
void StochasticTranscription::setRandomSeed(unsigned int seed)          { _randomSeed = seed; _rng.seed(seed); }
unsigned int StochasticTranscription::getRandomSeed() const             { return _randomSeed; }
int StochasticTranscription::getLastSynthesizedCount() const            { return _lastSynthesizedCount; }
