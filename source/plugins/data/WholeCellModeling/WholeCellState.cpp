#include "plugins/data/WholeCellModeling/WholeCellState.h"

#include <fstream>
#include <functional>
#include <stdexcept>

#include "tools/Biochemical/WholeCellParameterReader.h"

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &WholeCellState::GetPluginInformation;
}
#endif

namespace {

std::string makeCompartmentMetaboliteKey(const std::string& compartmentName, const std::string& metaboliteName) {
	return compartmentName + "::" + metaboliteName;
}

} // namespace

ModelDataDefinition* WholeCellState::NewInstance(Model* model, std::string name) {
	return new WholeCellState(model, name);
}

WholeCellState::WholeCellState(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<WholeCellState>(), name) {
	auto* propCellVolume = new SimulationControlDouble(
			std::bind(&WholeCellState::getCellVolume, this),
			std::bind(&WholeCellState::setCellVolume, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "CellVolume", "");
	auto* propCellMass = new SimulationControlDouble(
			std::bind(&WholeCellState::getCellMass, this),
			std::bind(&WholeCellState::setCellMass, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "CellMass", "");
	auto* propCurrentTime = new SimulationControlDouble(
			std::bind(&WholeCellState::getCurrentTime, this),
			std::bind(&WholeCellState::setCurrentTime, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "CurrentTime", "");
	auto* propStepCount = new SimulationControlInt(
			std::bind(&WholeCellState::getStepCount, this),
			std::bind(&WholeCellState::setStepCount, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "StepCount", "");
	auto* propLifecyclePhase = new SimulationControlGeneric<std::string>(
			std::bind(&WholeCellState::getLifecyclePhase, this),
			std::bind(&WholeCellState::setLifecyclePhase, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "LifecyclePhase", "");
	auto* propGenerationCount = new SimulationControlInt(
			std::bind(&WholeCellState::getGenerationCount, this),
			std::bind(&WholeCellState::setGenerationCount, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "GenerationCount", "");
	auto* propViable = new SimulationControlBool(
			std::bind(&WholeCellState::isViable, this),
			std::bind(&WholeCellState::setViable, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "Viable", "");
	auto* propLastDivisionTime = new SimulationControlDouble(
			std::bind(&WholeCellState::getLastDivisionTime, this),
			std::bind(&WholeCellState::setLastDivisionTime, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "LastDivisionTime", "");
	auto* propFixedConstantsPath = new SimulationControlGeneric<std::string>(
			std::bind(&WholeCellState::getFixedConstantsPath, this),
			std::bind(&WholeCellState::setFixedConstantsPath, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "FixedConstantsPath", "");
	auto* propParametersPath = new SimulationControlGeneric<std::string>(
			std::bind(&WholeCellState::getParametersPath, this),
			std::bind(&WholeCellState::setParametersPath, this, std::placeholders::_1),
			Util::TypeOf<WholeCellState>(), getName(), "ParametersPath", "");

	_parentModel->getControls()->insert(propCellVolume);
	_parentModel->getControls()->insert(propCellMass);
	_parentModel->getControls()->insert(propCurrentTime);
	_parentModel->getControls()->insert(propStepCount);
	_parentModel->getControls()->insert(propLifecyclePhase);
	_parentModel->getControls()->insert(propGenerationCount);
	_parentModel->getControls()->insert(propViable);
	_parentModel->getControls()->insert(propLastDivisionTime);
	_parentModel->getControls()->insert(propFixedConstantsPath);
	_parentModel->getControls()->insert(propParametersPath);

	_addSimulationControl(propCellVolume);
	_addSimulationControl(propCellMass);
	_addSimulationControl(propCurrentTime);
	_addSimulationControl(propStepCount);
	_addSimulationControl(propLifecyclePhase);
	_addSimulationControl(propGenerationCount);
	_addSimulationControl(propViable);
	_addSimulationControl(propLastDivisionTime);
	_addSimulationControl(propFixedConstantsPath);
	_addSimulationControl(propParametersPath);
}

PluginInformation* WholeCellState::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<WholeCellState>(), &WholeCellState::LoadInstance, &WholeCellState::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setDescriptionHelp(
			"Shared state container for whole-cell model simulations. Holds discrete molecule counts "
			"(for SSA), continuous metabolite pool (for FBA), resource budget (for fair allocation), "
			"cell geometry, and lifecycle metadata. Analogous to the Vivarium Store.");
	return info;
}

ModelDataDefinition* WholeCellState::LoadInstance(Model* model, PersistenceRecord* fields) {
	WholeCellState* newElement = new WholeCellState(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load WholeCellState instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string WholeCellState::show() {
	return ModelDataDefinition::show() +
			",cellVolume=" + std::to_string(_cellVolume) +
			",cellMass=" + std::to_string(_cellMass) +
			",currentTime=" + std::to_string(_currentTime) +
			",stepCount=" + std::to_string(_stepCount) +
			",lifecyclePhase=\"" + _lifecyclePhase + "\"" +
			",generationCount=" + std::to_string(_generationCount) +
			",viable=" + std::string(_viable ? "true" : "false") +
			",lastDivisionTime=" + std::to_string(_lastDivisionTime) +
			",moleculeSpeciesCount=" + std::to_string(_moleculeCounts.size()) +
			",metaboliteCount=" + std::to_string(_metabolitePool.size()) +
			",compartmentMetaboliteCount=" + std::to_string(_compartmentMetabolitePool.size()) +
			",pathwayActivityCount=" + std::to_string(_pathwayActivities.size());
}

bool WholeCellState::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_cellVolume          = fields->loadField("cellVolume",          DEFAULT.cellVolume);
			_cellMass            = fields->loadField("cellMass",            DEFAULT.cellMass);
			_currentTime         = fields->loadField("currentTime",         DEFAULT.currentTime);
			_stepCount           = static_cast<int>(fields->loadField("stepCount", static_cast<unsigned int>(DEFAULT.stepCount)));
			_lifecyclePhase      = fields->loadField("lifecyclePhase",      DEFAULT.lifecyclePhase);
			_generationCount     = static_cast<int>(fields->loadField("generationCount", static_cast<unsigned int>(DEFAULT.generationCount)));
			_viable              = fields->loadField("viable",              DEFAULT.viable);
			_lastDivisionTime    = fields->loadField("lastDivisionTime",    DEFAULT.lastDivisionTime);
			_fixedConstantsPath  = fields->loadField("fixedConstantsPath",  DEFAULT.fixedConstantsPath);
			_parametersPath      = fields->loadField("parametersPath",      DEFAULT.parametersPath);

		_moleculeCounts.clear();
		const unsigned int moleculeCount = fields->loadField("moleculeSpeciesCount", 0u);
		for (unsigned int i = 0; i < moleculeCount; ++i) {
			const std::string speciesName = fields->loadField("moleculeSpeciesName" + Util::StrIndex(i), std::string(""));
			const int count = static_cast<int>(fields->loadField("moleculeSpeciesCount" + Util::StrIndex(i), 0u));
			if (!speciesName.empty()) {
				_moleculeCounts[speciesName] = count;
			}
		}
			_initialMoleculeCounts = _moleculeCounts;

		_metabolitePool.clear();
		const unsigned int metaboliteCount = fields->loadField("metaboliteCount", 0u);
		for (unsigned int i = 0; i < metaboliteCount; ++i) {
			const std::string metaboliteName = fields->loadField("metaboliteName" + Util::StrIndex(i), std::string(""));
			const double amount = fields->loadField("metaboliteAmount" + Util::StrIndex(i), 0.0);
			if (!metaboliteName.empty()) {
				_metabolitePool[metaboliteName] = amount;
			}
		}
			_initialMetabolitePool = _metabolitePool;

		_compartmentMetabolitePool.clear();
		const unsigned int compartmentMetaboliteCount = fields->loadField("compartmentMetaboliteCount", 0u);
		for (unsigned int i = 0; i < compartmentMetaboliteCount; ++i) {
			const std::string key = fields->loadField("compartmentMetaboliteKey" + Util::StrIndex(i), std::string(""));
			const double amount = fields->loadField("compartmentMetaboliteAmount" + Util::StrIndex(i), 0.0);
			if (!key.empty()) {
				_compartmentMetabolitePool[key] = amount;
			}
		}
			_initialCompartmentMetabolitePool = _compartmentMetabolitePool;

		_pathwayActivities.clear();
		const unsigned int pathwayActivityCount = fields->loadField("pathwayActivityCount", 0u);
		for (unsigned int i = 0; i < pathwayActivityCount; ++i) {
			const std::string pathwayName = fields->loadField("pathwayActivityName" + Util::StrIndex(i), std::string(""));
			const double activity = fields->loadField("pathwayActivityValue" + Util::StrIndex(i), 0.0);
			if (!pathwayName.empty()) {
				_pathwayActivities[pathwayName] = activity;
			}
		}
			_initialPathwayActivities = _pathwayActivities;
			_initialLifecyclePhase = _lifecyclePhase;
			_initialGenerationCount = _generationCount;
			_initialViable = _viable;
			_initialLastDivisionTime = _lastDivisionTime;
		}
		return res;
	}

void WholeCellState::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("cellVolume",         _cellVolume,  DEFAULT.cellVolume,  saveDefaultValues);
	fields->saveField("cellMass",           _cellMass,    DEFAULT.cellMass,    saveDefaultValues);
	fields->saveField("currentTime",        _currentTime, DEFAULT.currentTime, saveDefaultValues);
	fields->saveField("stepCount",          static_cast<unsigned int>(_stepCount), static_cast<unsigned int>(DEFAULT.stepCount), saveDefaultValues);
	fields->saveField("lifecyclePhase",     _lifecyclePhase, DEFAULT.lifecyclePhase, saveDefaultValues);
	fields->saveField("generationCount",    static_cast<unsigned int>(_generationCount), static_cast<unsigned int>(DEFAULT.generationCount), saveDefaultValues);
	fields->saveField("viable",             _viable, DEFAULT.viable, saveDefaultValues);
	fields->saveField("lastDivisionTime",   _lastDivisionTime, DEFAULT.lastDivisionTime, saveDefaultValues);
	fields->saveField("fixedConstantsPath", _fixedConstantsPath, DEFAULT.fixedConstantsPath, saveDefaultValues);
	fields->saveField("parametersPath",     _parametersPath,     DEFAULT.parametersPath,     saveDefaultValues);

	unsigned int idx = 0u;
	fields->saveField("moleculeSpeciesCount", static_cast<unsigned int>(_moleculeCounts.size()), 0u, saveDefaultValues);
	for (const auto& [name, count] : _moleculeCounts) {
		fields->saveField("moleculeSpeciesName"  + Util::StrIndex(idx), name,                            std::string(""), saveDefaultValues);
		fields->saveField("moleculeSpeciesCount" + Util::StrIndex(idx), static_cast<unsigned int>(count), 0u,             saveDefaultValues);
		++idx;
	}

	idx = 0u;
	fields->saveField("metaboliteCount", static_cast<unsigned int>(_metabolitePool.size()), 0u, saveDefaultValues);
	for (const auto& [name, amount] : _metabolitePool) {
		fields->saveField("metaboliteName"   + Util::StrIndex(idx), name,   std::string(""), saveDefaultValues);
		fields->saveField("metaboliteAmount" + Util::StrIndex(idx), amount, 0.0,             saveDefaultValues);
		++idx;
	}

	idx = 0u;
	fields->saveField("compartmentMetaboliteCount", static_cast<unsigned int>(_compartmentMetabolitePool.size()), 0u, saveDefaultValues);
	for (const auto& [key, amount] : _compartmentMetabolitePool) {
		fields->saveField("compartmentMetaboliteKey" + Util::StrIndex(idx), key, std::string(""), saveDefaultValues);
		fields->saveField("compartmentMetaboliteAmount" + Util::StrIndex(idx), amount, 0.0, saveDefaultValues);
		++idx;
	}

	idx = 0u;
	fields->saveField("pathwayActivityCount", static_cast<unsigned int>(_pathwayActivities.size()), 0u, saveDefaultValues);
	for (const auto& [pathwayName, activity] : _pathwayActivities) {
		fields->saveField("pathwayActivityName" + Util::StrIndex(idx), pathwayName, std::string(""), saveDefaultValues);
		fields->saveField("pathwayActivityValue" + Util::StrIndex(idx), activity, 0.0, saveDefaultValues);
		++idx;
	}
}

bool WholeCellState::_check(std::string& errorMessage) {
	// Snapshot programmatically-set state as initial values for _initBetweenReplications.
	// Only take the snapshot once (first _check call) so that later checks during a
	// running simulation do not overwrite the baseline.
	if (_initialMoleculeCounts.empty() && !_moleculeCounts.empty()) {
		_initialMoleculeCounts = _moleculeCounts;
	}
	if (_initialMetabolitePool.empty() && !_metabolitePool.empty()) {
		_initialMetabolitePool = _metabolitePool;
	}
	if (_initialCompartmentMetabolitePool.empty() && !_compartmentMetabolitePool.empty()) {
		_initialCompartmentMetabolitePool = _compartmentMetabolitePool;
	}
	if (_initialPathwayActivities.empty() && !_pathwayActivities.empty()) {
		_initialPathwayActivities = _pathwayActivities;
	}
	// Geometry snapshot: always overwrite from current values because _check is called
	// once before the first replication, at which point geometry reflects the user setup.
	_initialCellMass   = _cellMass;
	_initialCellVolume = _cellVolume;
	_initialLifecyclePhase = _lifecyclePhase;
	_initialGenerationCount = _generationCount;
	_initialViable = _viable;
	_initialLastDivisionTime = _lastDivisionTime;

	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "WholeCellState must define a non-empty name. ";
		resultAll = false;
	}
	if (_cellVolume < 0.0) {
		errorMessage += "WholeCellState \"" + getName() + "\" cellVolume must be >= 0. ";
		resultAll = false;
	}
	if (_cellMass < 0.0) {
		errorMessage += "WholeCellState \"" + getName() + "\" cellMass must be >= 0. ";
		resultAll = false;
	}
	if (_generationCount < 0) {
		errorMessage += "WholeCellState \"" + getName() + "\" generationCount must be >= 0. ";
		resultAll = false;
	}
	return resultAll;
}

void WholeCellState::_initBetweenReplications() {
	_moleculeCounts  = _initialMoleculeCounts;
	_metabolitePool  = _initialMetabolitePool;
	_compartmentMetabolitePool = _initialCompartmentMetabolitePool;
	_pathwayActivities = _initialPathwayActivities;
	_resourceBudget.clear();
	_currentTime = DEFAULT.currentTime;
	_stepCount   = DEFAULT.stepCount;
	_lifecyclePhase = _initialLifecyclePhase;
	_generationCount = _initialGenerationCount;
	_viable = _initialViable;
	_lastDivisionTime = _initialLastDivisionTime;
	// Restore cell geometry so mass-based division triggers reproduce across replications.
	// _initial* fields are snapshotted in _check() when values are set programmatically.
	_cellMass   = _initialCellMass;
	_cellVolume = _initialCellVolume;
}

bool WholeCellState::loadFixedConstants(const std::string& jsonPath) {
	// fixedConstants.json carries structural metadata (compartments, arrays) but
	// most scalar values are in parameters.json. Validate accessibility only.
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		traceError("WholeCellState \"" + getName() + "\": cannot open fixedConstants file: " + jsonPath);
		return false;
	}
	traceSimulation(this, "WholeCellState: fixedConstants file accessible: " + jsonPath);
	return true;
}

bool WholeCellState::loadParameters(const std::string& jsonPath) {
	const auto params = WholeCellParameterReader::read(jsonPath);
	if (params.empty()) {
		traceError("WholeCellState \"" + getName() + "\": cannot read or parse parameters file: " + jsonPath);
		return false;
	}

	auto get = [&](const std::string& key, double defaultVal) -> double {
		auto it = params.find(key);
		return (it != params.end()) ? it->second : defaultVal;
	};

	// Cell geometry (states.Geometry.density in kg/m^3)
	const double density = get("states.Geometry.density", 1100.0);

	// Cell initial dry mass (states.Mass.cellInitialDryWeight in grams)
	const double dryWeight_g = get("states.Mass.cellInitialDryWeight", 3.93e-15);
	const double wetFraction  = get("states.Mass.fractionWetWeight",     0.7);
	_cellMass   = dryWeight_g / (1.0 - wetFraction);  // total wet mass in grams
	// Volume from mass / density  (convert g→kg: ÷1e3; density in kg/m^3; result in m^3 → fL: ×1e15)
	_cellVolume = (_cellMass * 1e-3) / density * 1e15;  // femtoliters (1 fL = 1e-15 L)

	// Store M. genitalium kinetic parameters as named entries in metabolite pool
	// so downstream components can read them without hard-coding values.
	_metabolitePool["param.rnaPolymeraseElongationRate"] = get("processes.Transcription.rnaPolymeraseElongationRate", 50.0);
	_metabolitePool["param.ribosomeElongationRate"]      = get("processes.Translation.ribosomeElongationRate",       16.0);
	_metabolitePool["param.tmRNABindingProbability"]     = get("processes.Translation.tmRNABindingProbability",       1e-5);
	_metabolitePool["param.ftsZActivationFwd"]           = get("processes.FtsZPolymerization.activationFwd",          1.1);
	_metabolitePool["param.ftsZNucleationFwd"]           = get("processes.FtsZPolymerization.nucleationFwd",    4200000.0);
	_metabolitePool["param.cytokinesisFtsZGtpHydrolysis"]= get("processes.Cytokinesis.rateFtsZGtpHydrolysis",        0.15);
	_metabolitePool["param.cellDivisionMassThreshold"]   = _cellMass * 2.0;  // doubles before division

	_initialMetabolitePool = _metabolitePool;

	traceSimulation(this, "WholeCellState \"" + getName() + "\": loaded parameters: "
		"cellMass=" + std::to_string(_cellMass) + " g, "
		"cellVolume=" + std::to_string(_cellVolume) + " fL, "
		"RNAP_elongRate=" + std::to_string(_metabolitePool["param.rnaPolymeraseElongationRate"]) + " nt/s");
	return true;
}

void WholeCellState::setMoleculeCount(const std::string& speciesName, int count) { _moleculeCounts[speciesName] = count; }
int  WholeCellState::getMoleculeCount(const std::string& speciesName) const {
	auto it = _moleculeCounts.find(speciesName);
	return (it != _moleculeCounts.end()) ? it->second : 0;
}
bool WholeCellState::hasMoleculeCount(const std::string& speciesName) const { return _moleculeCounts.count(speciesName) > 0; }
const std::map<std::string, int>& WholeCellState::getMoleculeCounts() const { return _moleculeCounts; }

void WholeCellState::setMetaboliteAmount(const std::string& metaboliteName, double amount) { _metabolitePool[metaboliteName] = amount; }
double WholeCellState::getMetaboliteAmount(const std::string& metaboliteName) const {
	auto it = _metabolitePool.find(metaboliteName);
	return (it != _metabolitePool.end()) ? it->second : 0.0;
}
const std::map<std::string, double>& WholeCellState::getMetabolitePool() const { return _metabolitePool; }

void WholeCellState::setCompartmentMetaboliteAmount(const std::string& compartmentName, const std::string& metaboliteName, double amount) {
	_compartmentMetabolitePool[makeCompartmentMetaboliteKey(compartmentName, metaboliteName)] = amount;
}

double WholeCellState::getCompartmentMetaboliteAmount(const std::string& compartmentName, const std::string& metaboliteName) const {
	auto it = _compartmentMetabolitePool.find(makeCompartmentMetaboliteKey(compartmentName, metaboliteName));
	return (it != _compartmentMetabolitePool.end()) ? it->second : 0.0;
}

bool WholeCellState::hasCompartmentMetaboliteAmount(const std::string& compartmentName, const std::string& metaboliteName) const {
	return _compartmentMetabolitePool.count(makeCompartmentMetaboliteKey(compartmentName, metaboliteName)) > 0u;
}

const std::map<std::string, double>& WholeCellState::getCompartmentMetabolitePool() const { return _compartmentMetabolitePool; }

void WholeCellState::setPathwayActivity(const std::string& pathwayName, double activity) { _pathwayActivities[pathwayName] = activity; }

double WholeCellState::getPathwayActivity(const std::string& pathwayName) const {
	auto it = _pathwayActivities.find(pathwayName);
	return (it != _pathwayActivities.end()) ? it->second : 0.0;
}

bool WholeCellState::hasPathwayActivity(const std::string& pathwayName) const {
	return _pathwayActivities.count(pathwayName) > 0u;
}

const std::map<std::string, double>& WholeCellState::getPathwayActivities() const { return _pathwayActivities; }

void WholeCellState::setResourceBudget(const std::string& speciesName, int budget) { _resourceBudget[speciesName] = budget; }
int  WholeCellState::getResourceBudget(const std::string& speciesName) const {
	auto it = _resourceBudget.find(speciesName);
	return (it != _resourceBudget.end()) ? it->second : 0;
}
void WholeCellState::clearResourceBudget() { _resourceBudget.clear(); }

void   WholeCellState::setCellVolume(double volume)  { _cellVolume = volume; }
double WholeCellState::getCellVolume() const          { return _cellVolume; }
void   WholeCellState::setCellMass(double mass)       { _cellMass = mass; }
double WholeCellState::getCellMass() const            { return _cellMass; }
void   WholeCellState::setCurrentTime(double time)    { _currentTime = time; }
double WholeCellState::getCurrentTime() const         { return _currentTime; }
void   WholeCellState::setStepCount(int stepCount)    { _stepCount = stepCount; }
int    WholeCellState::getStepCount() const           { return _stepCount; }
void   WholeCellState::incrementStep()                { ++_stepCount; }
void   WholeCellState::setLifecyclePhase(const std::string& phase) { _lifecyclePhase = phase; }
std::string WholeCellState::getLifecyclePhase() const              { return _lifecyclePhase; }
void   WholeCellState::setGenerationCount(int generationCount)     { _generationCount = generationCount; }
int    WholeCellState::getGenerationCount() const                  { return _generationCount; }
void   WholeCellState::setViable(bool viable)                      { _viable = viable; }
bool   WholeCellState::isViable() const                            { return _viable; }
void   WholeCellState::setLastDivisionTime(double time)            { _lastDivisionTime = time; }
double WholeCellState::getLastDivisionTime() const                 { return _lastDivisionTime; }

void        WholeCellState::setFixedConstantsPath(std::string path) { _fixedConstantsPath = std::move(path); }
std::string WholeCellState::getFixedConstantsPath() const           { return _fixedConstantsPath; }
void        WholeCellState::setParametersPath(std::string path)     { _parametersPath = std::move(path); }
std::string WholeCellState::getParametersPath() const               { return _parametersPath; }
