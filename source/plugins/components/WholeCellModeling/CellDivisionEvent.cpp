#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CellDivisionEvent::GetPluginInformation;
}
#endif

ModelDataDefinition* CellDivisionEvent::NewInstance(Model* model, std::string name) {
	return new CellDivisionEvent(model, name);
}

CellDivisionEvent::CellDivisionEvent(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<CellDivisionEvent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&CellDivisionEvent::getWholeCellState, this),
			std::bind(&CellDivisionEvent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<CellDivisionEvent>(), getName(), "WholeCellState", "");
	auto* propDivMass = new SimulationControlDouble(
			std::bind(&CellDivisionEvent::getDivisionMassThreshold, this),
			std::bind(&CellDivisionEvent::setDivisionMassThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellDivisionEvent>(), getName(), "DivisionMassThreshold", "");
	auto* propFtsZ = new SimulationControlDouble(
			std::bind(&CellDivisionEvent::getFtsZThreshold, this),
			std::bind(&CellDivisionEvent::setFtsZThreshold, this, std::placeholders::_1),
			Util::TypeOf<CellDivisionEvent>(), getName(), "FtsZThreshold", "");
	auto* propFtsZKey = new SimulationControlString(
			std::bind(&CellDivisionEvent::getFtsZRingKey, this),
			std::bind(&CellDivisionEvent::setFtsZRingKey, this, std::placeholders::_1),
			Util::TypeOf<CellDivisionEvent>(), getName(), "FtsZRingKey", "");
	auto* propSeed = new SimulationControlUInt(
			std::bind(&CellDivisionEvent::getRandomSeed, this),
			std::bind(&CellDivisionEvent::setRandomSeed, this, std::placeholders::_1),
			Util::TypeOf<CellDivisionEvent>(), getName(), "RandomSeed", "");
	auto* propDivCount = new SimulationControlInt(
			std::bind(&CellDivisionEvent::getDivisionCount, this),
			nullptr,
			Util::TypeOf<CellDivisionEvent>(), getName(), "DivisionCount", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propDivMass);
	_parentModel->getControls()->insert(propFtsZ);
	_parentModel->getControls()->insert(propFtsZKey);
	_parentModel->getControls()->insert(propSeed);
	_parentModel->getControls()->insert(propDivCount);

	_addSimulationControl(propState);
	_addSimulationControl(propDivMass);
	_addSimulationControl(propFtsZ);
	_addSimulationControl(propFtsZKey);
	_addSimulationControl(propSeed);
	_addSimulationControl(propDivCount);
}

PluginInformation* CellDivisionEvent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CellDivisionEvent>(), &CellDivisionEvent::LoadInstance, &CellDivisionEvent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(2);
	info->setDescriptionHelp(
		"Discrete cell division event for whole-cell simulations. "
		"Triggers when cell mass >= divisionMassThreshold and (optionally) FtsZ ring completion >= ftsZThreshold. "
		"On division, molecule counts are binomially partitioned (p=0.5) and cell mass/volume are halved. "
		"Port 0: normal flow (no division). Port 1: fired after a division event (optional handler).");
	return info;
}

ModelComponent* CellDivisionEvent::LoadInstance(Model* model, PersistenceRecord* fields) {
	CellDivisionEvent* newComponent = new CellDivisionEvent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CellDivisionEvent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string CellDivisionEvent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",divisionMassThreshold=" + std::to_string(_divisionMassThreshold) +
			",ftsZThreshold=" + std::to_string(_ftsZThreshold) +
			",ftsZRingKey=\"" + _ftsZRingKey + "\"" +
			",randomSeed=" + std::to_string(_randomSeed) +
			",lastDivided=" + (_lastDivided ? "true" : "false") +
			",divisionCount=" + std::to_string(_divisionCount);
}

bool CellDivisionEvent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_divisionMassThreshold = fields->loadField("divisionMassThreshold", DEFAULT.divisionMassThreshold);
		_ftsZThreshold         = fields->loadField("ftsZThreshold",         DEFAULT.ftsZThreshold);
		_ftsZRingKey           = fields->loadField("ftsZRingKey",           DEFAULT.ftsZRingKey);
		_randomSeed            = fields->loadField("randomSeed",            DEFAULT.randomSeed);
		_rng.seed(_randomSeed);
	}
	return res;
}

void CellDivisionEvent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState",        _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("divisionMassThreshold", _divisionMassThreshold, DEFAULT.divisionMassThreshold, saveDefaultValues);
	fields->saveField("ftsZThreshold",         _ftsZThreshold,         DEFAULT.ftsZThreshold,         saveDefaultValues);
	fields->saveField("ftsZRingKey",           _ftsZRingKey,           DEFAULT.ftsZRingKey,            saveDefaultValues);
	fields->saveField("randomSeed",            _randomSeed,            DEFAULT.randomSeed,             saveDefaultValues);
}

