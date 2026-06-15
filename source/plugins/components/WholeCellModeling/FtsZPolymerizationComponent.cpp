#include "plugins/components/WholeCellModeling/FtsZPolymerizationComponent.h"

#include <algorithm>
#include <cmath>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &FtsZPolymerizationComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* FtsZPolymerizationComponent::NewInstance(Model* model, std::string name) {
	return new FtsZPolymerizationComponent(model, name);
}

FtsZPolymerizationComponent::FtsZPolymerizationComponent(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<FtsZPolymerizationComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&FtsZPolymerizationComponent::getWholeCellState, this),
			std::bind(&FtsZPolymerizationComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "WholeCellState", "");
	auto* propActFwd = new SimulationControlDouble(
			std::bind(&FtsZPolymerizationComponent::getActivationFwd, this),
			std::bind(&FtsZPolymerizationComponent::setActivationFwd, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "ActivationFwd", "");
	auto* propActRev = new SimulationControlDouble(
			std::bind(&FtsZPolymerizationComponent::getActivationRev, this),
			std::bind(&FtsZPolymerizationComponent::setActivationRev, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "ActivationRev", "");
	auto* propVolumeNorm = new SimulationControlDouble(
			std::bind(&FtsZPolymerizationComponent::getVolumeNorm, this),
			std::bind(&FtsZPolymerizationComponent::setVolumeNorm, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "VolumeNorm", "");
	auto* propDeltaT = new SimulationControlDouble(
			std::bind(&FtsZPolymerizationComponent::getDeltaT, this),
			std::bind(&FtsZPolymerizationComponent::setDeltaT, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "DeltaT", "");
	auto* propRingKey = new SimulationControlString(
			std::bind(&FtsZPolymerizationComponent::getFtsZRingKey, this),
			std::bind(&FtsZPolymerizationComponent::setFtsZRingKey, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "FtsZRingKey", "");
	auto* propMonomerKey = new SimulationControlString(
			std::bind(&FtsZPolymerizationComponent::getFtsZMonomerKey, this),
			std::bind(&FtsZPolymerizationComponent::setFtsZMonomerKey, this, std::placeholders::_1),
			Util::TypeOf<FtsZPolymerizationComponent>(), getName(), "FtsZMonomerKey", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propActFwd);
	_parentModel->getControls()->insert(propActRev);
	_parentModel->getControls()->insert(propVolumeNorm);
	_parentModel->getControls()->insert(propDeltaT);
	_parentModel->getControls()->insert(propRingKey);
	_parentModel->getControls()->insert(propMonomerKey);

	_addSimulationControl(propState);
	_addSimulationControl(propActFwd);
	_addSimulationControl(propActRev);
	_addSimulationControl(propVolumeNorm);
	_addSimulationControl(propDeltaT);
	_addSimulationControl(propRingKey);
	_addSimulationControl(propMonomerKey);
}

PluginInformation* FtsZPolymerizationComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<FtsZPolymerizationComponent>(), &FtsZPolymerizationComponent::LoadInstance, &FtsZPolymerizationComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"FtsZ ring polymerization dynamics for whole-cell division timing. "
		"Advances ring completion each step via simplified activation-limited kinetics. "
		"Ring completion stored as per-mille integer (0-1000) in WholeCellState. "
		"Default parameters calibrated to give ~35% ring completion at ~12000 s (M. genitalium).");
	return info;
}

ModelComponent* FtsZPolymerizationComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	FtsZPolymerizationComponent* newComponent = new FtsZPolymerizationComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load FtsZPolymerizationComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string FtsZPolymerizationComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",activationFwd=" + std::to_string(_activationFwd) +
			",activationRev=" + std::to_string(_activationRev) +
			",volumeNorm=" + std::to_string(_volumeNorm) +
			",deltaT=" + std::to_string(_deltaT) +
			",ftsZRingKey=\"" + _ftsZRingKey + "\"" +
			",ftsZMonomerKey=\"" + _ftsZMonomerKey + "\"";
}

bool FtsZPolymerizationComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_activationFwd  = fields->loadField("activationFwd",  DEFAULT.activationFwd);
		_activationRev  = fields->loadField("activationRev",  DEFAULT.activationRev);
		_volumeNorm     = fields->loadField("volumeNorm",     DEFAULT.volumeNorm);
		_deltaT         = fields->loadField("deltaT",         DEFAULT.deltaT);
		_ftsZRingKey    = fields->loadField("ftsZRingKey",    DEFAULT.ftsZRingKey);
		_ftsZMonomerKey = fields->loadField("ftsZMonomerKey", DEFAULT.ftsZMonomerKey);
	}
	return res;
}

void FtsZPolymerizationComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("activationFwd",  _activationFwd,  DEFAULT.activationFwd,  saveDefaultValues);
	fields->saveField("activationRev",  _activationRev,  DEFAULT.activationRev,  saveDefaultValues);
	fields->saveField("volumeNorm",     _volumeNorm,     DEFAULT.volumeNorm,     saveDefaultValues);
	fields->saveField("deltaT",         _deltaT,         DEFAULT.deltaT,         saveDefaultValues);
	fields->saveField("ftsZRingKey",    _ftsZRingKey,    DEFAULT.ftsZRingKey,    saveDefaultValues);
	fields->saveField("ftsZMonomerKey", _ftsZMonomerKey, DEFAULT.ftsZMonomerKey, saveDefaultValues);
}

bool FtsZPolymerizationComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "FtsZPolymerizationComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	if (_volumeNorm <= 0.0) {
		errorMessage += "FtsZPolymerizationComponent \"" + getName() + "\" volumeNorm must be > 0. ";
		resultAll = false;
	}
	if (_deltaT <= 0.0) {
		errorMessage += "FtsZPolymerizationComponent \"" + getName() + "\" deltaT must be > 0. ";
		resultAll = false;
	}
	// Override activation rates from WholeCellState parameter cache if available.
	if (_wholeCellState != nullptr) {
		const double stateFwd = _wholeCellState->getMetaboliteAmount("param.ftsZActivationFwd");
		if (stateFwd > 0.0) {
			_activationFwd = stateFwd;
		}
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void FtsZPolymerizationComponent::_initBetweenReplications() {
	_nRef = 0;  // will be captured again from WholeCellState on first step
}

void FtsZPolymerizationComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"FtsZPolymerizationComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	const int nMonomer = _wholeCellState->getMoleculeCount(_ftsZMonomerKey);
	if (_nRef <= 0) {
		_nRef = (nMonomer > 0) ? nMonomer : 1;
	}

	const int ringPermille = _wholeCellState->getMoleculeCount(_ftsZRingKey);
	const double f = static_cast<double>(ringPermille) / 1000.0;

	const double phi = static_cast<double>(nMonomer) / static_cast<double>(_nRef);
	const double kPoly   = _activationFwd * _volumeNorm;
	const double kDepoly = _activationRev * _volumeNorm;

	const double df = (kPoly * phi * (1.0 - f) - kDepoly * f) * _deltaT;
	const double fNew = std::max(0.0, std::min(1.0, f + df));

	const int newPermille = static_cast<int>(std::round(fNew * 1000.0));
	_wholeCellState->setMoleculeCount(_ftsZRingKey, newPermille);

	_forwardEntity(entity);
}

void FtsZPolymerizationComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void FtsZPolymerizationComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "FtsZPolymerizationComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void FtsZPolymerizationComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* FtsZPolymerizationComponent::getWholeCellState() const      { return _wholeCellState; }
void FtsZPolymerizationComponent::setActivationFwd(double rate)              { _activationFwd = rate; }
double FtsZPolymerizationComponent::getActivationFwd() const                 { return _activationFwd; }
void FtsZPolymerizationComponent::setActivationRev(double rate)              { _activationRev = rate; }
double FtsZPolymerizationComponent::getActivationRev() const                 { return _activationRev; }
void FtsZPolymerizationComponent::setVolumeNorm(double norm)                 { _volumeNorm = norm; }
double FtsZPolymerizationComponent::getVolumeNorm() const                    { return _volumeNorm; }
void FtsZPolymerizationComponent::setDeltaT(double dt)                       { _deltaT = dt; }
double FtsZPolymerizationComponent::getDeltaT() const                        { return _deltaT; }
void FtsZPolymerizationComponent::setFtsZRingKey(std::string key)            { _ftsZRingKey = std::move(key); }
std::string FtsZPolymerizationComponent::getFtsZRingKey() const              { return _ftsZRingKey; }
void FtsZPolymerizationComponent::setFtsZMonomerKey(std::string key)         { _ftsZMonomerKey = std::move(key); }
std::string FtsZPolymerizationComponent::getFtsZMonomerKey() const           { return _ftsZMonomerKey; }
