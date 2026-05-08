/*
 * File:   BacteriaColony.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "../../../kernel/simulator/model/ModelSimulation.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/GroProgramCompiler.h"
#include "plugins/data/BiochemicalSimulation/GroProgramParser.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BacteriaColony::GetPluginInformation;
}
#endif

namespace {

constexpr const char* kBioSpeciesAssignmentPrefix = "bio_species_";

struct GroSeedPlacement {
	double positionX = 0.0;
	double positionY = 0.0;
	std::string programName = "";
	std::vector<double> programArguments;
};

std::string trim(const std::string& text) {
	std::size_t first = 0;
	while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
		++first;
	}

	std::size_t last = text.size();
	while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
		--last;
	}
	return text.substr(first, last - first);
}

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

std::vector<std::string> splitTopLevelArguments(const std::string& text) {
	std::vector<std::string> arguments;
	std::vector<char> delimiterStack;
	bool inString = false;
	char stringDelimiter = '\0';
	std::size_t argumentStart = 0;

	for (std::size_t i = 0; i < text.size(); ++i) {
		const char current = text[i];
		if (inString) {
			if (current == '\\') {
				++i;
				continue;
			}
			if (current == stringDelimiter) {
				inString = false;
				stringDelimiter = '\0';
			}
			continue;
		}

		if (current == '"' || current == '\'') {
			inString = true;
			stringDelimiter = current;
			continue;
		}

		if (current == '(' || current == '[' || current == '{') {
			delimiterStack.push_back(current);
			continue;
		}
		if (current == ')' || current == ']' || current == '}') {
			if (!delimiterStack.empty()) {
				delimiterStack.pop_back();
			}
			continue;
		}

		if (current == ',' && delimiterStack.empty()) {
			const std::string argument = text.substr(argumentStart, i - argumentStart);
			if (!argument.empty()) {
				arguments.push_back(argument);
			}
			argumentStart = i + 1;
		}
	}

	const std::string trailingArgument = text.substr(argumentStart);
	if (!trailingArgument.empty()) {
		arguments.push_back(trailingArgument);
	}
	return arguments;
}

std::vector<double> parseGroProgramArguments(const std::string& argumentsText) {
	std::vector<double> arguments;
	for (const std::string& argumentText : splitTopLevelArguments(argumentsText)) {
		const std::string trimmedArgument = trim(argumentText);
		if (trimmedArgument.empty()) {
			continue;
		}
		try {
			std::size_t parsedCharacters = 0;
			const double value = std::stod(trimmedArgument, &parsedCharacters);
			if (parsedCharacters == trimmedArgument.size() && std::isfinite(value)) {
				arguments.push_back(value);
			}
		} catch (const std::exception&) {
		}
	}
	return arguments;
}

unsigned int toCenteredGridIndex(double value, unsigned int size) {
	if (size == 0) {
		return 0;
	}

	// Gro coordinates are centered on the origin; convert them into grid space.
	const double centeredValue = value + 0.5 * static_cast<double>(size) - 0.5;
	const long long rawIndex = std::llround(centeredValue);
	const long long maxIndex = static_cast<long long>(size - 1);
	return static_cast<unsigned int>(std::clamp(rawIndex, 0ll, maxIndex));
}

std::vector<GroSeedPlacement> parseGroSeedPlacements(const std::string& sourceCode) {
	const std::regex ecoliPattern(
			R"(ecoli\s*\(\s*\[([^\]]*)\]\s*,\s*program\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(([^)]*)\)\s*\))");
	const std::regex xPattern(R"(\bx\s*:?=\s*([-+]?[0-9]*\.?[0-9]+))");
	const std::regex yPattern(R"(\by\s*:?=\s*([-+]?[0-9]*\.?[0-9]+))");
	std::vector<GroSeedPlacement> placements;
	for (std::sregex_iterator it(sourceCode.begin(), sourceCode.end(), ecoliPattern), end; it != end; ++it) {
		try {
			double xValue = 0.0;
			double yValue = 0.0;
			const std::string seedRecord = (*it)[1].str();
			std::smatch xMatch;
			if (std::regex_search(seedRecord, xMatch, xPattern) && xMatch.size() >= 2) {
				xValue = std::stod(xMatch[1].str());
			}
			std::smatch yMatch;
			if (std::regex_search(seedRecord, yMatch, yPattern) && yMatch.size() >= 2) {
				yValue = std::stod(yMatch[1].str());
			}
			if (!std::isfinite(xValue) || !std::isfinite(yValue)) {
				continue;
			}
			GroSeedPlacement placement;
			placement.positionX = xValue;
			placement.positionY = yValue;
			placement.programName = (*it)[2].str();
			placement.programArguments = parseGroProgramArguments((*it)[3].str());
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
	SimulationControlGeneric<unsigned int>* propNumSteps = new SimulationControlGeneric<unsigned int>(
			std::bind(&BacteriaColony::getNumSteps, this),
			std::bind(&BacteriaColony::setNumSteps, this, std::placeholders::_1),
			Util::TypeOf<BacteriaColony>(), getName(), "NumSteps",
			"Number of colony steps processed before forwarding the current entity");
	SimulationControlGenericEnum<Util::TimeUnit, Util>* propColonyTimeUnit =
			new SimulationControlGenericEnum<Util::TimeUnit, Util>(
					std::bind(&BacteriaColony::getColonyTimeUnit, this),
					std::bind(&BacteriaColony::setColonyTimeUnit, this, std::placeholders::_1),
					Util::TypeOf<BacteriaColony>(), getName(), "ColonyTimeUnit",
					"Time unit used by SimulationStep when scheduling colony events");
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
	_parentModel->getControls()->insert(propNumSteps);
	_parentModel->getControls()->insert(propColonyTimeUnit);
	_parentModel->getControls()->insert(propInitialPopulation);
	_parentModel->getControls()->insert(propGridWidth);
	_parentModel->getControls()->insert(propGridHeight);

	_addSimulationControl(propGroProgram);
	_addSimulationControl(propBioNetwork);
	_addSimulationControl(propSignalGrid);
	_addSimulationControl(propSimulationStep);
	_addSimulationControl(propNumSteps);
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
	       ",numSteps=" + std::to_string(_numSteps) +
	       ",currentStep=" + std::to_string(_currentStep) +
	       ",colonyTimeUnit=" + Util::convertEnumToStr(_colonyTimeUnit) +
	       ",colonyTime=" + Util::StrTruncIfInt(std::to_string(getColonyTime())) +
	       ",initialPopulation=" + std::to_string(_initialPopulation) +
	       ",populationSize=" + std::to_string(_populationSize) +
	       ",chemostatMode=" + std::string(_chemostatMode ? "true" : "false") +
	       ",barriers=" + std::to_string(_barriers.size()) +
	       ",gridWidth=" + std::to_string(getGridWidth()) +
	       ",gridHeight=" + std::to_string(getGridHeight());
}

PluginInformation* BacteriaColony::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BacteriaColony>(),
	                                                &BacteriaColony::LoadInstance,
	                                                &BacteriaColony::NewInstance);
	info->setCategory("Biologic/Biochemical");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("groprogram.so");
	info->insertDynamicLibFileDependence("bionetwork.so");
	info->insertDynamicLibFileDependence("bacteriasignalgrid.so");
	info->setDescriptionHelp("Self-contained biological simulation component for Gro-inspired bacteria colonies. "
	                         "When an entity arrives, the colony advances one configured Gro/runtime step and then "
	                         "reschedules the entity until NumSteps is reached. This first slice uses ModelSimulation time "
	                         "as the colony clock and owns population, optional BioNetwork, discrete grid configuration, and optional signal field while leaving complete "
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
	_groSeedsApplied = false;
	std::string runtimeConfigErrorMessage;
	(void)_refreshRuntimeConfigurationFromGroProgram(runtimeConfigErrorMessage);
	_ensureGroSeededBacteriaInitialized();
}

GroProgram* BacteriaColony::getGroProgram() const {
	return _groProgram;
}

void BacteriaColony::setSimulationStep(double simulationStep) {
	_simulationStep = simulationStep;
}

double BacteriaColony::getSimulationStep() const {
	return _simulationStep;
}

void BacteriaColony::setNumSteps(unsigned int numSteps) {
	_numSteps = std::max(1u, numSteps);
}

unsigned int BacteriaColony::getNumSteps() const {
	return _numSteps;
}

void BacteriaColony::setColonyTimeUnit(Util::TimeUnit colonyTimeUnit) {
	_colonyTimeUnit = colonyTimeUnit;
}

Util::TimeUnit BacteriaColony::getColonyTimeUnit() const {
	return _colonyTimeUnit;
}

double BacteriaColony::getColonyTime() const {
	if (_parentModel == nullptr || _parentModel->getSimulation() == nullptr) {
		return 0.0;
	}
	return _parentModel->getSimulation()->getSimulatedTime();
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

double BacteriaColony::getBacteriumPositionX(std::size_t index) const {
	return getBacteriumState(index).positionX;
}

double BacteriaColony::getBacteriumPositionY(std::size_t index) const {
	return getBacteriumState(index).positionY;
}

double BacteriaColony::getBacteriumDirectionRadians(std::size_t index) const {
	return getBacteriumState(index).directionRadians;
}

double BacteriaColony::getBacteriumVolume(std::size_t index) const {
	return getBacteriumState(index).volume;
}

double BacteriaColony::getBacteriumSize(std::size_t index) const {
	return getBacteriumState(index).size;
}

double BacteriaColony::getBacteriumGfp(std::size_t index) const {
	return getBacteriumState(index).gfp;
}

double BacteriaColony::getBacteriumRfp(std::size_t index) const {
	return getBacteriumState(index).rfp;
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

std::vector<std::vector<double>> BacteriaColony::getSignalMatrix() const {
	return _buildSignalMatrix();
}

std::string BacteriaColony::getSignalMatrixDump(std::size_t maxRows, std::size_t maxColumns) const {
	return _buildSignalFieldDump(maxRows, maxColumns);
}

double BacteriaColony::getBacteriumLocalSignal(std::size_t index) const {
	const BacteriumState& bacterium = getBacteriumState(index);
	return _signalValueAt(bacterium.gridX, bacterium.gridY);
}

void BacteriaColony::setChemostatMode(bool enabled) {
	_chemostatMode = enabled;
}

bool BacteriaColony::getChemostatMode() const {
	return _chemostatMode;
}

void BacteriaColony::addBarrier(double x1, double y1, double x2, double y2) {
	_barriers.push_back(BarrierSegment{x1, y1, x2, y2});
}

const std::vector<BacteriaColony::BarrierSegment>& BacteriaColony::getBarriers() const {
	return _barriers;
}

const std::vector<double>& BacteriaColony::getMappedCellValues() const {
	return _mappedCellValues;
}

const std::string& BacteriaColony::getMappedCellExpression() const {
	return _mappedCellExpression;
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
	_ensureGroSeededBacteriaInitialized();
	_mappedCellExpression.clear();
	_mappedCellValues.clear();

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
	runtimeState.colonyTime = getColonyTime();
	runtimeState.simulationStep = getSimulationStep();
	runtimeState.populationSize = _populationSize;
	runtimeState.tickCount = _colonyTickCount;
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
		if (!_applyColonyMutations(result.colonyMutations, result, true, result.errorMessage)) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony aggregate Gro program failed: " + result.errorMessage;
			return result;
		}
		if (runtimeState.simulationStep > 0.0 &&
		    std::fabs(runtimeState.simulationStep - getSimulationStep()) > 1e-12) {
			// The classic Gro source uses set("dt", ...) as the authoritative colony step.
			setSimulationStep(runtimeState.simulationStep);
		}
		if (!_applyBioNetworkAssignments(result.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			return result;
		}
		_removeBioNetworkAssignmentVariables(runtimeState.variables);
		// The colony owns the persistent program state between executions, so the
		// plugin runtime hands back the updated scalar-variable map after each run.
		_runtimeVariables = runtimeState.variables;
		_runtimeVariables["dt"] = getSimulationStep();
		_colonyTickCount = runtimeState.tickCount;
		_applyRuntimePopulationMutations(result.populationMutations, runtimeState.populationSize);
		_applySignalFieldStep();
		_refreshBacteriaUpdateTime();
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
		_numSteps = fields->loadField("numSteps", DEFAULT.numSteps);
		_colonyTimeUnit = fields->loadField("colonyTimeUnit", DEFAULT.colonyTimeUnit);
		_initialPopulation = fields->loadField("initialPopulation", DEFAULT.initialPopulation);
		_gridWidth = fields->loadField("gridWidth", DEFAULT.gridWidth);
		_gridHeight = fields->loadField("gridHeight", DEFAULT.gridHeight);
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
	fields->saveField("numSteps", _numSteps, DEFAULT.numSteps, saveDefaultValues);
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
	if (_numSteps == 0) {
		errorMessage += "BacteriaColony number of steps must be greater than zero. ";
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
	_groSeedsApplied = false;
	_currentStep = 0;
	_colonyTickCount = 0;
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

void BacteriaColony::_createAttachedAttributes() {
	if (_groProgram != nullptr) {
		_optionalEditableDataDefinitionInsert("GroProgram", _groProgram);
	} else {
		_optionalEditableDataDefinitionRemove("GroProgram");
	}
	if (_bioNetwork != nullptr) {
		_optionalEditableDataDefinitionInsert("BioNetwork", _bioNetwork);
	} else {
		_optionalEditableDataDefinitionRemove("BioNetwork");
	}
	if (_signalGrid != nullptr) {
		_optionalEditableDataDefinitionInsert("SignalGrid", _signalGrid);
	} else {
		_optionalEditableDataDefinitionRemove("SignalGrid");
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
		_applySignalFieldStep();
		_refreshBacteriaUpdateTime();
		traceSimulation(this, "Bacteria colony step executed at model time " + std::to_string(getColonyTime()));
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

	_currentStep++;
	if (_currentStep > _numSteps) {
		_currentStep = 0;
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

	// The biochemical state is exposed as species identifiers only; BioNetwork
	// time belongs to its own solver and is not synchronized by BacteriaColony.
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
	std::vector<GroSeedDefinition> newSeedDefinitions;
	if (_groProgram == nullptr) {
		_groSeedDefinitions.clear();
		_groSeedSignature.clear();
		_groSeedsApplied = false;
		return true;
	}

	const std::string& sourceCode = _groProgram->getSourceCode();
	double configuredDt = 0.0;
	if (parseGroConfiguredDt(sourceCode, configuredDt)) {
		setSimulationStep(configuredDt);
	}

	const std::vector<GroSeedPlacement> parsedPlacements = parseGroSeedPlacements(sourceCode);
	if (parsedPlacements.empty()) {
		if (!_groSeedDefinitions.empty()) {
			_groSeedsApplied = false;
		}
		_groSeedDefinitions.clear();
		_groSeedSignature.clear();
		return true;
	}

	unsigned int maxGridX = 0;
	unsigned int maxGridY = 0;
	double minPositionX = 0.0;
	double minPositionY = 0.0;
	double maxPositionX = 0.0;
	double maxPositionY = 0.0;
	bool hasAnySeedPosition = false;
	for (const GroSeedPlacement& placement : parsedPlacements) {
		if (!hasAnySeedPosition) {
			minPositionX = maxPositionX = placement.positionX;
			minPositionY = maxPositionY = placement.positionY;
			hasAnySeedPosition = true;
		} else {
			minPositionX = std::min(minPositionX, placement.positionX);
			minPositionY = std::min(minPositionY, placement.positionY);
			maxPositionX = std::max(maxPositionX, placement.positionX);
			maxPositionY = std::max(maxPositionY, placement.positionY);
		}
	}
	const double shiftX = minPositionX < 0.0 ? -minPositionX : 0.0;
	const double shiftY = minPositionY < 0.0 ? -minPositionY : 0.0;
	std::ostringstream seedSignature;
	for (const GroSeedPlacement& placement : parsedPlacements) {
		GroSeedDefinition definition;
		definition.gridX = static_cast<unsigned int>(std::max(0.0, std::round(placement.positionX + shiftX)));
		definition.gridY = static_cast<unsigned int>(std::max(0.0, std::round(placement.positionY + shiftY)));
		definition.programName = placement.programName;
		definition.programArguments = placement.programArguments;
		newSeedDefinitions.push_back(definition);
		seedSignature << definition.gridX << "," << definition.gridY << "," << definition.programName << "(";
		for (std::size_t index = 0; index < definition.programArguments.size(); ++index) {
			if (index != 0) {
				seedSignature << ",";
			}
			seedSignature << definition.programArguments[index];
		}
		seedSignature << ");";
		maxGridX = std::max(maxGridX, definition.gridX);
		maxGridY = std::max(maxGridY, definition.gridY);
	}
	const std::string newSeedSignature = seedSignature.str();
	if (newSeedSignature != _groSeedSignature) {
		_groSeedsApplied = false;
	}
	_groSeedDefinitions = std::move(newSeedDefinitions);
	_groSeedSignature = newSeedSignature;

	if (!_usesSignalGridDimensions()) {
		// Classic Gro seed placements define a discrete colony footprint even when no SignalGrid exists yet.
		const unsigned int requiredWidth = static_cast<unsigned int>(
				std::max(1.0, std::round(maxPositionX + shiftX + 1.0)));
		const unsigned int requiredHeight = static_cast<unsigned int>(
				std::max(1.0, std::round(maxPositionY + shiftY + 1.0)));
		_gridWidth = std::max(std::max(_gridWidth, maxGridX + 1), requiredWidth);
		_gridHeight = std::max(std::max(_gridHeight, maxGridY + 1), requiredHeight);
		if (_signalField.size() != static_cast<std::size_t>(_gridWidth) * static_cast<std::size_t>(_gridHeight)) {
			_signalField.assign(static_cast<std::size_t>(_gridWidth) * static_cast<std::size_t>(_gridHeight), 0.0);
		}
	}
	return true;
}

void BacteriaColony::_ensureGroSeededBacteriaInitialized() {
	if (_groSeedDefinitions.empty() || _groSeedsApplied) {
		return;
	}

	// Seed declarations are initial conditions from GRO source; apply them once
	// before execution so GUI-driven stepping starts from the authored colony.
	_rebuildInternalBacteriaFromGroSeeds();
	_groSeedsApplied = true;
}

void BacteriaColony::_rebuildInternalBacteriaFromGroSeeds() {
	_bacteria.clear();
	_nextBacteriumId = 1;
	for (const GroSeedDefinition& placement : _groSeedDefinitions) {
		_appendBacterium(placement);
	}
	_populationSize = static_cast<unsigned int>(_bacteria.size());
	_refreshBacteriaUpdateTime();
	_groSeedsApplied = true;
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

std::vector<std::vector<double>> BacteriaColony::_buildSignalMatrix() const {
	std::vector<std::vector<double>> matrix;
	const unsigned int width = getGridWidth();
	const unsigned int height = getGridHeight();
	matrix.reserve(height);
	for (unsigned int y = 0; y < height; ++y) {
		std::vector<double> row;
		row.reserve(width);
		for (unsigned int x = 0; x < width; ++x) {
			row.push_back(_signalValueAt(x, y));
		}
		matrix.push_back(std::move(row));
	}
	return matrix;
}

std::string BacteriaColony::_buildSignalFieldDump(std::size_t maxRows, std::size_t maxColumns) const {
	const std::vector<std::vector<double>> matrix = _buildSignalMatrix();
	const std::size_t availableRows = matrix.size();
	const std::size_t availableColumns = availableRows > 0 ? matrix.front().size() : 0;
	const std::size_t rowsToPrint = maxRows == 0 ? availableRows : std::min(maxRows, availableRows);
	const std::size_t columnsToPrint = maxColumns == 0 ? availableColumns : std::min(maxColumns, availableColumns);

	std::ostringstream dump;
	dump << "signal_matrix " << getGridWidth() << "x" << getGridHeight();
	if (rowsToPrint < availableRows || columnsToPrint < availableColumns) {
		dump << " (preview)";
	}
	dump << '\n';
	for (std::size_t rowIndex = 0; rowIndex < rowsToPrint; ++rowIndex) {
		dump << "row " << rowIndex << ':';
		for (std::size_t columnIndex = 0; columnIndex < columnsToPrint; ++columnIndex) {
			dump << ' ' << std::setprecision(6) << matrix[rowIndex][columnIndex];
		}
		if (columnsToPrint < availableColumns) {
			dump << " ...";
		}
		dump << '\n';
	}
	if (rowsToPrint < availableRows) {
		dump << "...";
	}
	return dump.str();
}

void BacteriaColony::_captureSignalFieldSnapshot(GroProgramRuntime::ExecutionResult& result,
                                                std::size_t maxRows,
                                                std::size_t maxColumns) const {
	const std::vector<std::vector<double>> matrix = _buildSignalMatrix();
	result.signalMatrixWidth = getGridWidth();
	result.signalMatrixHeight = getGridHeight();
	result.signalMatrixValues.clear();
	result.signalMatrixValues.reserve(static_cast<std::size_t>(result.signalMatrixWidth) *
	                                  static_cast<std::size_t>(result.signalMatrixHeight));
	for (const std::vector<double>& row : matrix) {
		result.signalMatrixValues.insert(result.signalMatrixValues.end(), row.begin(), row.end());
	}
	result.signalFieldDump = _buildSignalFieldDump(maxRows, maxColumns);
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

bool BacteriaColony::_applyColonyMutations(const std::vector<GroProgramRuntime::ColonyMutation>& mutations,
                                           GroProgramRuntime::ExecutionResult& result,
                                           bool allowStructureMutations,
                                           std::string& errorMessage) {
	(void)result;
	for (const GroProgramRuntime::ColonyMutation& mutation : mutations) {
		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::Reset) {
			if (!allowStructureMutations) {
				errorMessage = "BacteriaColony bacterium-scoped programs cannot reset the colony. ";
				return false;
			}
			_bacteria.clear();
			_nextBacteriumId = 1;
			_populationSize = 0;
			_colonyTickCount = 0;
			_groSeedsApplied = true;
			std::string signalErrorMessage;
			(void)_resetRuntimeSignalField(signalErrorMessage);
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SpawnSeed) {
			if (!allowStructureMutations) {
				errorMessage = "BacteriaColony bacterium-scoped programs cannot spawn seeds. ";
				return false;
			}
			if (mutation.arguments.size() < 2) {
				errorMessage = "BacteriaColony received an invalid runtime ecoli() mutation. ";
				return false;
			}
			const std::string mutationSource =
					"ecoli(" + trim(mutation.arguments[0]) + ", " + trim(mutation.arguments[1]) + ")";
			const std::vector<GroSeedPlacement> placements = parseGroSeedPlacements(mutationSource);
			if (placements.size() != 1) {
				errorMessage = "BacteriaColony could not interpret a seeded program from runtime ecoli(). ";
				return false;
			}
			const GroSeedPlacement& placement = placements.front();
			GroSeedDefinition seedDefinition;
			seedDefinition.gridX = static_cast<unsigned int>(std::max(0.0, std::round(placement.positionX)));
			seedDefinition.gridY = static_cast<unsigned int>(std::max(0.0, std::round(placement.positionY)));
			seedDefinition.programName = placement.programName;
			seedDefinition.programArguments = placement.programArguments;
			_appendBacterium(seedDefinition);
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SetChemostatMode) {
			if (mutation.numericArguments.size() != 1) {
				errorMessage = "BacteriaColony received an invalid chemostat mutation. ";
				return false;
			}
			setChemostatMode(mutation.numericArguments.front() != 0.0);
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::AddBarrier) {
			if (mutation.numericArguments.size() != 4) {
				errorMessage = "BacteriaColony received an invalid barrier mutation. ";
				return false;
			}
			addBarrier(mutation.numericArguments[0],
			           mutation.numericArguments[1],
			           mutation.numericArguments[2],
			           mutation.numericArguments[3]);
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::MapToCells) {
			if (mutation.expressionText.empty()) {
				errorMessage = "BacteriaColony received an empty map_to_cells expression. ";
				return false;
			}
			_mappedCellExpression = mutation.expressionText;
			_mappedCellValues.clear();
			_mappedCellValues.reserve(_bacteria.size());
			for (std::size_t index = 0; index < _bacteria.size(); ++index) {
				const BacteriumState& bacterium = _bacteria[index];
				GroProgramRuntimeState runtimeState = _createBacteriumRuntimeState(bacterium, index);
				double mappedValue = 0.0;
				std::string evaluationErrorMessage;
				if (!GroProgramRuntime::evaluateExpression(mutation.expressionText, runtimeState, mappedValue, evaluationErrorMessage)) {
					errorMessage = "BacteriaColony map_to_cells expression failed: " + evaluationErrorMessage;
					return false;
				}
				_mappedCellValues.push_back(mappedValue);
			}
			result.mappedCellExpression = _mappedCellExpression;
			result.mappedCellValues = _mappedCellValues;
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SetSignalGridWidth) {
			if (mutation.numericArguments.size() != 1) {
				errorMessage = "BacteriaColony received an invalid signal_grid_width mutation. ";
				return false;
			}
			setGridWidth(static_cast<unsigned int>(std::max(1.0, std::floor(mutation.numericArguments.front()))));
			std::string resetErrorMessage;
			if (!_resetRuntimeSignalField(resetErrorMessage)) {
				errorMessage = "BacteriaColony failed to resize signal field after signal_grid_width mutation: " + resetErrorMessage;
				return false;
			}
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SetSignalGridHeight) {
			if (mutation.numericArguments.size() != 1) {
				errorMessage = "BacteriaColony received an invalid signal_grid_height mutation. ";
				return false;
			}
			setGridHeight(static_cast<unsigned int>(std::max(1.0, std::floor(mutation.numericArguments.front()))));
			std::string resetErrorMessage;
			if (!_resetRuntimeSignalField(resetErrorMessage)) {
				errorMessage = "BacteriaColony failed to resize signal field after signal_grid_height mutation: " + resetErrorMessage;
				return false;
			}
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SetSignalAt) {
			if (mutation.numericArguments.size() != 4) {
				errorMessage = "BacteriaColony received an invalid set_signal mutation. ";
				return false;
			}
			const double xValue = mutation.numericArguments[1];
			const double yValue = mutation.numericArguments[2];
			const double signalValue = mutation.numericArguments[3];
			const unsigned int x = toCenteredGridIndex(xValue, getGridWidth());
			const unsigned int y = toCenteredGridIndex(yValue, getGridHeight());
			_setSignalValueAt(x, y, signalValue);
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::SetSignalRect) {
			if (mutation.numericArguments.size() != 6) {
				errorMessage = "BacteriaColony received an invalid set_signal_rect mutation. ";
				return false;
			}
			const double x1Value = mutation.numericArguments[1];
			const double y1Value = mutation.numericArguments[2];
			const double x2Value = mutation.numericArguments[3];
			const double y2Value = mutation.numericArguments[4];
			const double signalValue = mutation.numericArguments[5];
			const long long rawX1 = std::llround(x1Value + 0.5 * static_cast<double>(getGridWidth()) - 0.5);
			const long long rawY1 = std::llround(y1Value + 0.5 * static_cast<double>(getGridHeight()) - 0.5);
			const long long rawX2 = std::llround(x2Value + 0.5 * static_cast<double>(getGridWidth()) - 0.5);
			const long long rawY2 = std::llround(y2Value + 0.5 * static_cast<double>(getGridHeight()) - 0.5);
			const long long minX = std::min(rawX1, rawX2);
			const long long maxX = std::max(rawX1, rawX2);
			const long long minY = std::min(rawY1, rawY2);
			const long long maxY = std::max(rawY1, rawY2);
			const long long upperX = static_cast<long long>(getGridWidth() > 0 ? getGridWidth() - 1 : 0);
			const long long upperY = static_cast<long long>(getGridHeight() > 0 ? getGridHeight() - 1 : 0);
			for (long long x = std::max(0ll, minX); x <= std::min(upperX, maxX); ++x) {
				for (long long y = std::max(0ll, minY); y <= std::min(upperY, maxY); ++y) {
					_setSignalValueAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), signalValue);
				}
			}
			continue;
		}

		if (mutation.type == GroProgramRuntime::ColonyMutation::Type::GetSignalMatrix ||
		    mutation.type == GroProgramRuntime::ColonyMutation::Type::DumpSignalField) {
			_captureSignalFieldSnapshot(result, mutation.previewRows, mutation.previewColumns);
			continue;
		}
	}

	_populationSize = static_cast<unsigned int>(_bacteria.size());
	return true;
}

bool BacteriaColony::_executeSeededNamedGroPrograms(const GroProgramIr& ir,
                                                    GroProgramRuntime::ExecutionResult& result) {
	std::string unsupportedCommand;
	for (const auto& programEntry : ir.namedPrograms) {
		if (programEntry.first == "main") {
			continue;
		}
		if (_containsBacteriumScopedOnlyUnsupportedCommand(programEntry.second.commands, unsupportedCommand)) {
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
		if (!_applyColonyMutations(preludeResult.colonyMutations, result, true, result.errorMessage)) {
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
		for (const auto& mutation : preludeResult.colonyMutations) {
			result.colonyMutations.push_back(mutation);
		}
	}

	const auto mainEntry = ir.namedPrograms.find("main");
	if (mainEntry != ir.namedPrograms.end()) {
		GroProgramIr mainIr;
		mainIr.sourceForm = GroProgramAst::SourceForm::ProgramBlock;
		mainIr.programName = "main";
		mainIr.commands = mainEntry->second.commands;

		GroProgramRuntimeState mainState;
		mainState.colonyTime = getColonyTime();
		mainState.simulationStep = getSimulationStep();
		mainState.populationSize = _populationSize;
		mainState.tickCount = _colonyTickCount;
		mainState.variables = _runtimeVariables;
		mainState.variables["dt"] = mainState.simulationStep;
		_appendBioNetworkContextVariables(mainState, result.errorMessage);
		if (!result.errorMessage.empty()) {
			result.succeeded = false;
			return false;
		}

		GroProgramRuntime::ExecutionResult mainResult = runtime.execute(mainIr, mainState);
		if (!mainResult.succeeded) {
			result = mainResult;
			result.errorMessage = "BacteriaColony main program failed: " + result.errorMessage;
			return false;
		}
		if (!mainResult.signalMutations.empty()) {
			result.succeeded = false;
			result.errorMessage =
			        "BacteriaColony main program cannot emit or absorb local signals without bacterium context. ";
			return false;
		}
		if (mainState.simulationStep > 0.0 &&
		    std::fabs(mainState.simulationStep - getSimulationStep()) > 1e-12) {
			setSimulationStep(mainState.simulationStep);
		}
		if (!_applyBioNetworkAssignments(mainResult.assignedVariables, result.errorMessage)) {
			result.succeeded = false;
			return false;
		}
		if (!_applyColonyMutations(mainResult.colonyMutations, result, true, result.errorMessage)) {
			result.succeeded = false;
			return false;
		}
		_removeBioNetworkAssignmentVariables(mainState.variables);
		_runtimeVariables = mainState.variables;
		_runtimeVariables["dt"] = getSimulationStep();
		_colonyTickCount = mainState.tickCount;
		mainState.populationSize = _populationSize;
		_applyRuntimePopulationMutations(mainResult.populationMutations, mainState.populationSize);
		result.executedCommands += mainResult.executedCommands;
		result.assignedVariables.insert(mainResult.assignedVariables.begin(), mainResult.assignedVariables.end());
		result.unsupportedCommands.insert(result.unsupportedCommands.end(),
		                                  mainResult.unsupportedCommands.begin(),
		                                  mainResult.unsupportedCommands.end());
		result.skippedRawStatements.insert(result.skippedRawStatements.end(),
		                                   mainResult.skippedRawStatements.begin(),
		                                   mainResult.skippedRawStatements.end());
		for (const auto& mutation : mainResult.colonyMutations) {
			result.colonyMutations.push_back(mutation);
		}
	}

	_refreshBacteriaUpdateTime();

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
		bacteriumIr.commands = programEntry->second.commands;

		GroProgramRuntimeState runtimeState = _createBacteriumRuntimeState(bacterium, bacteriumIndex);
		_bindProgramArguments(runtimeState, programEntry->second.parameters, bacterium.programArguments);
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
		bacterium.tickCount = runtimeState.tickCount;
		_syncBacteriumSpatialState(bacterium, runtimeState);
		_applyBacteriumGrowth(bacterium);
		bacterium.justDivided = false;
		bacterium.daughter = false;
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
		for (const auto& mutation : bacteriumResult.motionMutations) {
			result.motionMutations.push_back(mutation);
		}
		for (const auto& mutation : bacteriumResult.colonyMutations) {
			result.colonyMutations.push_back(mutation);
		}

		_applyBacteriumSignalMutations(bacterium, bacteriumResult.signalMutations);
		if (!bacteriumResult.colonyMutations.empty() &&
		    !_applyColonyMutations(bacteriumResult.colonyMutations, result, false, result.errorMessage)) {
			result.succeeded = false;
			result.errorMessage = "BacteriaColony bacterium-scoped execution failed for bacterium id " +
			                      std::to_string(bacteriumId) + ": " + result.errorMessage;
			return false;
		}
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
	_applySignalFieldStep();
	for (BacteriumState& bacterium : _bacteria) {
		_updateBacteriumSpatialMotion(bacterium);
	}
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

	// In bacterium scope one colony execution still represents a single colony
	// step, but time itself is read from ModelSimulation and is not advanced here.
	_refreshBacteriaUpdateTime();

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
		const auto programEntry = ir.namedPrograms.find(bacterium.programName);
		if (programEntry != ir.namedPrograms.end()) {
			_bindProgramArguments(runtimeState, programEntry->second.parameters, bacterium.programArguments);
		}
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
		bacterium.tickCount = runtimeState.tickCount;
		_syncBacteriumSpatialState(bacterium, runtimeState);
		_applyBacteriumGrowth(bacterium);
		bacterium.justDivided = false;
		bacterium.daughter = false;
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
		for (const auto& mutation : bacteriumResult.motionMutations) {
			result.motionMutations.push_back(mutation);
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
	_applySignalFieldStep();
	for (BacteriumState& bacterium : _bacteria) {
		_updateBacteriumSpatialMotion(bacterium);
	}
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
		    command.functionName == "reset" ||
		    command.functionName == "ecoli" ||
		    command.functionName == "set_population" ||
		    command.functionName == "map_to_cells" ||
		    command.functionName == "get_signal_matrix" ||
		    command.functionName == "dump_signal_field") {
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
	runtimeState.tickCount = bacterium.tickCount;
	runtimeState.contextVariables["bacterium_id"] = static_cast<double>(bacterium.id);
	runtimeState.contextVariables["bacterium_parent_id"] = static_cast<double>(bacterium.parentId);
	runtimeState.contextVariables["bacterium_generation"] = static_cast<double>(bacterium.generation);
	runtimeState.contextVariables["bacterium_divisions"] = static_cast<double>(bacterium.divisionCount);
	runtimeState.contextVariables["bacterium_birth_time"] = bacterium.birthTime;
	runtimeState.contextVariables["bacterium_age"] = getBacteriumAge(bacteriumIndex);
	runtimeState.contextVariables["bacterium_grid_x"] = static_cast<double>(bacterium.gridX);
	runtimeState.contextVariables["bacterium_grid_y"] = static_cast<double>(bacterium.gridY);
	runtimeState.contextVariables["bacterium_x"] = bacterium.positionX;
	runtimeState.contextVariables["bacterium_y"] = bacterium.positionY;
	runtimeState.contextVariables["bacterium_direction"] = bacterium.directionRadians;
	runtimeState.contextVariables["bacterium_volume"] = bacterium.volume;
	runtimeState.contextVariables["bacterium_size"] = bacterium.size;
	runtimeState.contextVariables["bacterium_gfp"] = bacterium.gfp;
	runtimeState.contextVariables["bacterium_rfp"] = bacterium.rfp;
	runtimeState.contextVariables["bacterium_yfp"] = bacterium.yfp;
	runtimeState.contextVariables["bacterium_cfp"] = bacterium.cfp;
	runtimeState.contextVariables["bacterium_speed"] = bacterium.speed;
	runtimeState.contextVariables["bacterium_index"] = static_cast<double>(bacteriumIndex);
	runtimeState.contextVariables["selected"] = 0.0;
	runtimeState.contextVariables["id"] = static_cast<double>(bacterium.id);
	runtimeState.contextVariables["just_divided"] = bacterium.justDivided ? 1.0 : 0.0;
	runtimeState.contextVariables["daughter"] = bacterium.daughter ? 1.0 : 0.0;
	runtimeState.contextVariables["local_signal"] = _signalValueAt(bacterium.gridX, bacterium.gridY);
	runtimeState.contextVariables["neighbor_signal_sum"] = _computeNeighborSignalSum(bacterium.gridX, bacterium.gridY);
	runtimeState.contextVariables["local_bacteria_count"] = static_cast<double>(
			_computeLocalBacteriaCount(bacterium.gridX, bacterium.gridY));
	// Colony-level scalar declarations such as signal handles are visible to each
	// bacterium. Shared globals from `main()` stay authoritative across ticks, so
	// persisted bacterium-local variables only overlay names that are not global.
	runtimeState.variables = _runtimeVariables;
	for (const auto& entry : bacterium.runtimeVariables) {
		if (_runtimeVariables.find(entry.first) == _runtimeVariables.end()) {
			runtimeState.variables[entry.first] = entry.second;
		}
	}
	runtimeState.variables["x"] = bacterium.positionX;
	runtimeState.variables["y"] = bacterium.positionY;
	runtimeState.variables["direction"] = bacterium.directionRadians;
	runtimeState.variables["theta"] = bacterium.directionRadians;
	runtimeState.variables["volume"] = bacterium.volume;
	runtimeState.variables["size"] = bacterium.size;
	runtimeState.variables["gfp"] = bacterium.gfp;
	runtimeState.variables["rfp"] = bacterium.rfp;
	runtimeState.variables["yfp"] = bacterium.yfp;
	runtimeState.variables["cfp"] = bacterium.cfp;
	runtimeState.variables["speed"] = bacterium.speed;
	runtimeState.variables["id"] = static_cast<double>(bacterium.id);
	runtimeState.variables["selected"] = 0.0;
	runtimeState.variables["just_divided"] = bacterium.justDivided ? 1.0 : 0.0;
	runtimeState.variables["daughter"] = bacterium.daughter ? 1.0 : 0.0;
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

			const BacteriumState parent = _bacteria[parentIndex];
			_bacteria[parentIndex].divisionCount += mutation.value;
			_bacteria[parentIndex].lastDivisionTime = getColonyTime();
			_bacteria[parentIndex].volume = std::max(_bacteria[parentIndex].volume, parent.volume + 0.15 * mutation.value);
			_bacteria[parentIndex].size = std::max(0.75, std::sqrt(std::max(0.1, _bacteria[parentIndex].volume)));
			const std::string childProgramName = _bacteria[parentIndex].programName;
			for (unsigned int i = 0; i < mutation.value; ++i) {
				_appendBacterium(bacteriumId, parentGeneration + 1, childProgramName);
				BacteriumState& child = _bacteria.back();
				child.programArguments = parent.programArguments;
				child.positionX = parent.positionX + 0.35 * std::cos(parent.directionRadians + 0.35 * static_cast<double>(i + 1));
				child.positionY = parent.positionY + 0.35 * std::sin(parent.directionRadians + 0.35 * static_cast<double>(i + 1));
				_setBacteriumPosition(child, child.positionX, child.positionY);
				child.directionRadians = parent.directionRadians + 0.2 * static_cast<double>(i + 1);
				child.volume = std::max(0.75, parent.volume * 0.92);
				child.size = std::max(0.65, std::sqrt(std::max(0.1, child.volume)));
				child.gfp = parent.gfp;
				child.rfp = parent.rfp;
				child.yfp = parent.yfp;
				child.cfp = parent.cfp;
				child.speed = parent.speed;
				child.tickCount = 0;
				child.justDivided = true;
				child.daughter = true;
			}
			_bacteria[parentIndex].justDivided = true;
			_bacteria[parentIndex].daughter = false;
			result.populationMutations.push_back(mutation);
			continue;
		}

		if (mutation.type == GroProgramRuntime::PopulationMutationType::Divide) {
			const std::size_t parentIndex = _findBacteriumIndexById(bacteriumId);
			if (parentIndex >= _bacteria.size()) {
				result.succeeded = false;
				result.errorMessage = "BacteriaColony could not find divide parent bacterium id " +
				                      std::to_string(bacteriumId) + ". ";
				return;
			}

			// A bacterium-scoped divide reproduces only the current bacterium,
			// unlike aggregate divide(), which doubles the whole population.
			const BacteriumState parent = _bacteria[parentIndex];
			++_bacteria[parentIndex].divisionCount;
			_bacteria[parentIndex].lastDivisionTime = getColonyTime();
			const std::string childProgramName = _bacteria[parentIndex].programName;
			_appendBacterium(bacteriumId, parentGeneration + 1, childProgramName);
			BacteriumState& child = _bacteria.back();
			child.programArguments = parent.programArguments;
			child.positionX = parent.positionX + 0.45 * std::cos(parent.directionRadians + 0.45);
			child.positionY = parent.positionY + 0.45 * std::sin(parent.directionRadians + 0.45);
			_setBacteriumPosition(child, child.positionX, child.positionY);
			child.directionRadians = parent.directionRadians + 0.3;
			child.volume = std::max(0.65, parent.volume * 0.5);
			child.size = std::max(0.55, std::sqrt(std::max(0.1, child.volume)));
			child.gfp = parent.gfp * 0.85;
			child.rfp = parent.rfp * 0.85;
			child.yfp = parent.yfp * 0.85;
			child.cfp = parent.cfp * 0.85;
			child.speed = parent.speed;
			child.tickCount = 0;
			child.justDivided = true;
			child.daughter = true;
			_bacteria[parentIndex].volume = std::max(0.65, parent.volume * 0.5);
			_bacteria[parentIndex].size = std::max(0.55, std::sqrt(std::max(0.1, _bacteria[parentIndex].volume)));
			_bacteria[parentIndex].justDivided = true;
			_bacteria[parentIndex].daughter = false;
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
				"BacteriaColony bacterium-scoped programs only support grow, divide, and die population mutations. ";
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
	bacterium.positionX = -1.0;
	bacterium.positionY = -1.0;
	bacterium.directionRadians = 0.0;
	bacterium.volume = 1.0;
	bacterium.size = 1.0;
	bacterium.gfp = 0.0;
	bacterium.rfp = 0.0;
	bacterium.speed = 0.15;
	bacterium.tickCount = 0;
	bacterium.hasExplicitGridPosition = false;
	bacterium.justDivided = false;
	bacterium.daughter = false;
	bacterium.alive = true;
	_assignBacteriumGridPosition(bacterium, _bacteria.size());
	_initializeBacteriumPhenotype(bacterium, _bacteria.size());
	_bacteria.push_back(bacterium);
}

void BacteriaColony::_appendBacterium(const GroSeedDefinition& seedDefinition) {
	_appendBacterium(0, 0, seedDefinition.programName);
	if (_bacteria.empty()) {
		return;
	}

	BacteriumState& bacterium = _bacteria.back();
	bacterium.programArguments = seedDefinition.programArguments;
	bacterium.gridX = seedDefinition.gridX;
	bacterium.gridY = seedDefinition.gridY;
	bacterium.hasExplicitGridPosition = true;
	bacterium.positionX = static_cast<double>(seedDefinition.gridX);
	bacterium.positionY = static_cast<double>(seedDefinition.gridY);
	bacterium.size = std::max(0.9, bacterium.size);
	bacterium.volume = std::max(1.0, bacterium.volume);
	if (bacterium.programArguments.size() == 1) {
		bacterium.runtimeVariables["g0"] = bacterium.programArguments.front();
	}
	if (bacterium.runtimeVariables.find("g0") == bacterium.runtimeVariables.end() &&
	    bacterium.programName == "oscillator") {
		// Preserve a visible fluorescence baseline for the bundled oscillator demo
		// even when no explicit seed argument was parsed from the source.
		bacterium.runtimeVariables["g0"] = 1.0;
	}
	for (std::size_t index = 0; index < bacterium.programArguments.size(); ++index) {
		bacterium.runtimeVariables["program_argument_" + std::to_string(index)] = bacterium.programArguments[index];
	}
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

void BacteriaColony::_applyBacteriumGrowth(BacteriumState& bacterium) const {
	if (!bacterium.alive) {
		return;
	}

	// Keep the visible phenotype moving even when a GRO program leaves growth
	// implicit, which is the common case in the bundled demos.
	double growthRate = 0.045;
	const auto growthRateIt = bacterium.runtimeVariables.find("ecoli_growth_rate");
	if (growthRateIt != bacterium.runtimeVariables.end()) {
		growthRate = std::max(0.0, growthRateIt->second);
	}
	growthRate += 0.004 * static_cast<double>(bacterium.generation);
	const double localSignal = _signalValueAt(bacterium.gridX, bacterium.gridY);
	growthRate += std::clamp(localSignal * 0.0025, 0.0, 0.08);

	const double stepScale = std::max(0.25, getSimulationStep() * 5.0);
	const double deltaVolume = std::clamp(growthRate * stepScale, 0.01, 0.35);
	bacterium.volume = std::max(0.1, bacterium.volume + deltaVolume);
	bacterium.size = std::max(0.65, std::sqrt(std::max(0.1, bacterium.volume)));
	bacterium.speed = std::max(0.03, bacterium.speed + 0.015 * deltaVolume);

	bacterium.runtimeVariables["ecoli_growth_rate"] = growthRate;
	bacterium.runtimeVariables["volume"] = bacterium.volume;
	bacterium.runtimeVariables["size"] = bacterium.size;
	bacterium.runtimeVariables["speed"] = bacterium.speed;
}

void BacteriaColony::_rebuildBacteriaGridPositions() {
	for (std::size_t index = 0; index < _bacteria.size(); ++index) {
		_assignBacteriumGridPosition(_bacteria[index], index);
	}
}

void BacteriaColony::_assignBacteriumGridPosition(BacteriumState& bacterium, std::size_t index) const {
	if (bacterium.hasExplicitGridPosition) {
		_setBacteriumPosition(bacterium, bacterium.positionX, bacterium.positionY);
		return;
	}
	if (getGridWidth() == 0 || getGridHeight() == 0) {
		bacterium.gridX = 0;
		bacterium.gridY = 0;
		bacterium.positionX = 0.0;
		bacterium.positionY = 0.0;
		return;
	}
	if (!std::isfinite(bacterium.positionX) || !std::isfinite(bacterium.positionY) ||
	    bacterium.positionX < 0.0 || bacterium.positionY < 0.0) {
		bacterium.positionX = static_cast<double>(index % getGridWidth());
		bacterium.positionY = static_cast<double>((index / getGridWidth()) % getGridHeight());
	}
	_setBacteriumPosition(bacterium, bacterium.positionX, bacterium.positionY);
}

void BacteriaColony::_initializeBacteriumPhenotype(BacteriumState& bacterium, std::size_t index) const {
	constexpr double kTwoPi = 6.28318530717958647692;
	if (!bacterium.hasExplicitGridPosition) {
		if (getGridWidth() > 0 && getGridHeight() > 0) {
			bacterium.positionX = static_cast<double>(bacterium.gridX);
			bacterium.positionY = static_cast<double>(bacterium.gridY);
		} else {
			bacterium.positionX = 0.0;
			bacterium.positionY = 0.0;
		}
	}

	const double generationFactor = static_cast<double>(bacterium.generation);
	bacterium.volume = std::max(1.0, bacterium.volume);
	bacterium.size = std::max(0.75, std::sqrt(std::max(0.1, bacterium.volume)));
	bacterium.gfp = std::max(0.0, bacterium.gfp);
	bacterium.rfp = std::max(0.0, bacterium.rfp);
	bacterium.yfp = std::max(0.0, bacterium.yfp);
	bacterium.cfp = std::max(0.0, bacterium.cfp);
	bacterium.speed = std::max(0.05, bacterium.speed + 0.01 * generationFactor);
	if (!bacterium.hasExplicitGridPosition) {
		const double baseDirection = 0.85 * static_cast<double>(index + 1) + 0.41 * generationFactor + 0.19 * static_cast<double>(bacterium.id);
		bacterium.directionRadians = std::fmod(baseDirection, kTwoPi);
	}
	_setBacteriumPosition(bacterium, bacterium.positionX, bacterium.positionY);
}

void BacteriaColony::_setBacteriumPosition(BacteriumState& bacterium, double x, double y) const {
	const double clampedX = _clampPositionX(x);
	const double clampedY = _clampPositionY(y);
	bacterium.positionX = clampedX;
	bacterium.positionY = clampedY;
	bacterium.gridX = static_cast<unsigned int>(std::llround(clampedX));
	bacterium.gridY = static_cast<unsigned int>(std::llround(clampedY));
}

double BacteriaColony::_clampPositionX(double value) const {
	const double maxX = getGridWidth() > 0 ? static_cast<double>(getGridWidth() - 1) : 0.0;
	return std::clamp(value, 0.0, maxX);
}

double BacteriaColony::_clampPositionY(double value) const {
	const double maxY = getGridHeight() > 0 ? static_cast<double>(getGridHeight() - 1) : 0.0;
	return std::clamp(value, 0.0, maxY);
}

void BacteriaColony::_syncBacteriumSpatialState(BacteriumState& bacterium, const GroProgramRuntimeState& runtimeState) const {
	const auto positionX = runtimeState.variables.find("x");
	if (positionX != runtimeState.variables.end()) {
		bacterium.positionX = positionX->second;
	}
	const auto positionY = runtimeState.variables.find("y");
	if (positionY != runtimeState.variables.end()) {
		bacterium.positionY = positionY->second;
	}
	const auto direction = runtimeState.variables.find("direction");
	if (direction != runtimeState.variables.end()) {
		bacterium.directionRadians = direction->second;
	} else {
		const auto theta = runtimeState.variables.find("theta");
		if (theta != runtimeState.variables.end()) {
			bacterium.directionRadians = theta->second;
		}
	}
	const auto volume = runtimeState.variables.find("volume");
	if (volume != runtimeState.variables.end()) {
		bacterium.volume = std::max(0.0, volume->second);
	}
	const auto size = runtimeState.variables.find("size");
	if (size != runtimeState.variables.end()) {
		bacterium.size = std::max(0.0, size->second);
	}
	const auto gfp = runtimeState.variables.find("gfp");
	if (gfp != runtimeState.variables.end()) {
		bacterium.gfp = std::max(0.0, gfp->second);
	}
	const auto rfp = runtimeState.variables.find("rfp");
	if (rfp != runtimeState.variables.end()) {
		bacterium.rfp = std::max(0.0, rfp->second);
	}
	const auto yfp = runtimeState.variables.find("yfp");
	if (yfp != runtimeState.variables.end()) {
		bacterium.yfp = std::max(0.0, yfp->second);
	}
	const auto cfp = runtimeState.variables.find("cfp");
	if (cfp != runtimeState.variables.end()) {
		bacterium.cfp = std::max(0.0, cfp->second);
	}
	const auto speed = runtimeState.variables.find("speed");
	if (speed != runtimeState.variables.end()) {
		bacterium.speed = std::max(0.0, speed->second);
	}

	if (bacterium.size <= 0.0) {
		bacterium.size = std::max(0.75, std::sqrt(std::max(0.1, bacterium.volume)));
	}
	if (bacterium.volume <= 0.0) {
		bacterium.volume = std::max(0.1, bacterium.size * bacterium.size);
	}
	bacterium.size = std::max(bacterium.size, std::sqrt(std::max(0.1, bacterium.volume)));
	_setBacteriumPosition(bacterium, bacterium.positionX, bacterium.positionY);
}

void BacteriaColony::_updateBacteriumSpatialMotion(BacteriumState& bacterium) const {
	constexpr double kPi = 3.14159265358979323846;
	if (!bacterium.alive) {
		return;
	}

	double direction = bacterium.directionRadians;
	if (!std::isfinite(direction)) {
		direction = 0.0;
	}
	double speed = bacterium.speed;
	if (!std::isfinite(speed) || speed <= 0.0) {
		speed = 0.08 + 0.01 * static_cast<double>(bacterium.generation);
	}
	const double stepScale = std::max(0.35, getSimulationStep() * 2.5);
	double nextX = bacterium.positionX + std::cos(direction) * speed * stepScale;
	double nextY = bacterium.positionY + std::sin(direction) * speed * stepScale;

	const double maxX = getGridWidth() > 0 ? static_cast<double>(getGridWidth() - 1) : 0.0;
	const double maxY = getGridHeight() > 0 ? static_cast<double>(getGridHeight() - 1) : 0.0;
	bool reflected = false;
	if (nextX < 0.0 || nextX > maxX) {
		nextX = std::clamp(nextX, 0.0, maxX);
		direction = kPi - direction;
		reflected = true;
	}
	if (nextY < 0.0 || nextY > maxY) {
		nextY = std::clamp(nextY, 0.0, maxY);
		direction = -direction;
		reflected = true;
	}
	if (reflected) {
		bacterium.directionRadians = direction;
	}
	_setBacteriumPosition(bacterium, nextX, nextY);
}

void BacteriaColony::_bindProgramArguments(GroProgramRuntimeState& runtimeState,
                                           const std::vector<std::string>& parameterNames,
                                           const std::vector<double>& arguments) const {
	if (arguments.empty()) {
		return;
	}

	const std::size_t boundArgumentCount = std::min(parameterNames.size(), arguments.size());
	for (std::size_t index = 0; index < boundArgumentCount; ++index) {
		runtimeState.variables[parameterNames[index]] = arguments[index];
		runtimeState.contextVariables["program_parameter_" + std::to_string(index)] = arguments[index];
	}
	if (parameterNames.empty() && arguments.size() == 1) {
		// Preserve the current GRO default convention where a single anonymous
		// argument is exposed through g0 when the source uses program oscillator(g0).
		runtimeState.variables["g0"] = arguments.front();
	}
	for (std::size_t index = boundArgumentCount; index < arguments.size(); ++index) {
		runtimeState.variables["program_argument_" + std::to_string(index)] = arguments[index];
	}
	runtimeState.contextVariables["program_argument_count"] = static_cast<double>(arguments.size());
	runtimeState.contextVariables["program_parameter_count"] = static_cast<double>(parameterNames.size());
}

bool BacteriaColony::_usesSignalGridDimensions() const {
	return _signalGrid != nullptr;
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

// void BacteriaColony::_createInternalStatisticReporters() { }

// void BacteriaColony::_createEditableDataDefinitions() { }