bool CellDivisionEvent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_divisionMassThreshold < 0.0) {
		errorMessage += "CellDivisionEvent \"" + getName() + "\" divisionMassThreshold must be >= 0 (0 disables mass-based trigger). ";
		resultAll = false;
	}
	if (_wholeCellState == nullptr) {
		errorMessage += "CellDivisionEvent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void CellDivisionEvent::_initBetweenReplications() {
	_rng.seed(_randomSeed);
	_lastDivided   = false;
	_divisionCount = 0;
}

void CellDivisionEvent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	_lastDivided = false;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"CellDivisionEvent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity, 0u);
		return;
	}

	if (_checkDivisionCondition()) {
		_applyDivision();
		_lastDivided = true;
		++_divisionCount;

		{
			std::ostringstream _msg;
			_msg << std::scientific << std::setprecision(3);
			_msg << "CellDivisionEvent \"" << getName() << "\": division event #" << _divisionCount
			     << " — cell mass=" << _wholeCellState->getCellMass() << " g"
			     << " volume=" << _wholeCellState->getCellVolume() << " fL";
			traceSimulation(this, TraceManager::Level::L2_results, _msg.str());
		}

		// Port 1 is optional; fall back to port 0 if not connected
		Connection* divConn = this->getConnectionManager()->getConnectionAtPort(1u);
		if (divConn != nullptr && divConn->component != nullptr) {
			_forwardEntity(entity, 1u);
		} else {
			_forwardEntity(entity, 0u);
		}
	} else {
		_forwardEntity(entity, 0u);
	}
}

bool CellDivisionEvent::_checkDivisionCondition() const {
	const double mass = _wholeCellState->getCellMass();
	if (mass < _divisionMassThreshold) return false;

	if (_ftsZThreshold > 0.0) {
		const int ftsZRaw = _wholeCellState->getMoleculeCount(_ftsZRingKey);
		// stored as per-mille integer (0–1000); divide by 1000 for fraction
		const double ftsZCompletion = static_cast<double>(ftsZRaw) / 1000.0;
		if (ftsZCompletion < _ftsZThreshold) return false;
	}

	return true;
}

void CellDivisionEvent::_applyDivision() {
	std::binomial_distribution<int> coin(1, 0.5);

	// Partition integer molecule counts binomially
	const auto& counts = _wholeCellState->getMoleculeCounts();
	for (const auto& [species, count] : counts) {
		if (count <= 0) continue;
		int daughter1 = 0;
		for (int i = 0; i < count; ++i) {
			daughter1 += coin(_rng);
		}
		_wholeCellState->setMoleculeCount(species, daughter1);
	}

	// Cell geometry: this cell becomes daughter 1 (halved)
	_wholeCellState->setCellMass(_wholeCellState->getCellMass() * 0.5);
	_wholeCellState->setCellVolume(_wholeCellState->getCellVolume() * 0.5);

	// Lifecycle bookkeeping for the daughter cell that remains in this state object.
	_wholeCellState->setGenerationCount(_wholeCellState->getGenerationCount() + 1);
	_wholeCellState->setLastDivisionTime(_wholeCellState->getCurrentTime());
	_wholeCellState->setLifecyclePhase("post_division");
	_wholeCellState->setViable(true);

	// Reset step counter for new generation
	_wholeCellState->setStepCount(0);
}

void CellDivisionEvent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void CellDivisionEvent::_forwardEntity(Entity* entity, unsigned int port) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(port);
	if (conn == nullptr || conn->component == nullptr) {
		if (port == 0u) {
			traceSimulation(this, "CellDivisionEvent: invalid front connection, entity removed.");
			_parentModel->removeEntity(entity);
		}
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void CellDivisionEvent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* CellDivisionEvent::getWholeCellState() const      { return _wholeCellState; }
void CellDivisionEvent::setDivisionMassThreshold(double t)         { _divisionMassThreshold = t; }
double CellDivisionEvent::getDivisionMassThreshold() const         { return _divisionMassThreshold; }
void CellDivisionEvent::setFtsZThreshold(double t)                 { _ftsZThreshold = t; }
double CellDivisionEvent::getFtsZThreshold() const                 { return _ftsZThreshold; }
void CellDivisionEvent::setFtsZRingKey(std::string key)            { _ftsZRingKey = std::move(key); }
std::string CellDivisionEvent::getFtsZRingKey() const              { return _ftsZRingKey; }
void CellDivisionEvent::setRandomSeed(unsigned int seed)           { _randomSeed = seed; _rng.seed(seed); }
unsigned int CellDivisionEvent::getRandomSeed() const              { return _randomSeed; }
bool CellDivisionEvent::getLastDivided() const                     { return _lastDivided; }
int CellDivisionEvent::getDivisionCount() const                    { return _divisionCount; }
