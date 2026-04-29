/*
 * File:   BioSteadyState.cpp
 * Author: GenESyS
 *
 * Component that evaluates BioNetwork steady-state status.
 */

#include "plugins/components/BiochemicalSimulation/BioSteadyState.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioSteadyState::GetPluginInformation;
}
#endif

ModelDataDefinition* BioSteadyState::NewInstance(Model* model, std::string name) {
	return new BioSteadyState(model, name);
}

BioSteadyState::BioSteadyState(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<BioSteadyState>(), name) {
	auto* propBioNetwork = new SimulationControlGenericClass<BioNetwork*, Model*, BioNetwork>(
			_parentModel,
			std::bind(&BioSteadyState::getBioNetwork, this),
			std::bind(&BioSteadyState::setBioNetwork, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "BioNetwork", "");
	auto* propTolerance = new SimulationControlDouble(
			std::bind(&BioSteadyState::getTolerance, this),
			std::bind(&BioSteadyState::setTolerance, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "Tolerance", "");
	auto* propRunSimulationBeforeCheck = new SimulationControlGeneric<bool>(
			std::bind(&BioSteadyState::getRunSimulationBeforeCheck, this),
			std::bind(&BioSteadyState::setRunSimulationBeforeCheck, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "RunSimulationBeforeCheck", "");
	auto* propLastSteady = new SimulationControlGeneric<bool>(
			std::bind(&BioSteadyState::getLastSteady, this),
			std::bind(&BioSteadyState::setLastSteady, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "LastSteady", "");
	auto* propLastMaxAbsoluteDerivative = new SimulationControlDouble(
			std::bind(&BioSteadyState::getLastMaxAbsoluteDerivative, this),
			std::bind(&BioSteadyState::setLastMaxAbsoluteDerivative, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "LastMaxAbsoluteDerivative", "");
	auto* propLastMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioSteadyState::getLastMessage, this),
			std::bind(&BioSteadyState::setLastMessage, this, std::placeholders::_1),
			Util::TypeOf<BioSteadyState>(), getName(), "LastMessage", "");

	_parentModel->getControls()->insert(propBioNetwork);
	_parentModel->getControls()->insert(propTolerance);
	_parentModel->getControls()->insert(propRunSimulationBeforeCheck);
	_parentModel->getControls()->insert(propLastSteady);
	_parentModel->getControls()->insert(propLastMaxAbsoluteDerivative);
	_parentModel->getControls()->insert(propLastMessage);

	_addSimulationControl(propBioNetwork);
	_addSimulationControl(propTolerance);
	_addSimulationControl(propRunSimulationBeforeCheck);
	_addSimulationControl(propLastSteady);
	_addSimulationControl(propLastMaxAbsoluteDerivative);
	_addSimulationControl(propLastMessage);
}

PluginInformation* BioSteadyState::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioSteadyState>(), &BioSteadyState::LoadInstance, &BioSteadyState::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("bionetwork.so");
	info->setDescriptionHelp("Checks BioNetwork steady-state status from the last available simulation result. "
	                         "The component can optionally run one simulation before the check and stores the "
	                         "last steady flag and derivative magnitude for downstream logic.");
	return info;
}

ModelComponent* BioSteadyState::LoadInstance(Model* model, PersistenceRecord* fields) {
	BioSteadyState* newComponent = new BioSteadyState(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newComponent;
}

std::string BioSteadyState::show() {
	return ModelComponent::show() +
	       ",bioNetwork=\"" + (_bioNetwork != nullptr ? _bioNetwork->getName() : std::string()) + "\"" +
	       ",tolerance=" + Util::StrTruncIfInt(std::to_string(_tolerance)) +
	       ",runSimulationBeforeCheck=" + std::to_string(_runSimulationBeforeCheck ? 1 : 0) +
	       ",lastSteady=" + std::to_string(_lastSteady ? 1 : 0) +
	       ",lastMaxAbsoluteDerivative=" + Util::StrTruncIfInt(std::to_string(_lastMaxAbsoluteDerivative));
}

bool BioSteadyState::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string bioNetworkName = fields->loadField("bioNetwork", DEFAULT.bioNetworkName);
		_bioNetwork = nullptr;
		if (!bioNetworkName.empty()) {
			ModelDataDefinition* definition = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioNetwork>(), bioNetworkName);
			_bioNetwork = dynamic_cast<BioNetwork*>(definition);
		}
		_tolerance = fields->loadField("tolerance", DEFAULT.tolerance);
		_runSimulationBeforeCheck = fields->loadField("runSimulationBeforeCheck", DEFAULT.runSimulationBeforeCheck ? 1u : 0u) != 0u;
		_lastSteady = fields->loadField("lastSteady", DEFAULT.lastSteady ? 1u : 0u) != 0u;
		_lastMaxAbsoluteDerivative = fields->loadField("lastMaxAbsoluteDerivative", DEFAULT.lastMaxAbsoluteDerivative);
		_lastMessage = fields->loadField("lastMessage", DEFAULT.lastMessage);
	}
	return res;
}

