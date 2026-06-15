#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ResourceAllocationComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* ResourceAllocationComponent::NewInstance(Model* model, std::string name) {
	return new ResourceAllocationComponent(model, name);
}

ResourceAllocationComponent::ResourceAllocationComponent(Model* model, std::string name)
		: ModelComponent(model, Util::TypeOf<ResourceAllocationComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&ResourceAllocationComponent::getWholeCellState, this),
			std::bind(&ResourceAllocationComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<ResourceAllocationComponent>(), getName(), "WholeCellState", "");
	auto* propRnap = new SimulationControlString(
			std::bind(&ResourceAllocationComponent::getRnapCountKey, this),
			std::bind(&ResourceAllocationComponent::setRnapCountKey, this, std::placeholders::_1),
			Util::TypeOf<ResourceAllocationComponent>(), getName(), "RnapCountKey", "");
	auto* propRibo = new SimulationControlString(
			std::bind(&ResourceAllocationComponent::getRibosomeCountKey, this),
			std::bind(&ResourceAllocationComponent::setRibosomeCountKey, this, std::placeholders::_1),
			Util::TypeOf<ResourceAllocationComponent>(), getName(), "RibosomeCountKey", "");
	auto* propMrna = new SimulationControlString(
			std::bind(&ResourceAllocationComponent::getMRNASpeciesPrefix, this),
			std::bind(&ResourceAllocationComponent::setMRNASpeciesPrefix, this, std::placeholders::_1),
			Util::TypeOf<ResourceAllocationComponent>(), getName(), "MRNASpeciesPrefix", "");
	auto* propProt = new SimulationControlString(
			std::bind(&ResourceAllocationComponent::getProteinSpeciesPrefix, this),
			std::bind(&ResourceAllocationComponent::setProteinSpeciesPrefix, this, std::placeholders::_1),
			Util::TypeOf<ResourceAllocationComponent>(), getName(), "ProteinSpeciesPrefix", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propRnap);
	_parentModel->getControls()->insert(propRibo);
	_parentModel->getControls()->insert(propMrna);
	_parentModel->getControls()->insert(propProt);

	_addSimulationControl(propState);
	_addSimulationControl(propRnap);
	_addSimulationControl(propRibo);
	_addSimulationControl(propMrna);
	_addSimulationControl(propProt);
}

PluginInformation* ResourceAllocationComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ResourceAllocationComponent>(), &ResourceAllocationComponent::LoadInstance, &ResourceAllocationComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->setDescriptionHelp(
		"Fair allocation of RNAP and ribosomes across gene species before each SSA step. "
		"Reads RNAP_free and ribosome_free from WholeCellState molecule counts, "
		"divides them proportionally across mRNA gene species, and writes the budget "
		"to WholeCellState::setResourceBudget(). Must fire before StochasticTranscription.");
	return info;
}

ModelComponent* ResourceAllocationComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	ResourceAllocationComponent* newComponent = new ResourceAllocationComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load ResourceAllocationComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string ResourceAllocationComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",rnapCountKey=\"" + _rnapCountKey + "\"" +
			",ribosomeCountKey=\"" + _ribosomeCountKey + "\"" +
			",mRNASpeciesPrefix=\"" + _mRNASpeciesPrefix + "\"" +
			",proteinSpeciesPrefix=\"" + _proteinSpeciesPrefix + "\"";
}

bool ResourceAllocationComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_rnapCountKey        = fields->loadField("rnapCountKey",        DEFAULT.rnapCountKey);
		_ribosomeCountKey    = fields->loadField("ribosomeCountKey",    DEFAULT.ribosomeCountKey);
		_mRNASpeciesPrefix   = fields->loadField("mRNASpeciesPrefix",   DEFAULT.mRNASpeciesPrefix);
		_proteinSpeciesPrefix = fields->loadField("proteinSpeciesPrefix", DEFAULT.proteinSpeciesPrefix);
	}
	return res;
}

void ResourceAllocationComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState",      _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("rnapCountKey",        _rnapCountKey,        DEFAULT.rnapCountKey,        saveDefaultValues);
	fields->saveField("ribosomeCountKey",    _ribosomeCountKey,    DEFAULT.ribosomeCountKey,    saveDefaultValues);
	fields->saveField("mRNASpeciesPrefix",   _mRNASpeciesPrefix,   DEFAULT.mRNASpeciesPrefix,   saveDefaultValues);
	fields->saveField("proteinSpeciesPrefix", _proteinSpeciesPrefix, DEFAULT.proteinSpeciesPrefix, saveDefaultValues);
}

bool ResourceAllocationComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "ResourceAllocationComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void ResourceAllocationComponent::_initBetweenReplications() {
	// Resource budgets are cleared in WholeCellState::_initBetweenReplications.
}

void ResourceAllocationComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"ResourceAllocationComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	// Clear previous step's budget.
	_wholeCellState->clearResourceBudget();

	const int rnapFree     = _wholeCellState->getMoleculeCount(_rnapCountKey);
	const int ribosomeFree = _wholeCellState->getMoleculeCount(_ribosomeCountKey);

	// Count active gene species (keys starting with mRNASpeciesPrefix).
	int nGenes = 0;
	for (const auto& [name, count] : _wholeCellState->getMoleculeCounts()) {
		if (name.rfind(_mRNASpeciesPrefix, 0) == 0) {
			++nGenes;
		}
	}

	if (nGenes == 0) {
		_forwardEntity(entity);
		return;
	}

	const int rnapPerGene     = rnapFree     / nGenes;
	const int ribosomePerGene = ribosomeFree / nGenes;

	for (const auto& [name, count] : _wholeCellState->getMoleculeCounts()) {
		if (name.rfind(_mRNASpeciesPrefix, 0) == 0) {
			_wholeCellState->setResourceBudget(name, rnapPerGene);
			// Ribosome budget: keyed by matching protein species
			const std::string proteinName = _proteinSpeciesPrefix + name.substr(_mRNASpeciesPrefix.size());
			_wholeCellState->setResourceBudget(proteinName, ribosomePerGene);
		}
	}

	_forwardEntity(entity);
}

void ResourceAllocationComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

void ResourceAllocationComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getConnectionAtPort(0u);
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "ResourceAllocationComponent: no output connection, entity removed.");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void ResourceAllocationComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; }
WholeCellState* ResourceAllocationComponent::getWholeCellState() const      { return _wholeCellState; }
void ResourceAllocationComponent::setRnapCountKey(std::string key)          { _rnapCountKey = std::move(key); }
std::string ResourceAllocationComponent::getRnapCountKey() const             { return _rnapCountKey; }
void ResourceAllocationComponent::setRibosomeCountKey(std::string key)      { _ribosomeCountKey = std::move(key); }
std::string ResourceAllocationComponent::getRibosomeCountKey() const         { return _ribosomeCountKey; }
void ResourceAllocationComponent::setMRNASpeciesPrefix(std::string prefix)  { _mRNASpeciesPrefix = std::move(prefix); }
std::string ResourceAllocationComponent::getMRNASpeciesPrefix() const        { return _mRNASpeciesPrefix; }
void ResourceAllocationComponent::setProteinSpeciesPrefix(std::string prefix){ _proteinSpeciesPrefix = std::move(prefix); }
std::string ResourceAllocationComponent::getProteinSpeciesPrefix() const     { return _proteinSpeciesPrefix; }
