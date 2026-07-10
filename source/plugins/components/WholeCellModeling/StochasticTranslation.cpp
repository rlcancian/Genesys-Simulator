#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &StochasticTranslation::GetPluginInformation;
}
#endif

ModelDataDefinition* StochasticTranslation::NewInstance(Model* model, std::string name) {
	return new StochasticTranslation(model, name);
}

StochasticTranslation::StochasticTranslation(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<StochasticTranslation>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&StochasticTranslation::getWholeCellState, this),
			std::bind(&StochasticTranslation::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "WholeCellState", "");
	auto* propElong = new SimulationControlDouble(
			std::bind(&StochasticTranslation::getElongationRate, this),
			std::bind(&StochasticTranslation::setElongationRate, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "ElongationRate", "");
	auto* propProtLen = new SimulationControlDouble(
			std::bind(&StochasticTranslation::getMeanProteinLength, this),
			std::bind(&StochasticTranslation::setMeanProteinLength, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "MeanProteinLength", "");
	auto* propWindow = new SimulationControlDouble(
			std::bind(&StochasticTranslation::getTimeWindow, this),
			std::bind(&StochasticTranslation::setTimeWindow, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "TimeWindow", "");
	auto* propMRNAPrefix = new SimulationControlString(
			std::bind(&StochasticTranslation::getMRNASpeciesPrefix, this),
			std::bind(&StochasticTranslation::setMRNASpeciesPrefix, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "MRNASpeciesPrefix", "");
	auto* propProtPrefix = new SimulationControlString(
			std::bind(&StochasticTranslation::getProteinSpeciesPrefix, this),
			std::bind(&StochasticTranslation::setProteinSpeciesPrefix, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "ProteinSpeciesPrefix", "");
	auto* propRiboKey = new SimulationControlString(
			std::bind(&StochasticTranslation::getRibosomeCountKey, this),
			std::bind(&StochasticTranslation::setRibosomeCountKey, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "RibosomeCountKey", "");
	auto* propSeed = new SimulationControlUInt(
			std::bind(&StochasticTranslation::getRandomSeed, this),
			std::bind(&StochasticTranslation::setRandomSeed, this, std::placeholders::_1),
			Util::TypeOf<StochasticTranslation>(), getName(), "RandomSeed", "");
	auto* propLastSynth = new SimulationControlInt(
			std::bind(&StochasticTranslation::getLastSynthesizedCount, this),
			nullptr,
			Util::TypeOf<StochasticTranslation>(), getName(), "LastSynthesizedCount", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propElong);
	_parentModel->getControls()->insert(propProtLen);
	_parentModel->getControls()->insert(propWindow);
	_parentModel->getControls()->insert(propMRNAPrefix);
	_parentModel->getControls()->insert(propProtPrefix);
	_parentModel->getControls()->insert(propRiboKey);
	_parentModel->getControls()->insert(propSeed);
	_parentModel->getControls()->insert(propLastSynth);

	_addSimulationControl(propState);
	_addSimulationControl(propElong);
	_addSimulationControl(propProtLen);
	_addSimulationControl(propWindow);
	_addSimulationControl(propMRNAPrefix);
	_addSimulationControl(propProtPrefix);
	_addSimulationControl(propRiboKey);
	_addSimulationControl(propSeed);
	_addSimulationControl(propLastSynth);
}

PluginInformation* StochasticTranslation::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<StochasticTranslation>(), &StochasticTranslation::LoadInstance, &StochasticTranslation::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Stochastic protein synthesis via Poisson tau-leaping for whole-cell simulations. "
		"Models the M. genitalium Translation process (Karr et al. 2012). "
		"Each mRNA template (prefix proteinSpeciesPrefix) is matched to its mRNA species "
		"(prefix mRNASpeciesPrefix) in WholeCellState. Protein synthesis count per gene "
		"is a Poisson draw with mean proportional to ribosome elongation rate, free ribosome count, "
		"mRNA abundance, and the inverse of mean protein length.");
	return info;
}

ModelComponent* StochasticTranslation::LoadInstance(Model* model, PersistenceRecord* fields) {
	StochasticTranslation* newComponent = new StochasticTranslation(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load StochasticTranslation: " + std::string(e.what()));
	}
	return newComponent;
}

std::string StochasticTranslation::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",elongationRate=" + std::to_string(_elongationRate) +
			",meanProteinLength=" + std::to_string(_meanProteinLength) +
			",timeWindow=" + std::to_string(_timeWindow) +
			",mRNASpeciesPrefix=\"" + _mRNASpeciesPrefix + "\"" +
			",proteinSpeciesPrefix=\"" + _proteinSpeciesPrefix + "\"" +
			",ribosomeCountKey=\"" + _ribosomeCountKey + "\"" +
			",randomSeed=" + std::to_string(_randomSeed) +
			",lastSynthesizedCount=" + std::to_string(_lastSynthesizedCount);
}

bool StochasticTranslation::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_elongationRate       = fields->loadField("elongationRate",     DEFAULT.elongationRate);
		_meanProteinLength    = fields->loadField("meanProteinLength",  DEFAULT.meanProteinLength);
		_timeWindow           = fields->loadField("timeWindow",         DEFAULT.timeWindow);
		_mRNASpeciesPrefix    = fields->loadField("mRNASpeciesPrefix",  DEFAULT.mRNASpeciesPrefix);
		_proteinSpeciesPrefix = fields->loadField("proteinSpeciesPrefix", DEFAULT.proteinSpeciesPrefix);
		_ribosomeCountKey     = fields->loadField("ribosomeCountKey",   DEFAULT.ribosomeCountKey);
		_randomSeed           = fields->loadField("randomSeed",         DEFAULT.randomSeed);
		_rng.seed(_randomSeed);
	}
	return res;
}

