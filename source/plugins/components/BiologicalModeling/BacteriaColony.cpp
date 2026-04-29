/*
 * File:   BacteriaColony.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiologicalModeling/GroProgramCompiler.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>
#include <stdexcept>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BacteriaColony::GetPluginInformation;
}
#endif

namespace {

constexpr const char* kBioSpeciesAssignmentPrefix = "bio_species_";

struct GroSeedPlacement {
	unsigned int gridX = 0;
	unsigned int gridY = 0;
	std::string programName = "";
};

std::string toGroIdentifierSuffix(const std::string& text) {
	std::string identifier;
	identifier.reserve(text.size());
	for (char ch : text) {
		if (std::isalnum(static_cast<unsigned char>(ch)) != 0) {
			identifier += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
		} else {
			identifier += '_';
		}
	}
	if (identifier.empty()) {
		return "unnamed";
	}
	if (std::isdigit(static_cast<unsigned char>(identifier.front())) != 0) {
		identifier.insert(identifier.begin(), '_');
	}
	return identifier;
}

bool parseGroConfiguredDt(const std::string& sourceCode, double& configuredDt) {
	const std::regex dtPattern(R"(set\s*\(\s*["']dt["']\s*,\s*([-+]?[0-9]*\.?[0-9]+)\s*\))");
	std::smatch match;
	if (!std::regex_search(sourceCode, match, dtPattern) || match.size() < 2) {
		return false;
	}

	try {
		configuredDt = std::stod(match[1].str());
		return std::isfinite(configuredDt) && configuredDt > 0.0;
	} catch (const std::exception&) {
		return false;
	}
}

std::vector<GroSeedPlacement> parseGroSeedPlacements(const std::string& sourceCode) {
	const std::regex ecoliPattern(
			R"(ecoli\s*\(\s*\[\s*x\s*:?=\s*([-+]?[0-9]*\.?[0-9]+)\s*,\s*y\s*:?=\s*([-+]?[0-9]*\.?[0-9]+)\s*\]\s*,\s*program\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(\s*\)\s*\))");
	std::vector<GroSeedPlacement> placements;
	for (std::sregex_iterator it(sourceCode.begin(), sourceCode.end(), ecoliPattern), end; it != end; ++it) {
		try {
			const double xValue = std::stod((*it)[1].str());
			const double yValue = std::stod((*it)[2].str());
			if (!std::isfinite(xValue) || !std::isfinite(yValue)) {
				continue;
			}
			GroSeedPlacement placement;
			placement.gridX = static_cast<unsigned int>(std::max(0.0, std::round(xValue)));
			placement.gridY = static_cast<unsigned int>(std::max(0.0, std::round(yValue)));
			placement.programName = (*it)[3].str();
			placements.push_back(placement);
		} catch (const std::exception&) {
		}
	}
	return placements;
}

} // namespace

ModelDataDefinition* BacteriaColony::NewInstance(Model* model, std::string name) {
	return new BacteriaColony(model, name);
}

BacteriaColony::BacteriaColony(Model* model, std::string name) :
		ModelComponent(model, Util::TypeOf<BacteriaColony>(), name) {
	SimulationControlGenericClass<GroProgram*, Model*, GroProgram>* propGroProgram =
			new SimulationControlGenericClass<GroProgram*, Model*, GroProgram>(
					_parentModel,
					std::bind(&BacteriaColony::getGroProgram, this),
					std::bind(&BacteriaColony::setGroProgram, this, std::placeholders::_1),
					Util::TypeOf<BacteriaColony>(), getName(), "GroProgram",
					"Reusable Gro source program interpreted by this colony");
	SimulationControlGenericClass<BioNetwork*, Model*, BioNetwork>* propBioNetwork =
			new SimulationControlGenericClass<BioNetwork*, Model*, BioNetwork>(
					_parentModel,
					std::bind(&BacteriaColony::getBioNetwork, this),
					std::bind(&BacteriaColony::setBioNetwork, this, std::placeholders::_1),
					Util::TypeOf<BacteriaColony>(), getName(), "BioNetwork",
					"Optional BioNetwork reused as the colony biochemical state");
	SimulationControlGenericClass<BacteriaSignalGrid*, Model*, BacteriaSignalGrid>* propSignalGrid =
			new SimulationControlGenericClass<BacteriaSignalGrid*, Model*, BacteriaSignalGrid>(
					_parentModel,
					std::bind(&BacteriaColony::getSignalGrid, this),
					std::bind(&BacteriaColony::setSignalGrid, this, std::placeholders::_1),
					Util::TypeOf<BacteriaColony>(), getName(), "SignalGrid",
					"Reusable discrete signal-grid definition copied into the colony runtime state");
	SimulationControlDouble* propSimulationStep = new SimulationControlDouble(
			std::bind(&BacteriaColony::getSimulationStep, this),
			std::bind(&BacteriaColony::setSimulationStep, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "SimulationStep",
			"Internal biological time increment used by one colony step");
	SimulationControlDouble* propInitialColonyTime = new SimulationControlDouble(
			std::bind(&BacteriaColony::getInitialColonyTime, this),
			std::bind(&BacteriaColony::setInitialColonyTime, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "InitialColonyTime",
			"Initial internal biological time for each replication");
	SimulationControlDouble* propFinalColonyTime = new SimulationControlDouble(
			std::bind(&BacteriaColony::getFinalColonyTime, this),
			std::bind(&BacteriaColony::setFinalColonyTime, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "FinalColonyTime",
			"Final internal biological time reached before the current entity leaves the colony");
	SimulationControlGenericEnum<Util::TimeUnit, Util>* propColonyTimeUnit =
			new SimulationControlGenericEnum<Util::TimeUnit, Util>(
					std::bind(&BacteriaColony::getColonyTimeUnit, this),
					std::bind(&BacteriaColony::setColonyTimeUnit, this, std::placeholders::_1),
					Util::TypeOf<BacteriaColony>(), getName(), "ColonyTimeUnit",
					"Time unit used by InitialColonyTime, SimulationStep, and FinalColonyTime");
	SimulationControlGeneric<unsigned int>* propInitialPopulation = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaColony::getInitialPopulation, this),
			std::bind(&BacteriaColony::setInitialPopulation, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "InitialPopulation",
			"Initial bacteria population summary");
	SimulationControlGeneric<unsigned int>* propGridWidth = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaColony::getGridWidth, this),
			std::bind(&BacteriaColony::setGridWidth, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "GridWidth",
			"Discrete grid width reserved for colony spatial state");
	SimulationControlGeneric<unsigned int>* propGridHeight = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaColony::getGridHeight, this),
			std::bind(&BacteriaColony::setGridHeight, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "GridHeight",
			"Discrete grid height reserved for colony spatial state");

	_parentModel->getControls()->insert(propGroProgram);
	_parentModel->getControls()->insert(propBioNetwork);
	_parentModel->getControls()->insert(propSignalGrid);
	_parentModel->getControls()->insert(propSimulationStep);
	_parentModel->getControls()->insert(propInitialColonyTime);
	_parentModel->getControls()->insert(propFinalColonyTime);
	_parentModel->getControls()->insert(propColonyTimeUnit);
	_parentModel->getControls()->insert(propInitialPopulation);
	_parentModel->getControls()->insert(propGridWidth);
	_parentModel->getControls()->insert(propGridHeight);

	_addSimulationControl(propGroProgram);
	_addSimulationControl(propBioNetwork);
	_addSimulationControl(propSignalGrid);
	_addSimulationControl(propSimulationStep);
	_addSimulationControl(propInitialColonyTime);
	_addSimulationControl(propFinalColonyTime);
	_addSimulationControl(propColonyTimeUnit);
	_addSimulationControl(propInitialPopulation);
	_addSimulationControl(propGridWidth);
	_addSimulationControl(propGridHeight);
}

std::string BacteriaColony::show() {
	return ModelComponent::show() +
	       ",groProgram=\"" + (_groProgram != nullptr ? _groProgram->getName() : "") + "\"" +
	       ",bioNetwork=\"" + (_bioNetwork != nullptr ? _bioNetwork->getName() : "") + "\"" +
	       ",signalGrid=\"" + (_signalGrid != nullptr ? _signalGrid->getName() : "") + "\"" +
	       ",simulationStep=" + Util::StrTruncIfInt(std::to_string(getSimulationStep())) +
	       ",initialColonyTime=" + Util::StrTruncIfInt(std::to_string(getInitialColonyTime())) +
	       ",finalColonyTime=" + Util::StrTruncIfInt(std::to_string(getFinalColonyTime())) +
	       ",colonyTimeUnit=" + Util::convertEnumToStr(_colonyTimeUnit) +
	       ",colonyTime=" + Util::StrTruncIfInt(std::to_string(getColonyTime())) +
	       ",initialPopulation=" + std::to_string(_initialPopulation) +
	       ",populationSize=" + std::to_string(_populationSize) +
	       ",gridWidth=" + std::to_string(getGridWidth()) +
	       ",gridHeight=" + std::to_string(getGridHeight());
}

PluginInformation* BacteriaColony::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BacteriaColony>(),
	                                                &BacteriaColony::LoadInstance,
	                                                &BacteriaColony::NewInstance);
	info->setCategory("BiologicalModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("groprogram.so");
	info->insertDynamicLibFileDependence("bionetwork.so");
	info->insertDynamicLibFileDependence("bacteriasignalgrid.so");
	info->setDescriptionHelp("Self-contained biological simulation component for Gro-inspired bacteria colonies. "
	                         "When an entity arrives, the colony advances one configured Gro/runtime step and then "
	                         "forwards the entity through its output connection. This first slice owns an internal "
	                         "colony clock, population summary, optional BioNetwork, discrete grid configuration, and optional signal field while leaving complete "
	                         "Gro parsing and execution semantics to future plugin-side helpers.");
	return info;
}

ModelComponent* BacteriaColony::LoadInstance(Model* model, PersistenceRecord* fields) {
	BacteriaColony* newComponent = new BacteriaColony(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

void BacteriaColony::setGroProgram(GroProgram* groProgram) {
	_groProgram = groProgram;
	std::string runtimeConfigErrorMessage;
	(void)_refreshRuntimeConfigurationFromGroProgram(runtimeConfigErrorMessage);
}

GroProgram* BacteriaColony::getGroProgram() const {
	return _groProgram;
}

void BacteriaColony::setSimulationStep(double simulationStep) {
	_simulationStep = simulationStep;
	if (_usesBioNetworkTime()) {
		// When a BioNetwork is attached, its step size becomes the
		// authoritative biological step used by the colony as well.
		_bioNetwork->setStepSize(simulationStep);
	}
}

double BacteriaColony::getSimulationStep() const {
	return _usesBioNetworkTime() ? _bioNetwork->getStepSize() : _simulationStep;
}

void BacteriaColony::setInitialColonyTime(double initialColonyTime) {
	_initialColonyTime = initialColonyTime;
	_colonyTime = initialColonyTime;
	if (_usesBioNetworkTime()) {
		// Keep the attached BioNetwork aligned with the configured colony start.
		_bioNetwork->setStartTime(initialColonyTime);
		_bioNetwork->setCurrentTime(initialColonyTime);
	}
}

double BacteriaColony::getInitialColonyTime() const {
	return _usesBioNetworkTime() ? _bioNetwork->getStartTime() : _initialColonyTime;
}

void BacteriaColony::setFinalColonyTime(double finalColonyTime) {
	_finalColonyTime = finalColonyTime;
	if (_usesBioNetworkTime()) {
		_bioNetwork->setStopTime(finalColonyTime);
	}
}

double BacteriaColony::getFinalColonyTime() const {
	return _usesBioNetworkTime() ? _bioNetwork->getStopTime() : _finalColonyTime;
}

void BacteriaColony::setColonyTimeUnit(Util::TimeUnit colonyTimeUnit) {
	_colonyTimeUnit = colonyTimeUnit;
}

Util::TimeUnit BacteriaColony::getColonyTimeUnit() const {
	return _colonyTimeUnit;
}

double BacteriaColony::getColonyTime() const {
	return _usesBioNetworkTime() ? _bioNetwork->getCurrentTime() : _colonyTime;
}

void BacteriaColony::setInitialPopulation(unsigned int initialPopulation) {
	_initialPopulation = initialPopulation;
}

unsigned int BacteriaColony::getInitialPopulation() const {
	return _initialPopulation;
}

unsigned int BacteriaColony::getPopulationSize() const {
	return _populationSize;
}

void BacteriaColony::setBioNetwork(BioNetwork* bioNetwork) {
	_bioNetwork = bioNetwork;
	_synchronizeTemporalStateFromBioNetwork();
}

BioNetwork* BacteriaColony::getBioNetwork() const {
	return _bioNetwork;
}

void BacteriaColony::setSignalGrid(BacteriaSignalGrid* signalGrid) {
	_signalGrid = signalGrid;
	_synchronizeGridDimensionsFromSignalGrid();
	_rebuildBacteriaGridPositions();
}

BacteriaSignalGrid* BacteriaColony::getSignalGrid() const {
	return _signalGrid;
}

std::size_t BacteriaColony::getInternalBacteriaCount() const {
	return _bacteria.size();
}

const BacteriaColony::BacteriumState& BacteriaColony::getBacteriumState(std::size_t index) const {
	if (index >= _bacteria.size()) {
		throw std::out_of_range("BacteriaColony bacterium index is out of range");
	}
	return _bacteria[index];
}

double BacteriaColony::getBacteriumAge(std::size_t index) const {
	const BacteriumState& bacterium = getBacteriumState(index);
	const double colonyTime = getColonyTime();
	if (colonyTime < bacterium.birthTime) {
		return 0.0;
	}
	return colonyTime - bacterium.birthTime;
}

double BacteriaColony::getBacteriumRuntimeVariableValue(std::size_t index, const std::string& variableName) const {
	const BacteriumState& bacterium = getBacteriumState(index);
	const auto found = bacterium.runtimeVariables.find(variableName);
	if (found == bacterium.runtimeVariables.end()) {
		throw std::out_of_range("BacteriaColony runtime variable \"" + variableName +
		                        "\" is not available for bacterium index " + std::to_string(index));
	}
	return found->second;
}

bool BacteriaColony::hasBacteriumRuntimeVariable(std::size_t index, const std::string& variableName) const {
	const BacteriumState& bacterium = getBacteriumState(index);
	return bacterium.runtimeVariables.find(variableName) != bacterium.runtimeVariables.end();
}

double BacteriaColony::getRuntimeVariableValue(const std::string& variableName) const {
	const auto found = _runtimeVariables.find(variableName);
	if (found == _runtimeVariables.end()) {
		throw std::out_of_range("BacteriaColony runtime variable \"" + variableName + "\" is not available");
	}
	return found->second;
}

bool BacteriaColony::hasRuntimeVariable(const std::string& variableName) const {
	return _runtimeVariables.find(variableName) != _runtimeVariables.end();
}

double BacteriaColony::getSignalValueAt(unsigned int x, unsigned int y) const {
	if (x >= getGridWidth() || y >= getGridHeight()) {
		throw std::out_of_range("BacteriaColony signal coordinate is out of range");
	}
	return _signalValueAt(x, y);
}

double BacteriaColony::getBacteriumLocalSignal(std::size_t index) const {
	const BacteriumState& bacterium = getBacteriumState(index);
	return _signalValueAt(bacterium.gridX, bacterium.gridY);
}

void BacteriaColony::setGridWidth(unsigned int gridWidth) {
	if (_usesSignalGridDimensions()) {
		// The reusable SignalGrid owns the authoritative width/height values.
		_signalGrid->setWidth(gridWidth);
		_synchronizeGridDimensionsFromSignalGrid();
	} else {
		_gridWidth = gridWidth;
	}
	_rebuildBacteriaGridPositions();
}

unsigned int BacteriaColony::getGridWidth() const {
	return _usesSignalGridDimensions() ? _signalGrid->getWidth() : _gridWidth;
}

void BacteriaColony::setGridHeight(unsigned int gridHeight) {
	if (_usesSignalGridDimensions()) {
		// The reusable SignalGrid owns the authoritative width/height values.
		_signalGrid->setHeight(gridHeight);
		_synchronizeGridDimensionsFromSignalGrid();
	} else {
		_gridHeight = gridHeight;
	}
	_rebuildBacteriaGridPositions();
}

unsigned int BacteriaColony::getGridHeight() const {
	return _usesSignalGridDimensions() ? _signalGrid->getHeight() : _gridHeight;
}

double BacteriaColony::advanceColonyTime() {
	const double nextColonyTime = std::min(getColonyTime() + getSimulationStep(), getFinalColonyTime());
	_colonyTime = nextColonyTime;
	if (_usesBioNetworkTime()) {
		_bioNetwork->setCurrentTime(nextColonyTime);
	}
	_refreshBacteriaUpdateTime();
	return _colonyTime;
}

GroProgramRuntime::ExecutionResult BacteriaColony::executeGroProgram() {
	GroProgramRuntime::ExecutionResult result;

	if (_groProgram == nullptr) {
		result.succeeded = false;
		result.errorMessage = "BacteriaColony requires a GroProgram before execution. ";
		return result;
	}

	std::string runtimeConfigErrorMessage;
	if (!_refreshRuntimeConfigurationFromGroProgram(runtimeConfigErrorMessage)) {
		result.succeeded = false;
		result.errorMessage = runtimeConfigErrorMessage;
		return result;
	}

	GroProgramParser::Result parseResult = GroProgramParser().parse(_groProgram->getSourceCode());
	if (!parseResult.accepted) {
		result.succeeded = false;
		result.errorMessage = parseResult.errorMessage;
		return result;
	}

	GroProgramIr ir = GroProgramCompiler().compile(parseResult.ast);
	if (ir.isProgramBlock() && ir.programName == "bacterium") {
		_executeBacteriumScopedGroProgram(ir, result);
		return result;
	}
	if (!ir.namedPrograms.empty()) {
		_executeSeededNamedGroPrograms(ir, result);
		return result;
	}

	GroProgramRuntimeState runtimeState;
	std::string bioContextErrorMessage;
	const double previousColonyTime = getColonyTime();
	runtimeState.colonyTime = previousColonyTime;
	runtimeState.simulationStep = getSimulationStep();
	runtimeState.populationSize = _populationSize;
	runtimeState.variables = _runtimeVariables;
	runtimeState.variables["dt"] = runtimeState.simulationStep;
	_appendBioNetworkContextVariables(runtimeState, bioContextErrorMessage);
	if (!bioContextErrorMessage.empty()) {
		result.succeeded = false;
		result.errorMessage = bioContextErrorMessage;
		return result;
	}

	result = GroProgramRuntime().execute(ir, runtimeState);
	if (result.succeeded) {
		if (!result.signalMutations.empty()) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony aggregate Gro programs cannot mutate local signals. "
			                      "Use program bacterium() for signal-aware colony logic. ";
			return result;
		}
		if (runtimeState.simulationStep > 0.0 &&
		    std::fabs(runtimeState.simulationStep - getSimulationStep()) > 1e-12) {
			// The classic Gro source uses set("dt", ...) as the authoritative colony step.
			setSimulationStep(runtimeState.simulationStep);
		}
		if (runtimeState.colonyTime <= previousColonyTime + 1e-12) {
			runtimeState.colonyTime = std::min(previousColonyTime + getSimulationStep(), getFinalColonyTime());
		}
		if (!_applyBioNetworkAssignments(result.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			return result;
		}
		_removeBioNetworkAssignmentVariables(runtimeState.variables);
		const double bioNetworkStepSize = std::max(0.0, runtimeState.colonyTime - previousColonyTime);
		if (!_advanceBioNetworkStep(bioNetworkStepSize, result.errorMessage)) {
			result.succeeded = false;
			return result;
		}
		// The colony owns the persistent program state between executions, so the
		// plugin runtime hands back the updated scalar-variable map after each run.
		_colonyTime = _usesBioNetworkTime() ? _bioNetwork->getCurrentTime() : runtimeState.colonyTime;
		_runtimeVariables = runtimeState.variables;
		_runtimeVariables["dt"] = getSimulationStep();
		_applyRuntimePopulationMutations(result.populationMutations, runtimeState.populationSize);
		_applySignalFieldStep();
	}
	return result;
}

bool BacteriaColony::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string groProgramName = fields->loadField("groProgram", DEFAULT.groProgramName);
		if (!groProgramName.empty()) {
			ModelDataDefinition* datum = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<GroProgram>(),
			                                                                               groProgramName);
			_groProgram = dynamic_cast<GroProgram*>(datum);
		}
		const std::string bioNetworkName = fields->loadField("bioNetwork", DEFAULT.bioNetworkName);
		if (!bioNetworkName.empty()) {
			ModelDataDefinition* datum = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(),
			                                                                               bioNetworkName);
			_bioNetwork = dynamic_cast<BioNetwork*>(datum);
		}
		const std::string signalGridName = fields->loadField("signalGrid", DEFAULT.signalGridName);
		if (!signalGridName.empty()) {
			ModelDataDefinition* datum = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BacteriaSignalGrid>(),
			                                                                               signalGridName);
			_signalGrid = dynamic_cast<BacteriaSignalGrid*>(datum);
		}
		_simulationStep = fields->loadField("simulationStep", DEFAULT.simulationStep);
		_initialColonyTime = fields->loadField("initialColonyTime", DEFAULT.initialColonyTime);
		_finalColonyTime = fields->loadField("finalColonyTime", DEFAULT.finalColonyTime);
		_colonyTimeUnit = fields->loadField("colonyTimeUnit", DEFAULT.colonyTimeUnit);
		_colonyTime = _initialColonyTime;
		_initialPopulation = fields->loadField("initialPopulation", DEFAULT.initialPopulation);
		_gridWidth = fields->loadField("gridWidth", DEFAULT.gridWidth);
		_gridHeight = fields->loadField("gridHeight", DEFAULT.gridHeight);
		_synchronizeTemporalStateFromBioNetwork();
		_synchronizeGridDimensionsFromSignalGrid();
		std::string runtimeConfigErrorMessage;
		(void)_refreshRuntimeConfigurationFromGroProgram(runtimeConfigErrorMessage);
		if (_groSeedDefinitions.empty()) {
			_rebuildInternalBacteria(_initialPopulation);
		} else {
			_rebuildInternalBacteriaFromGroSeeds();
		}
		_runtimeVariables["dt"] = getSimulationStep();
		std::string signalErrorMessage;
		(void)_resetRuntimeSignalField(signalErrorMessage);
	}
	return res;
}

void BacteriaColony::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("groProgram", _groProgram != nullptr ? _groProgram->getName() : DEFAULT.groProgramName,
		                  DEFAULT.groProgramName, saveDefaultValues);
	fields->saveField("bioNetwork", _bioNetwork != nullptr ? _bioNetwork->getName() : DEFAULT.bioNetworkName,
	                  DEFAULT.bioNetworkName, saveDefaultValues);
	fields->saveField("signalGrid", _signalGrid != nullptr ? _signalGrid->getName() : DEFAULT.signalGridName,
	                  DEFAULT.signalGridName, saveDefaultValues);
	fields->saveField("simulationStep", _simulationStep, DEFAULT.simulationStep, saveDefaultValues);
	fields->saveField("initialColonyTime", _initialColonyTime, DEFAULT.initialColonyTime, saveDefaultValues);
	fields->saveField("finalColonyTime", _finalColonyTime, DEFAULT.finalColonyTime, saveDefaultValues);
	fields->saveField("colonyTimeUnit", _colonyTimeUnit, DEFAULT.colonyTimeUnit, saveDefaultValues);
	fields->saveField("initialPopulation", _initialPopulation, DEFAULT.initialPopulation, saveDefaultValues);
	fields->saveField("gridWidth", _gridWidth, DEFAULT.gridWidth, saveDefaultValues);
	fields->saveField("gridHeight", _gridHeight, DEFAULT.gridHeight, saveDefaultValues);
}

bool BacteriaColony::_check(std::string& errorMessage) {
	bool resultAll = true;

	if (getSimulationStep() <= 0.0) {
		errorMessage += "BacteriaColony simulation step must be greater than zero. ";
		resultAll = false;
	}
	if (_initialPopulation == 0) {
		errorMessage += "BacteriaColony initial population must be greater than zero. ";
		resultAll = false;
	}
	if (getGridWidth() == 0 || getGridHeight() == 0) {
		errorMessage += "BacteriaColony grid dimensions must be greater than zero. ";
		resultAll = false;
	}
	if (getFinalColonyTime() < getInitialColonyTime()) {
		errorMessage += "BacteriaColony final colony time must be greater than or equal to the initial colony time. ";
		resultAll = false;
	}

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<GroProgram>(), _groProgram, "GroProgram",
	                                                   errorMessage);
	if (_bioNetwork != nullptr) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<BioNetwork>(), _bioNetwork, "BioNetwork",
		                                                   errorMessage);
	}
	if (_signalGrid != nullptr) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<BacteriaSignalGrid>(), _signalGrid, "SignalGrid",
		                                                   errorMessage);
	}
	if (_groProgram != nullptr) {
		resultAll &= _groProgram->validateSyntax(errorMessage);
	}

	return resultAll;
}

void BacteriaColony::_initBetweenReplications() {
	_runtimeVariables.clear();
	if (_bioNetwork != nullptr) {
		// The colony reuses the referenced biochemical state machine and resets it
		// alongside the colony replication lifecycle.
		ModelDataDefinition::InitBetweenReplications(_bioNetwork);
	}
	_colonyTime = getInitialColonyTime();
	if (_usesBioNetworkTime()) {
		_colonyTime = _bioNetwork->getCurrentTime();
	}
	std::string runtimeConfigErrorMessage;
	(void)_refreshRuntimeConfigurationFromGroProgram(runtimeConfigErrorMessage);
	if (_groSeedDefinitions.empty()) {
		_rebuildInternalBacteria(_initialPopulation);
	} else {
		_rebuildInternalBacteriaFromGroSeeds();
	}
	_runtimeVariables["dt"] = getSimulationStep();
	std::string signalErrorMessage;
	(void)_resetRuntimeSignalField(signalErrorMessage);
}

void BacteriaColony::_createInternalAndAttachedData() {
	if (_groProgram != nullptr) {
		_attachedDataInsert("GroProgram", _groProgram);
	} else {
		_attachedDataRemove("GroProgram");
	}
	if (_bioNetwork != nullptr) {
		_attachedDataInsert("BioNetwork", _bioNetwork);
	} else {
		_attachedDataRemove("BioNetwork");
	}
	if (_signalGrid != nullptr) {
		_attachedDataInsert("SignalGrid", _signalGrid);
	} else {
		_attachedDataRemove("SignalGrid");
	}
}

void BacteriaColony::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_groProgram != nullptr) {
		GroProgramRuntime::ExecutionResult result = executeGroProgram();
		if (result.succeeded) {
			traceSimulation(this, "Bacteria colony Gro program executed " + std::to_string(result.executedCommands) +
								  " command(s)");
		} else {
			traceSimulation(this, "Bacteria colony Gro program execution failed: " + result.errorMessage);
		}
	} else {
		advanceColonyTime();
		traceSimulation(this, "Bacteria colony internal time advanced to " + std::to_string(getColonyTime()));
	}
	if (entity == nullptr) {
		return;
	}

	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "Bacteria colony dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}

	if (getColonyTime() >= getFinalColonyTime()) {
		_colonyTime = getInitialColonyTime();
		if (_usesBioNetworkTime()) {
			_bioNetwork->setCurrentTime(_colonyTime);
		}
		_parentModel->sendEntityToComponent(entity, frontConnection);
	} else {
		// The colony step is expressed in colony-local time units and must be
		// converted to the model replication base unit before re-scheduling.
		double timeScale = Util::TimeUnitConvert(_colonyTimeUnit, _parentModel->getSimulation()->getReplicationBaseTimeUnit());
		double stepDelay = getSimulationStep() * timeScale;
		_parentModel->sendEntityToComponent(entity, this, stepDelay);
	}
}

bool BacteriaColony::_resetRuntimeSignalField(std::string& errorMessage) {
	if (_signalGrid != nullptr) {
		return _signalGrid->buildInitialField(_signalField, errorMessage);
	}

	_signalField.assign(static_cast<std::size_t>(getGridWidth()) * static_cast<std::size_t>(getGridHeight()), 0.0);
	return true;
}

bool BacteriaColony::_collectBioNetworkSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const {
	species.clear();
	errorMessage.clear();
	if (_bioNetwork == nullptr) {
		return true;
	}

	const std::vector<std::string>& speciesNames = _bioNetwork->getSpeciesNames();
	if (speciesNames.empty()) {
		List<ModelDataDefinition*>* list = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());
		for (ModelDataDefinition* definition : *list->list()) {
			auto* bioSpecies = dynamic_cast<BioSpecies*>(definition);
			if (bioSpecies != nullptr) {
				species.push_back(bioSpecies);
			}
		}
	} else {
		for (const std::string& speciesName : speciesNames) {
			ModelDataDefinition* definition =
					_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName);
			auto* bioSpecies = dynamic_cast<BioSpecies*>(definition);
			if (bioSpecies == nullptr) {
				errorMessage += "BacteriaColony BioNetwork \"" + _bioNetwork->getName() +
				                "\" references missing BioSpecies \"" + speciesName + "\". ";
				continue;
			}
			species.push_back(bioSpecies);
		}
	}
	return errorMessage.empty();
}

bool BacteriaColony::_collectBioNetworkSpeciesByIdentifier(std::map<std::string, BioSpecies*>& speciesByIdentifier,
                                                           std::string& errorMessage) const {
	speciesByIdentifier.clear();
	errorMessage.clear();
	if (_bioNetwork == nullptr) {
		return true;
	}

	std::vector<BioSpecies*> species;
	if (!_collectBioNetworkSpecies(species, errorMessage)) {
		return false;
	}

	for (BioSpecies* bioSpecies : species) {
		const std::string identifier = toGroIdentifierSuffix(bioSpecies->getName());
		const auto found = speciesByIdentifier.find(identifier);
		if (found != speciesByIdentifier.end() && found->second != bioSpecies) {
			errorMessage += "BacteriaColony BioNetwork \"" + _bioNetwork->getName() +
			                "\" exposes ambiguous GRO identifier \"bio_species_" + identifier + "\". ";
			continue;
		}
		speciesByIdentifier[identifier] = bioSpecies;
	}
	return errorMessage.empty();
}

void BacteriaColony::_appendBioNetworkContextVariables(GroProgramRuntimeState& runtimeState,
                                                       std::string& errorMessage) const {
	errorMessage.clear();
	if (_bioNetwork == nullptr) {
		return;
	}

	std::vector<BioSpecies*> species;
	if (!_collectBioNetworkSpecies(species, errorMessage)) {
		return;
	}

	// The biochemical state is exposed as read-only GRO identifiers so colony
	// logic can already react to concentrations without writing into BioNetwork.
	runtimeState.contextVariables["bio_current_time"] = _bioNetwork->getCurrentTime();
	runtimeState.contextVariables["bio_step_size"] = _bioNetwork->getStepSize();
	runtimeState.contextVariables["bio_species_count"] = static_cast<double>(species.size());
	for (BioSpecies* bioSpecies : species) {
		runtimeState.contextVariables["bio_species_" + toGroIdentifierSuffix(bioSpecies->getName())] =
				bioSpecies->getAmount();
	}
}

bool BacteriaColony::_applyBioNetworkAssignments(const std::map<std::string, double>& assignedVariables,
                                                 std::string& errorMessage) {
	errorMessage.clear();
	if (_bioNetwork == nullptr) {
		return true;
	}

	std::map<std::string, BioSpecies*> speciesByIdentifier;
	if (!_collectBioNetworkSpeciesByIdentifier(speciesByIdentifier, errorMessage)) {
		return false;
	}

	for (const auto& entry : assignedVariables) {
		if (entry.first.rfind(kBioSpeciesAssignmentPrefix, 0) != 0) {
			continue;
		}

		const std::string identifier = entry.first.substr(std::char_traits<char>::length(kBioSpeciesAssignmentPrefix));
		const auto found = speciesByIdentifier.find(identifier);
		if (found == speciesByIdentifier.end()) {
			errorMessage = "BacteriaColony BioNetwork assignment target \"" + entry.first +
			              "\" does not match any referenced BioSpecies. ";
			return false;
		}

		// GRO assignments use the same non-negative amount semantics already used
		// by BioNetwork state updates, so negative writes are rejected early.
		if (entry.second < 0.0) {
			errorMessage = "BacteriaColony BioNetwork assignment target \"" + entry.first +
			              "\" must receive a non-negative amount. ";
			return false;
		}
		found->second->setAmount(entry.second);
	}
	return true;
}

void BacteriaColony::_removeBioNetworkAssignmentVariables(std::map<std::string, double>& variables) const {
	for (auto it = variables.begin(); it != variables.end();) {
		if (it->first.rfind(kBioSpeciesAssignmentPrefix, 0) == 0) {
			it = variables.erase(it);
			continue;
		}
		++it;
	}
}

bool BacteriaColony::_refreshRuntimeConfigurationFromGroProgram(std::string& errorMessage) {
	errorMessage.clear();
	_groSeedDefinitions.clear();
	if (_groProgram == nullptr) {
		return true;
	}

	const std::string& sourceCode = _groProgram->getSourceCode();
	double configuredDt = 0.0;
	if (parseGroConfiguredDt(sourceCode, configuredDt)) {
		setSimulationStep(configuredDt);
	}

	const std::vector<GroSeedPlacement> parsedPlacements = parseGroSeedPlacements(sourceCode);
	if (parsedPlacements.empty()) {
		return true;
	}

	unsigned int maxGridX = 0;
	unsigned int maxGridY = 0;
	for (const GroSeedPlacement& placement : parsedPlacements) {
		GroSeedDefinition definition;
		definition.gridX = placement.gridX;
		definition.gridY = placement.gridY;
		definition.programName = placement.programName;
		_groSeedDefinitions.push_back(definition);
		maxGridX = std::max(maxGridX, placement.gridX);
		maxGridY = std::max(maxGridY, placement.gridY);
	}

	if (!_usesSignalGridDimensions()) {
		// Classic Gro seed placements define a discrete colony footprint even when no SignalGrid exists yet.
		_gridWidth = std::max(_gridWidth, maxGridX + 1);
		_gridHeight = std::max(_gridHeight, maxGridY + 1);
		if (_signalField.size() != static_cast<std::size_t>(_gridWidth) * static_cast<std::size_t>(_gridHeight)) {
			_signalField.assign(static_cast<std::size_t>(_gridWidth) * static_cast<std::size_t>(_gridHeight), 0.0);
		}
	}
	return true;
}

void BacteriaColony::_rebuildInternalBacteriaFromGroSeeds() {
	_bacteria.clear();
	_nextBacteriumId = 1;
	for (const GroSeedDefinition& placement : _groSeedDefinitions) {
		_appendBacterium(0, 0, placement.programName);
		BacteriumState& bacterium = _bacteria.back();
		bacterium.gridX = placement.gridX;
		bacterium.gridY = placement.gridY;
		bacterium.hasExplicitGridPosition = true;
	}
	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
}

bool BacteriaColony::_advanceBioNetworkStep(double stepSize, std::string& errorMessage) {
	errorMessage.clear();
	if (_bioNetwork == nullptr || stepSize <= 0.0) {
		return true;
	}

	// Preserve the configured BioNetwork time horizon and only consume the
	// amount of biological time requested by the colony execution.
	const double configuredStepSize = _bioNetwork->getStepSize();
	const double targetTime = std::min(_bioNetwork->getCurrentTime() + stepSize, _bioNetwork->getStopTime());
	while (_bioNetwork->getCurrentTime() + 1e-12 < targetTime) {
		const double remaining = targetTime - _bioNetwork->getCurrentTime();
		const double integrationStep = std::min(configuredStepSize > 0.0 ? configuredStepSize : remaining, remaining);
		_bioNetwork->setStepSize(integrationStep);
		if (!_bioNetwork->advanceOneStep(errorMessage)) {
			_bioNetwork->setStepSize(configuredStepSize);
			return false;
		}
	}
	_bioNetwork->setStepSize(configuredStepSize);
	_colonyTime = _bioNetwork->getCurrentTime();
	return true;
}

std::size_t BacteriaColony::_signalIndex(unsigned int x, unsigned int y) const {
	return static_cast<std::size_t>(y) * static_cast<std::size_t>(getGridWidth()) + static_cast<std::size_t>(x);
}

double BacteriaColony::_signalValueAt(unsigned int x, unsigned int y) const {
	if (getGridWidth() == 0 || getGridHeight() == 0 || x >= getGridWidth() || y >= getGridHeight()) {
		return 0.0;
	}
	const std::size_t index = _signalIndex(x, y);
	if (index >= _signalField.size()) {
		return 0.0;
	}
	return _signalField[index];
}

void BacteriaColony::_setSignalValueAt(unsigned int x, unsigned int y, double value) {
	if (x >= getGridWidth() || y >= getGridHeight()) {
		return;
	}
	const std::size_t index = _signalIndex(x, y);
	if (index >= _signalField.size()) {
		return;
	}
	_signalField[index] = value;
}

void BacteriaColony::_addSignalAt(unsigned int x, unsigned int y, double value) {
	if (x >= getGridWidth() || y >= getGridHeight()) {
		return;
	}
	const std::size_t index = _signalIndex(x, y);
	if (index >= _signalField.size()) {
		return;
	}
	_signalField[index] += value;
}

double BacteriaColony::_computeNeighborSignalSum(unsigned int x, unsigned int y) const {
	double sum = 0.0;
	if (x > 0) {
		sum += _signalValueAt(x - 1, y);
	}
	if (x + 1 < getGridWidth()) {
		sum += _signalValueAt(x + 1, y);
	}
	if (y > 0) {
		sum += _signalValueAt(x, y - 1);
	}
	if (y + 1 < getGridHeight()) {
		sum += _signalValueAt(x, y + 1);
	}
	return sum;
}

unsigned int BacteriaColony::_computeLocalBacteriaCount(unsigned int x, unsigned int y) const {
	unsigned int count = 0;
	for (const BacteriumState& bacterium : _bacteria) {
		if (bacterium.alive && bacterium.gridX == x && bacterium.gridY == y) {
			++count;
		}
	}
	return count;
}

void BacteriaColony::_applySignalFieldStep() {
	if (_signalGrid == nullptr || _signalField.empty()) {
		return;
	}

	const double diffusionRate = _signalGrid->getDiffusionRate();
	const double decayFactor = 1.0 - _signalGrid->getDecayRate();
	if (diffusionRate == 0.0 && decayFactor == 1.0) {
		return;
	}

	std::vector<double> updatedField = _signalField;
	for (unsigned int y = 0; y < getGridHeight(); ++y) {
		for (unsigned int x = 0; x < getGridWidth(); ++x) {
			const double currentValue = _signalValueAt(x, y);
			double neighborSum = 0.0;
			unsigned int neighborCount = 0;
			if (x > 0) {
				neighborSum += _signalValueAt(x - 1, y);
				++neighborCount;
			}
			if (x + 1 < getGridWidth()) {
				neighborSum += _signalValueAt(x + 1, y);
				++neighborCount;
			}
			if (y > 0) {
				neighborSum += _signalValueAt(x, y - 1);
				++neighborCount;
			}
			if (y + 1 < getGridHeight()) {
				neighborSum += _signalValueAt(x, y + 1);
				++neighborCount;
			}

			// The field uses a simple von-Neumann relaxation so this first slice
			// already exposes local diffusion/decay without a full PDE subsystem.
			double relaxedValue = currentValue;
			if (neighborCount > 0) {
				const double neighborAverage = neighborSum / static_cast<double>(neighborCount);
				relaxedValue += diffusionRate * (neighborAverage - currentValue);
			}
			updatedField[_signalIndex(x, y)] = std::max(0.0, relaxedValue * decayFactor);
		}
	}
	_signalField = std::move(updatedField);
}

void BacteriaColony::_applyBacteriumSignalMutations(const BacteriumState& bacterium,
                                                    const std::vector<GroProgramRuntime::SignalMutation>& mutations) {
	for (const GroProgramRuntime::SignalMutation& mutation : mutations) {
		if (mutation.type == GroProgramRuntime::SignalMutationType::Emit) {
			_addSignalAt(bacterium.gridX, bacterium.gridY, mutation.value);
			continue;
		}
		if (mutation.type == GroProgramRuntime::SignalMutationType::Consume) {
			const double currentValue = _signalValueAt(bacterium.gridX, bacterium.gridY);
			_setSignalValueAt(bacterium.gridX, bacterium.gridY, std::max(0.0, currentValue - mutation.value));
			continue;
		}
		if (mutation.type == GroProgramRuntime::SignalMutationType::Set) {
			_setSignalValueAt(bacterium.gridX, bacterium.gridY, mutation.value);
		}
	}
}

void BacteriaColony::_rebuildInternalBacteria(unsigned int populationSize) {
	_bacteria.clear();
	_nextBacteriumId = 1;
	_resizeInternalBacteria(populationSize);
}

void BacteriaColony::_resizeInternalBacteria(unsigned int populationSize) {
	while (_bacteria.size() > populationSize) {
		_bacteria.pop_back();
	}

	while (_bacteria.size() < populationSize) {
		_appendBacterium();
	}

	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
	_rebuildBacteriaGridPositions();
}

void BacteriaColony::_applyRuntimePopulationMutations(const std::vector<GroProgramRuntime::PopulationMutation>& mutations,
                                                       unsigned int finalPopulationSize) {
	for (const GroProgramRuntime::PopulationMutation& mutation : mutations) {
		if (mutation.type == GroProgramRuntime::PopulationMutationType::Grow) {
			for (unsigned int i = 0; i < mutation.value; ++i) {
				_appendBacterium();
			}
			continue;
		}

		if (mutation.type == GroProgramRuntime::PopulationMutationType::Divide) {
			const std::size_t parentCount = _bacteria.size();
			for (std::size_t index = 0; index < parentCount; ++index) {
				const unsigned int parentId = _bacteria[index].id;
				const unsigned int childGeneration = _bacteria[index].generation + 1;
				++_bacteria[index].divisionCount;
				_bacteria[index].lastDivisionTime = getColonyTime();
				_appendBacterium(parentId, childGeneration);
			}
			continue;
		}

		if (mutation.type == GroProgramRuntime::PopulationMutationType::Die) {
			_removeBacteria(mutation.value);
			continue;
		}

		if (mutation.type == GroProgramRuntime::PopulationMutationType::SetPopulation) {
			_resizeInternalBacteria(mutation.resultingPopulationSize);
		}
	}

	if (_bacteria.size() != finalPopulationSize) {
		_resizeInternalBacteria(finalPopulationSize);
	}

	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
	_rebuildBacteriaGridPositions();
}

bool BacteriaColony::_executeSeededNamedGroPrograms(const GroProgramIr& ir,
                                                    GroProgramRuntime::ExecutionResult& result) {
	std::string unsupportedCommand;
	for (const auto& programEntry : ir.namedPrograms) {
		if (_containsBacteriumScopedOnlyUnsupportedCommand(programEntry.second, unsupportedCommand)) {
			result.succeeded = false;
			result.errorMessage =
					"BacteriaColony seeded GRO programs do not support command \"" + unsupportedCommand +
					"\". Use assignments, rules, grow, die, and signal commands in this mode. ";
			return false;
		}
	}

	GroProgramRuntime runtime;
	if (!ir.commands.empty()) {
		GroProgramIr preludeIr;
		preludeIr.commands = ir.commands;

		GroProgramRuntimeState preludeState;
		preludeState.colonyTime = getColonyTime();
		preludeState.simulationStep = getSimulationStep();
		preludeState.populationSize = _populationSize;
		preludeState.variables = _runtimeVariables;
		preludeState.variables["dt"] = preludeState.simulationStep;
		_appendBioNetworkContextVariables(preludeState, result.errorMessage);
		if (!result.errorMessage.empty()) {
			result.succeeded = false;
			return false;
		}

		GroProgramRuntime::ExecutionResult preludeResult = runtime.execute(preludeIr, preludeState);
		if (!preludeResult.succeeded) {
			result = preludeResult;
			result.errorMessage = "BacteriaColony seeded-program prelude failed: " + result.errorMessage;
			return false;
		}
		if (!preludeResult.signalMutations.empty() || !preludeResult.populationMutations.empty()) {
			result.succeeded = false;
			result.errorMessage =
					"BacteriaColony seeded-program prelude currently supports only scalar assignments and set(\"dt\", ...). ";
			return false;
		}
		if (preludeState.simulationStep > 0.0 &&
		    std::fabs(preludeState.simulationStep - getSimulationStep()) > 1e-12) {
			setSimulationStep(preludeState.simulationStep);
		}
		if (!_applyBioNetworkAssignments(preludeResult.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			return false;
		}
		_removeBioNetworkAssignmentVariables(preludeState.variables);
		_runtimeVariables = preludeState.variables;
		_runtimeVariables["dt"] = getSimulationStep();
		result.executedCommands += preludeResult.executedCommands;
		result.assignedVariables.insert(preludeResult.assignedVariables.begin(), preludeResult.assignedVariables.end());
		result.unsupportedCommands.insert(result.unsupportedCommands.end(),
		                                  preludeResult.unsupportedCommands.begin(),
		                                  preludeResult.unsupportedCommands.end());
		result.skippedRawStatements.insert(result.skippedRawStatements.end(),
		                                   preludeResult.skippedRawStatements.begin(),
		                                   preludeResult.skippedRawStatements.end());
	}

	advanceColonyTime();

	std::vector<unsigned int> bacteriumIds;
	bacteriumIds.reserve(_bacteria.size());
	for (const BacteriumState& bacterium : _bacteria) {
		if (bacterium.alive) {
			bacteriumIds.push_back(bacterium.id);
		}
	}

	for (unsigned int bacteriumId : bacteriumIds) {
		const std::size_t bacteriumIndex = _findBacteriumIndexById(bacteriumId);
		if (bacteriumIndex >= _bacteria.size()) {
			continue;
		}

		BacteriumState& bacterium = _bacteria[bacteriumIndex];
		const auto programEntry = ir.namedPrograms.find(bacterium.programName);
		if (programEntry == ir.namedPrograms.end()) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony could not find GRO program \"" + bacterium.programName +
			                      "\" for bacterium id " + std::to_string(bacteriumId) + ". ";
			return false;
		}

		GroProgramIr bacteriumIr;
		bacteriumIr.sourceForm = GroProgramAst::SourceForm::ProgramBlock;
		bacteriumIr.programName = bacterium.programName;
		bacteriumIr.commands = programEntry->second;

		GroProgramRuntimeState runtimeState = _createBacteriumRuntimeState(bacterium, bacteriumIndex);
		std::string bioContextErrorMessage;
		_appendBioNetworkContextVariables(runtimeState, bioContextErrorMessage);
		if (!bioContextErrorMessage.empty()) {
			result.succeeded = false;
			result.errorMessage = bioContextErrorMessage;
			return false;
		}

		GroProgramRuntime::ExecutionResult bacteriumResult = runtime.execute(bacteriumIr, runtimeState);
		if (!bacteriumResult.succeeded) {
			result = bacteriumResult;
			result.errorMessage = "BacteriaColony seeded-program execution failed for bacterium id " +
			                      std::to_string(bacteriumId) + ": " + result.errorMessage;
			return false;
		}
		if (runtimeState.simulationStep > 0.0 &&
		    std::fabs(runtimeState.simulationStep - getSimulationStep()) > 1e-12) {
			setSimulationStep(runtimeState.simulationStep);
			_runtimeVariables["dt"] = getSimulationStep();
		}

		if (!_applyBioNetworkAssignments(bacteriumResult.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony seeded-program biochemical assignment failed for bacterium id " +
			                      std::to_string(bacteriumId) + ": " + result.errorMessage;
			return false;
		}
		_removeBioNetworkAssignmentVariables(runtimeState.variables);
		bacterium.runtimeVariables = runtimeState.variables;
		result.executedCommands += bacteriumResult.executedCommands;

		for (const auto& entry : bacteriumResult.assignedVariables) {
			result.assignedVariables["bacterium[" + std::to_string(bacteriumId) + "]." + entry.first] = entry.second;
		}
		for (const std::string& command : bacteriumResult.unsupportedCommands) {
			result.unsupportedCommands.push_back("bacterium[" + std::to_string(bacteriumId) + "]:" + command);
		}
		for (const std::string& statement : bacteriumResult.skippedRawStatements) {
			result.skippedRawStatements.push_back("bacterium[" + std::to_string(bacteriumId) + "]:" + statement);
		}
		for (const auto& mutation : bacteriumResult.signalMutations) {
			result.signalMutations.push_back(mutation);
		}

		_applyBacteriumSignalMutations(bacterium, bacteriumResult.signalMutations);
		_applyBacteriumScopedPopulationMutations(bacteriumId, bacterium.generation,
		                                         bacteriumResult.populationMutations, result);
		if (!result.succeeded) {
			return false;
		}

		_populationSize = static_cast<unsigned int>(_bacteria.size());
		_rebuildBacteriaGridPositions();
	}

	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
	if (!_advanceBioNetworkStep(getSimulationStep(), result.errorMessage)) {
		result.succeeded = false;
		return false;
	}
	_applySignalFieldStep();
	_rebuildBacteriaGridPositions();
	return true;
}

bool BacteriaColony::_executeBacteriumScopedGroProgram(const GroProgramIr& ir,
                                                       GroProgramRuntime::ExecutionResult& result) {
	std::string unsupportedCommand;
	if (_containsBacteriumScopedOnlyUnsupportedCommand(ir.commands, unsupportedCommand)) {
		result.succeeded = false;
		result.errorMessage =
				"BacteriaColony bacterium-scoped programs do not support command \"" + unsupportedCommand +
				"\". Use only assignments, conditions, grow, die, and unsupported raw statements in this mode. ";
		return false;
	}

	// In bacterium scope one colony execution still represents a single biological
	// step, so the colony clock advances once before the per-bacterium decisions.
	advanceColonyTime();

	std::vector<unsigned int> bacteriumIds;
	bacteriumIds.reserve(_bacteria.size());
	for (const BacteriumState& bacterium : _bacteria) {
		if (bacterium.alive) {
			bacteriumIds.push_back(bacterium.id);
		}
	}

	GroProgramRuntime runtime;
	for (unsigned int bacteriumId : bacteriumIds) {
		const std::size_t bacteriumIndex = _findBacteriumIndexById(bacteriumId);
		if (bacteriumIndex >= _bacteria.size()) {
			continue;
		}

		BacteriumState& bacterium = _bacteria[bacteriumIndex];
		GroProgramRuntimeState runtimeState = _createBacteriumRuntimeState(bacterium, bacteriumIndex);
		std::string bioContextErrorMessage;
		_appendBioNetworkContextVariables(runtimeState, bioContextErrorMessage);
		if (!bioContextErrorMessage.empty()) {
			result.succeeded = false;
			result.errorMessage = bioContextErrorMessage;
			return false;
		}
		GroProgramRuntime::ExecutionResult bacteriumResult = runtime.execute(ir, runtimeState);
		if (!bacteriumResult.succeeded) {
			result = bacteriumResult;
			result.errorMessage = "BacteriaColony bacterium-scoped execution failed for bacterium id " +
			                      std::to_string(bacteriumId) + ": " + result.errorMessage;
			return false;
		}
		if (runtimeState.simulationStep > 0.0 &&
		    std::fabs(runtimeState.simulationStep - getSimulationStep()) > 1e-12) {
			setSimulationStep(runtimeState.simulationStep);
		}

		if (!_applyBioNetworkAssignments(bacteriumResult.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony bacterium-scoped biochemical assignment failed for bacterium id " +
			                      std::to_string(bacteriumId) + ": " + result.errorMessage;
			return false;
		}
		_removeBioNetworkAssignmentVariables(runtimeState.variables);
		bacterium.runtimeVariables = runtimeState.variables;
		result.executedCommands += bacteriumResult.executedCommands;

		for (const auto& entry : bacteriumResult.assignedVariables) {
			result.assignedVariables["bacterium[" + std::to_string(bacteriumId) + "]." + entry.first] = entry.second;
		}
		for (const std::string& command : bacteriumResult.unsupportedCommands) {
			result.unsupportedCommands.push_back("bacterium[" + std::to_string(bacteriumId) + "]:" + command);
		}
		for (const std::string& statement : bacteriumResult.skippedRawStatements) {
			result.skippedRawStatements.push_back("bacterium[" + std::to_string(bacteriumId) + "]:" + statement);
		}
		for (const auto& mutation : bacteriumResult.signalMutations) {
			result.signalMutations.push_back(mutation);
		}

		_applyBacteriumSignalMutations(bacterium, bacteriumResult.signalMutations);
		_applyBacteriumScopedPopulationMutations(bacteriumId, bacterium.generation,
		                                         bacteriumResult.populationMutations, result);
		if (!result.succeeded) {
			return false;
		}

		_populationSize = static_cast<unsigned int>(_bacteria.size());
		_rebuildBacteriaGridPositions();
	}

	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
	if (!_advanceBioNetworkStep(getSimulationStep(), result.errorMessage)) {
		result.succeeded = false;
		return false;
	}
	_applySignalFieldStep();
	_rebuildBacteriaGridPositions();
	return true;
}

bool BacteriaColony::_containsBacteriumScopedOnlyUnsupportedCommand(const std::vector<GroProgramIr::Command>& commands,
                                                                    std::string& unsupportedCommand) const {
	for (const GroProgramIr::Command& command : commands) {
		if (command.isIfStatement()) {
			if (_containsBacteriumScopedOnlyUnsupportedCommand(command.thenCommands, unsupportedCommand)) {
				return true;
			}
			if (_containsBacteriumScopedOnlyUnsupportedCommand(command.elseCommands, unsupportedCommand)) {
				return true;
			}
			continue;
		}

		if (!command.isFunctionCall()) {
			continue;
		}

		if (command.functionName == "tick" ||
		    command.functionName == "divide" ||
		    command.functionName == "set_population") {
			unsupportedCommand = command.sourceText;
			return true;
		}
	}
	return false;
}

std::size_t BacteriaColony::_findBacteriumIndexById(unsigned int bacteriumId) const {
	for (std::size_t index = 0; index < _bacteria.size(); ++index) {
		if (_bacteria[index].id == bacteriumId) {
			return index;
		}
	}
	return _bacteria.size();
}

GroProgramRuntimeState BacteriaColony::_createBacteriumRuntimeState(const BacteriumState& bacterium,
                                                                    std::size_t bacteriumIndex) const {
	GroProgramRuntimeState runtimeState;
	runtimeState.colonyTime = getColonyTime();
	runtimeState.simulationStep = getSimulationStep();
	runtimeState.populationSize = _populationSize;
	runtimeState.contextVariables["bacterium_id"] = static_cast<double>(bacterium.id);
	runtimeState.contextVariables["bacterium_parent_id"] = static_cast<double>(bacterium.parentId);
	runtimeState.contextVariables["bacterium_generation"] = static_cast<double>(bacterium.generation);
	runtimeState.contextVariables["bacterium_divisions"] = static_cast<double>(bacterium.divisionCount);
	runtimeState.contextVariables["bacterium_birth_time"] = bacterium.birthTime;
	runtimeState.contextVariables["bacterium_age"] = getBacteriumAge(bacteriumIndex);
	runtimeState.contextVariables["bacterium_grid_x"] = static_cast<double>(bacterium.gridX);
	runtimeState.contextVariables["bacterium_grid_y"] = static_cast<double>(bacterium.gridY);
	runtimeState.contextVariables["bacterium_index"] = static_cast<double>(bacteriumIndex);
	runtimeState.contextVariables["local_signal"] = _signalValueAt(bacterium.gridX, bacterium.gridY);
	runtimeState.contextVariables["neighbor_signal_sum"] = _computeNeighborSignalSum(bacterium.gridX, bacterium.gridY);
	runtimeState.contextVariables["local_bacteria_count"] = static_cast<double>(
			_computeLocalBacteriaCount(bacterium.gridX, bacterium.gridY));
	// Colony-level scalar declarations such as signal handles are visible to each
	// bacterium, but per-bacterium state still overrides the shared defaults.
	runtimeState.variables = _runtimeVariables;
	for (const auto& entry : bacterium.runtimeVariables) {
		runtimeState.variables[entry.first] = entry.second;
	}
	runtimeState.variables["dt"] = runtimeState.simulationStep;
	return runtimeState;
}

void BacteriaColony::_applyBacteriumScopedPopulationMutations(
		unsigned int bacteriumId,
		unsigned int parentGeneration,
		const std::vector<GroProgramRuntime::PopulationMutation>& mutations,
		GroProgramRuntime::ExecutionResult& result) {
	for (const GroProgramRuntime::PopulationMutation& mutation : mutations) {
		if (mutation.type == GroProgramRuntime::PopulationMutationType::Grow) {
			const std::size_t parentIndex = _findBacteriumIndexById(bacteriumId);
			if (parentIndex >= _bacteria.size()) {
				result.succeeded = false;
				result.errorMessage = "BacteriaColony could not find grow parent bacterium id " +
				                      std::to_string(bacteriumId) + ". ";
				return;
			}

			_bacteria[parentIndex].divisionCount += mutation.value;
			_bacteria[parentIndex].lastDivisionTime = getColonyTime();
			const std::string childProgramName = _bacteria[parentIndex].programName;
			for (unsigned int i = 0; i < mutation.value; ++i) {
				_appendBacterium(bacteriumId, parentGeneration + 1, childProgramName);
			}
			result.populationMutations.push_back(mutation);
			continue;
		}

		if (mutation.type == GroProgramRuntime::PopulationMutationType::Die) {
			if (mutation.value != 1) {
				result.succeeded = false;
				result.errorMessage =
						"BacteriaColony bacterium-scoped die command must remove exactly one bacterium. ";
				return;
			}
			if (!_removeBacteriumById(bacteriumId)) {
				result.succeeded = false;
				result.errorMessage = "BacteriaColony could not remove bacterium id " +
				                      std::to_string(bacteriumId) + ". ";
				return;
			}
			result.populationMutations.push_back(mutation);
			continue;
		}

		result.succeeded = false;
		result.errorMessage =
				"BacteriaColony bacterium-scoped programs only support grow and die population mutations. ";
		return;
	}
}

void BacteriaColony::_appendBacterium(unsigned int parentId, unsigned int generation, const std::string& programName) {
	BacteriumState bacterium;
	bacterium.id = _nextBacteriumId++;
	bacterium.parentId = parentId;
	bacterium.generation = generation;
	bacterium.programName = programName;
	bacterium.divisionCount = 0;
	bacterium.birthTime = getColonyTime();
	bacterium.lastUpdateTime = getColonyTime();
	bacterium.lastDivisionTime = 0.0;
	bacterium.hasExplicitGridPosition = false;
	bacterium.alive = true;
	_assignBacteriumGridPosition(bacterium, _bacteria.size());
	_bacteria.push_back(bacterium);
}

void BacteriaColony::_removeBacteria(unsigned int amount) {
	for (unsigned int i = 0; i < amount && !_bacteria.empty(); ++i) {
		_bacteria.back().alive = false;
		_bacteria.pop_back();
	}
}

bool BacteriaColony::_removeBacteriumById(unsigned int bacteriumId) {
	const std::size_t index = _findBacteriumIndexById(bacteriumId);
	if (index >= _bacteria.size()) {
		return false;
	}

	_bacteria[index].alive = false;
	_bacteria.erase(_bacteria.begin() + static_cast<std::ptrdiff_t>(index));
	return true;
}

void BacteriaColony::_refreshBacteriaUpdateTime() {
	for (BacteriumState& bacterium : _bacteria) {
		if (bacterium.alive) {
			bacterium.lastUpdateTime = getColonyTime();
		}
	}
}

void BacteriaColony::_rebuildBacteriaGridPositions() {
	for (std::size_t index = 0; index < _bacteria.size(); ++index) {
		_assignBacteriumGridPosition(_bacteria[index], index);
	}
}

void BacteriaColony::_assignBacteriumGridPosition(BacteriumState& bacterium, std::size_t index) const {
	if (bacterium.hasExplicitGridPosition) {
		return;
	}
	if (getGridWidth() == 0 || getGridHeight() == 0) {
		bacterium.gridX = 0;
		bacterium.gridY = 0;
		return;
	}

	bacterium.gridX = static_cast<unsigned int>(index % getGridWidth());
	bacterium.gridY = static_cast<unsigned int>((index / getGridWidth()) % getGridHeight());
}

bool BacteriaColony::_usesBioNetworkTime() const {
	return _bioNetwork != nullptr;
}

bool BacteriaColony::_usesSignalGridDimensions() const {
	return _signalGrid != nullptr;
}

void BacteriaColony::_synchronizeTemporalStateFromBioNetwork() {
	if (_bioNetwork == nullptr) {
		return;
	}

	// Mirror the authoritative BioNetwork timing into the legacy colony fields
	// so fallback serialization and diagnostics stay coherent.
	_simulationStep = _bioNetwork->getStepSize();
	_initialColonyTime = _bioNetwork->getStartTime();
	_finalColonyTime = _bioNetwork->getStopTime();
	_colonyTime = _bioNetwork->getCurrentTime();
}

void BacteriaColony::_synchronizeGridDimensionsFromSignalGrid() {
	if (_signalGrid == nullptr) {
		return;
	}

	// Mirror the authoritative SignalGrid dimensions into the legacy colony
	// fields so fallback serialization and diagnostics stay coherent.
	_gridWidth = _signalGrid->getWidth();
	_gridHeight = _signalGrid->getHeight();
}

void BacteriaColony::_createReportStatisticsDataDefinitions() {
}

void BacteriaColony::_createEditableDataDefinitions() {
}

void BacteriaColony::_createOthersDataDefinitions() {
}
