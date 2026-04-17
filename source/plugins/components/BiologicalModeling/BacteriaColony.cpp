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
	_connections->setMinOutputConnections(0);
	_connections->setMaxOutputConnections(0);

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
	info->setCategory("Biological Modeling");
	info->setSource(true);
	info->setSink(true);
	info->setMinimumInputs(0);
	info->setMaximumInputs(0);
	info->setMinimumOutputs(0);
	info->setMaximumOutputs(0);
	info->insertDynamicLibFileDependence("groprogram.so");
	info->setDescriptionHelp("Self-contained biological simulation component for Gro-inspired bacteria colonies. "
	                         "This first slice owns an internal colony clock, population summary, and discrete grid "
	                         "configuration while leaving complete Gro parsing and execution semantics to future "
	                         "plugin-side helpers.");
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

void BacteriaColony::setGridWidth(unsigned int gridWidth) {
	_gridWidth = gridWidth;
}

unsigned int BacteriaColony::getGridWidth() const {
	return _gridWidth;
}

void BacteriaColony::setGridHeight(unsigned int gridHeight) {
	_gridHeight = gridHeight;
}

unsigned int BacteriaColony::getGridHeight() const {
	return _gridHeight;
}

double BacteriaColony::advanceColonyTime() {
	_colonyTime += _simulationStep;
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
		_populationSize = runtimeState.populationSize;
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
		_populationSize = _initialPopulation;
		_gridWidth = fields->loadField("gridWidth", DEFAULT.gridWidth);
		_gridHeight = fields->loadField("gridHeight", DEFAULT.gridHeight);
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
	_populationSize = _initialPopulation;
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
	if (entity != nullptr) {
		_parentModel->removeEntity(entity);
	}
}