void StochasticTranslation::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState",       _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("elongationRate",       _elongationRate,       DEFAULT.elongationRate,       saveDefaultValues);
	fields->saveField("meanProteinLength",    _meanProteinLength,    DEFAULT.meanProteinLength,    saveDefaultValues);
	fields->saveField("timeWindow",           _timeWindow,           DEFAULT.timeWindow,           saveDefaultValues);
	fields->saveField("mRNASpeciesPrefix",    _mRNASpeciesPrefix,    DEFAULT.mRNASpeciesPrefix,    saveDefaultValues);
	fields->saveField("proteinSpeciesPrefix", _proteinSpeciesPrefix, DEFAULT.proteinSpeciesPrefix, saveDefaultValues);
	fields->saveField("ribosomeCountKey",     _ribosomeCountKey,     DEFAULT.ribosomeCountKey,     saveDefaultValues);
	fields->saveField("randomSeed",           _randomSeed,           DEFAULT.randomSeed,           saveDefaultValues);
}

bool StochasticTranslation::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_elongationRate <= 0.0) {
		errorMessage += "StochasticTranslation \"" + getName() + "\" elongationRate must be > 0. ";
		resultAll = false;
	}
	if (_meanProteinLength <= 0.0) {
		errorMessage += "StochasticTranslation \"" + getName() + "\" meanProteinLength must be > 0. ";
		resultAll = false;
	}
	if (_timeWindow <= 0.0) {
		errorMessage += "StochasticTranslation \"" + getName() + "\" timeWindow must be > 0. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void StochasticTranslation::_initBetweenReplications() {
	_rng.seed(_randomSeed);
	_lastSynthesizedCount = 0;
}

