/*
 * File:   BacteriaColony.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiologicalModeling/GroProgramCompiler.h"
#include "plugins/data/BiologicalModeling/GroProgramParser.h"

#include <stdexcept>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BacteriaColony::GetPluginInformation;
}
#endif

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
	_parentModel->getControls()->insert(propSimulationStep);
	_parentModel->getControls()->insert(propInitialColonyTime);
	_parentModel->getControls()->insert(propInitialPopulation);
	_parentModel->getControls()->insert(propGridWidth);
	_parentModel->getControls()->insert(propGridHeight);

	_addProperty(propGroProgram);
	_addProperty(propSimulationStep);
	_addProperty(propInitialColonyTime);
	_addProperty(propInitialPopulation);
	_addProperty(propGridWidth);
	_addProperty(propGridHeight);
}

std::string BacteriaColony::show() {
	return ModelComponent::show() +
	       ",groProgram=\"" + (_groProgram != nullptr ? _groProgram->getName() : "") + "\"" +
	       ",simulationStep=" + Util::StrTruncIfInt(std::to_string(_simulationStep)) +
	       ",initialColonyTime=" + Util::StrTruncIfInt(std::to_string(_initialColonyTime)) +
	       ",colonyTime=" + Util::StrTruncIfInt(std::to_string(_colonyTime)) +
	       ",initialPopulation=" + std::to_string(_initialPopulation) +
	       ",populationSize=" + std::to_string(_populationSize) +
	       ",gridWidth=" + std::to_string(_gridWidth) +
	       ",gridHeight=" + std::to_string(_gridHeight);
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
	info->setDescriptionHelp("Self-contained biological simulation component for Gro-inspired bacteria colonies. "
	                         "When an entity arrives, the colony advances one configured Gro/runtime step and then "
	                         "forwards the entity through its output connection. This first slice owns an internal "
	                         "colony clock, population summary, and discrete grid configuration while leaving complete "
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

void BacteriaColony::setInitialColonyTime(double initialColonyTime) {
	_initialColonyTime = initialColonyTime;
}

double BacteriaColony::getInitialColonyTime() const {
	return _initialColonyTime;
}

double BacteriaColony::getColonyTime() const {
	return _colonyTime;
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
	if (_colonyTime < bacterium.birthTime) {
		return 0.0;
	}
	return _colonyTime - bacterium.birthTime;
}

void BacteriaColony::setGridWidth(unsigned int gridWidth) {
	_gridWidth = gridWidth;
	_rebuildBacteriaGridPositions();
}

unsigned int BacteriaColony::getGridWidth() const {
	return _gridWidth;
}

void BacteriaColony::setGridHeight(unsigned int gridHeight) {
	_gridHeight = gridHeight;
	_rebuildBacteriaGridPositions();
}

unsigned int BacteriaColony::getGridHeight() const {
	return _gridHeight;
}

double BacteriaColony::advanceColonyTime() {
	_colonyTime += _simulationStep;
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

	GroProgramParser::Result parseResult = GroProgramParser().parse(_groProgram->getSourceCode());
	if (!parseResult.accepted) {
		result.succeeded = false;
		result.errorMessage = parseResult.errorMessage;
		return result;
	}

	GroProgramIr ir = GroProgramCompiler().compile(parseResult.ast);
	GroProgramRuntimeState runtimeState;
	runtimeState.colonyTime = _colonyTime;
	runtimeState.simulationStep = _simulationStep;
	runtimeState.populationSize = _populationSize;

	result = GroProgramRuntime().execute(ir, runtimeState);
	if (result.succeeded) {
		_colonyTime = runtimeState.colonyTime;
		_applyRuntimePopulationMutations(result.populationMutations, runtimeState.populationSize);
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
		_simulationStep = fields->loadField("simulationStep", DEFAULT.simulationStep);
		_initialColonyTime = fields->loadField("initialColonyTime", DEFAULT.initialColonyTime);
		_colonyTime = _initialColonyTime;
		_initialPopulation = fields->loadField("initialPopulation", DEFAULT.initialPopulation);
		_gridWidth = fields->loadField("gridWidth", DEFAULT.gridWidth);
		_gridHeight = fields->loadField("gridHeight", DEFAULT.gridHeight);
		_rebuildInternalBacteria(_initialPopulation);
	}
	return res;
}

void BacteriaColony::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("groProgram", _groProgram != nullptr ? _groProgram->getName() : DEFAULT.groProgramName,
	                  DEFAULT.groProgramName, saveDefaultValues);
	fields->saveField("simulationStep", _simulationStep, DEFAULT.simulationStep, saveDefaultValues);
	fields->saveField("initialColonyTime", _initialColonyTime, DEFAULT.initialColonyTime, saveDefaultValues);
	fields->saveField("initialPopulation", _initialPopulation, DEFAULT.initialPopulation, saveDefaultValues);
	fields->saveField("gridWidth", _gridWidth, DEFAULT.gridWidth, saveDefaultValues);
	fields->saveField("gridHeight", _gridHeight, DEFAULT.gridHeight, saveDefaultValues);
}

bool BacteriaColony::_check(std::string& errorMessage) {
	bool resultAll = true;

	if (_simulationStep <= 0.0) {
		errorMessage += "BacteriaColony simulation step must be greater than zero. ";
		resultAll = false;
	}
	if (_initialPopulation == 0) {
		errorMessage += "BacteriaColony initial population must be greater than zero. ";
		resultAll = false;
	}
	if (_gridWidth == 0 || _gridHeight == 0) {
		errorMessage += "BacteriaColony grid dimensions must be greater than zero. ";
		resultAll = false;
	}

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<GroProgram>(), _groProgram, "GroProgram",
	                                                   errorMessage);
	if (_groProgram != nullptr) {
		resultAll &= _groProgram->validateSyntax(errorMessage);
	}

	return resultAll;
}

void BacteriaColony::_initBetweenReplications() {
	_colonyTime = _initialColonyTime;
	_rebuildInternalBacteria(_initialPopulation);
}

void BacteriaColony::_createInternalAndAttachedData() {
	if (_groProgram != nullptr) {
		_attachedDataInsert("GroProgram", _groProgram);
	} else {
		_attachedDataRemove("GroProgram");
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
		traceSimulation(this, "Bacteria colony internal time advanced to " + std::to_string(_colonyTime));
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

	_parentModel->sendEntityToComponent(entity, frontConnection);
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
				_bacteria[index].lastDivisionTime = _colonyTime;
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

void BacteriaColony::_appendBacterium(unsigned int parentId, unsigned int generation) {
	BacteriumState bacterium;
	bacterium.id = _nextBacteriumId++;
	bacterium.parentId = parentId;
	bacterium.generation = generation;
	bacterium.divisionCount = 0;
	bacterium.birthTime = _colonyTime;
	bacterium.lastUpdateTime = _colonyTime;
	bacterium.lastDivisionTime = 0.0;
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

void BacteriaColony::_refreshBacteriaUpdateTime() {
	for (BacteriumState& bacterium : _bacteria) {
		if (bacterium.alive) {
			bacterium.lastUpdateTime = _colonyTime;
		}
	}
}

void BacteriaColony::_rebuildBacteriaGridPositions() {
	for (std::size_t index = 0; index < _bacteria.size(); ++index) {
		_assignBacteriumGridPosition(_bacteria[index], index);
	}
}

void BacteriaColony::_assignBacteriumGridPosition(BacteriumState& bacterium, std::size_t index) const {
	if (_gridWidth == 0 || _gridHeight == 0) {
		bacterium.gridX = 0;
		bacterium.gridY = 0;
		return;
	}

	bacterium.gridX = static_cast<unsigned int>(index % _gridWidth);
	bacterium.gridY = static_cast<unsigned int>((index / _gridWidth) % _gridHeight);
}