void BioSteadyState::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("bioNetwork", _bioNetwork != nullptr ? _bioNetwork->getName() : DEFAULT.bioNetworkName, DEFAULT.bioNetworkName, saveDefaultValues);
	fields->saveField("tolerance", _tolerance, DEFAULT.tolerance, saveDefaultValues);
	fields->saveField("runSimulationBeforeCheck", _runSimulationBeforeCheck ? 1u : 0u, DEFAULT.runSimulationBeforeCheck ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastSteady", _lastSteady ? 1u : 0u, DEFAULT.lastSteady ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastMaxAbsoluteDerivative", _lastMaxAbsoluteDerivative, DEFAULT.lastMaxAbsoluteDerivative, saveDefaultValues);
	fields->saveField("lastMessage", _lastMessage, DEFAULT.lastMessage, saveDefaultValues);
}

bool BioSteadyState::_check(std::string& errorMessage) {
	bool resultAll = true;
	_createInternalAndAttachedData();

	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<BioNetwork>(), _bioNetwork, "BioNetwork", errorMessage);
	if (_tolerance < 0.0) {
		errorMessage += "BioSteadyState \"" + getName() + "\" must define tolerance >= 0. ";
		resultAll = false;
	}
	return resultAll;
}

void BioSteadyState::_createInternalAndAttachedData() {
	if (_bioNetwork != nullptr) {
		_attachedDataInsert("BioNetwork", _bioNetwork);
	} else {
		_attachedDataRemove("BioNetwork");
	}
}

void BioSteadyState::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_bioNetwork == nullptr) {
		_lastSteady = false;
		_lastMaxAbsoluteDerivative = 0.0;
		_lastMessage = "BioSteadyState requires a referenced BioNetwork.";
		traceSimulation(this, TraceManager::Level::L1_errorFatal, _lastMessage);
		_forwardEntity(entity);
		return;
	}

	std::string errorMessage;
	if (_runSimulationBeforeCheck && !_bioNetwork->simulate(errorMessage)) {
		_lastSteady = false;
		_lastMaxAbsoluteDerivative = 0.0;
		_lastMessage = errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "BioSteadyState pre-simulation failed for BioNetwork \"" + _bioNetwork->getName() + "\": " + errorMessage);
		_forwardEntity(entity);
		return;
	}

	BioSteadyStateCheck check;
	if (!_bioNetwork->checkLastSampleSteadyState(_tolerance, &check, &errorMessage)) {
		_lastSteady = false;
		_lastMaxAbsoluteDerivative = 0.0;
		_lastMessage = errorMessage;
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
		                "BioSteadyState failed for BioNetwork \"" + _bioNetwork->getName() + "\": " + errorMessage);
		_forwardEntity(entity);
		return;
	}

	_lastSteady = check.steady;
	_lastMaxAbsoluteDerivative = check.maxAbsoluteDerivative;
	_lastMessage = "BioSteadyState evaluated BioNetwork \"" + _bioNetwork->getName() + "\".";
	traceSimulation(this, TraceManager::Level::L2_results,
	                "BioSteadyState evaluated BioNetwork \"" + _bioNetwork->getName() +
	                "\": steady=" + std::string(_lastSteady ? "true" : "false") +
	                ", maxAbsDerivative=" + Util::StrTruncIfInt(std::to_string(_lastMaxAbsoluteDerivative)));
	_forwardEntity(entity);
}

void BioSteadyState::_forwardEntity(Entity* entity) {
	if (entity == nullptr) {
		return;
	}
	Connection* frontConnection = this->getConnectionManager()->getFrontConnection();
	if (frontConnection == nullptr || frontConnection->component == nullptr) {
		traceSimulation(this, "BioSteadyState dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, frontConnection);
}

void BioSteadyState::setBioNetwork(BioNetwork* bioNetwork) {
	_bioNetwork = bioNetwork;
}

BioNetwork* BioSteadyState::getBioNetwork() const {
	return _bioNetwork;
}

void BioSteadyState::setTolerance(double tolerance) {
	_tolerance = tolerance;
}

double BioSteadyState::getTolerance() const {
	return _tolerance;
}

void BioSteadyState::setRunSimulationBeforeCheck(bool runSimulationBeforeCheck) {
	_runSimulationBeforeCheck = runSimulationBeforeCheck;
}

bool BioSteadyState::getRunSimulationBeforeCheck() const {
	return _runSimulationBeforeCheck;
}

void BioSteadyState::setLastSteady(bool lastSteady) {
	_lastSteady = lastSteady;
}

bool BioSteadyState::getLastSteady() const {
	return _lastSteady;
}

void BioSteadyState::setLastMaxAbsoluteDerivative(double lastMaxAbsoluteDerivative) {
	_lastMaxAbsoluteDerivative = lastMaxAbsoluteDerivative;
}

double BioSteadyState::getLastMaxAbsoluteDerivative() const {
	return _lastMaxAbsoluteDerivative;
}

void BioSteadyState::setLastMessage(std::string lastMessage) {
	_lastMessage = lastMessage;
}

std::string BioSteadyState::getLastMessage() const {
	return _lastMessage;
}

void BioSteadyState::_createReportStatisticsDataDefinitions() {
}

void BioSteadyState::_createEditableDataDefinitions() {
}

void BioSteadyState::_createOthersDataDefinitions() {
}