void StochasticTranslation::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"StochasticTranslation \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	// Free ribosome count from WholeCellState
	const int ribosomeFree = _wholeCellState->getMoleculeCount(_ribosomeCountKey);
	if (ribosomeFree <= 0) {
		_lastSynthesizedCount = 0;
		_forwardEntity(entity);
		return;
	}

	// Collect mRNA species and map each to its protein counterpart
	// Convention: mRNA_ prefix → prot_ prefix, same gene suffix
	const std::size_t mrnaPrefixLen  = _mRNASpeciesPrefix.size();
	const std::size_t protPrefixLen  = _proteinSpeciesPrefix.size();
	(void)protPrefixLen;

	struct GeneEntry { std::string mrnaKey; std::string protKey; int mrnaCount; };
	std::vector<GeneEntry> genes;
	int totalMRNA = 0;
	for (const auto& [species, count] : _wholeCellState->getMoleculeCounts()) {
		if (species.rfind(_mRNASpeciesPrefix, 0u) == 0u) {
			const std::string geneSuffix = species.substr(mrnaPrefixLen);
			genes.push_back({species, _proteinSpeciesPrefix + geneSuffix, count});
			totalMRNA += count;
		}
	}

	if (genes.empty() || totalMRNA == 0) {
		_lastSynthesizedCount = 0;
		_forwardEntity(entity);
		return;
	}

	// Poisson tau-leaping for each gene:
	//   λ_i = (elongRate / protLen) × mRNA_i × ribosome_free / max(1, totalMRNA) × Δt
	const double riboFrac = static_cast<double>(ribosomeFree) / static_cast<double>(std::max(1, totalMRNA));
	const double baseRate = (_elongationRate / _meanProteinLength) * riboFrac * _timeWindow;

	int totalSynthesized = 0;
	for (const GeneEntry& gene : genes) {
		if (gene.mrnaCount <= 0) continue;
		const double lambda = baseRate * static_cast<double>(gene.mrnaCount);
		if (lambda <= 0.0) continue;
		std::poisson_distribution<int> poisson(lambda);
		const int nNew = poisson(_rng);
		if (nNew > 0) {
			const int current = _wholeCellState->getMoleculeCount(gene.protKey);
			_wholeCellState->setMoleculeCount(gene.protKey, current + nNew);
			totalSynthesized += nNew;
		}
	}
	_lastSynthesizedCount = totalSynthesized;

	traceSimulation(this, TraceManager::Level::L2_results,
		"StochasticTranslation \"" + getName() + "\": synthesized " +
		std::to_string(totalSynthesized) + " proteins across " +
		std::to_string(genes.size()) + " genes (window=" + std::to_string(_timeWindow) + " s)");

	_forwardEntity(entity);
}

void StochasticTranslation::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void StochasticTranslation::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getFrontConnection();
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "StochasticTranslation: invalid front connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void StochasticTranslation::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* StochasticTranslation::getWholeCellState() const      { return _wholeCellState; }
void StochasticTranslation::setElongationRate(double rate)            { _elongationRate = rate; }
double StochasticTranslation::getElongationRate() const               { return _elongationRate; }
void StochasticTranslation::setMeanProteinLength(double length)       { _meanProteinLength = length; }
double StochasticTranslation::getMeanProteinLength() const            { return _meanProteinLength; }
void StochasticTranslation::setTimeWindow(double window)              { _timeWindow = window; }
double StochasticTranslation::getTimeWindow() const                   { return _timeWindow; }
void StochasticTranslation::setMRNASpeciesPrefix(std::string prefix)  { _mRNASpeciesPrefix = std::move(prefix); }
std::string StochasticTranslation::getMRNASpeciesPrefix() const       { return _mRNASpeciesPrefix; }
void StochasticTranslation::setProteinSpeciesPrefix(std::string prefix){ _proteinSpeciesPrefix = std::move(prefix); }
std::string StochasticTranslation::getProteinSpeciesPrefix() const    { return _proteinSpeciesPrefix; }
void StochasticTranslation::setRibosomeCountKey(std::string key)      { _ribosomeCountKey = std::move(key); }
std::string StochasticTranslation::getRibosomeCountKey() const        { return _ribosomeCountKey; }
void StochasticTranslation::setRandomSeed(unsigned int seed)          { _randomSeed = seed; _rng.seed(seed); }
unsigned int StochasticTranslation::getRandomSeed() const             { return _randomSeed; }
int StochasticTranslation::getLastSynthesizedCount() const            { return _lastSynthesizedCount; }
