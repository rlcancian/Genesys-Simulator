#include "plugins/components/WholeCellModeling/CompartmentExchangeComponent.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/ConnectionManager.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CompartmentExchangeComponent::GetPluginInformation;
}
#endif

ModelDataDefinition* CompartmentExchangeComponent::NewInstance(Model* model, std::string name) {
	return new CompartmentExchangeComponent(model, name);
}

CompartmentExchangeComponent::CompartmentExchangeComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<CompartmentExchangeComponent>(), name) {
	auto* propState = new SimulationControlGenericClass<WholeCellState*, Model*, WholeCellState>(
			_parentModel,
			std::bind(&CompartmentExchangeComponent::getWholeCellState, this),
			std::bind(&CompartmentExchangeComponent::setWholeCellState, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "WholeCellState", "");
	auto* propSourceRegion = new SimulationControlString(
			std::bind(&CompartmentExchangeComponent::getSourceRegion, this),
			std::bind(&CompartmentExchangeComponent::setSourceRegion, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "SourceRegion", "");
	auto* propSourceKey = new SimulationControlString(
			std::bind(&CompartmentExchangeComponent::getSourceMetaboliteKey, this),
			std::bind(&CompartmentExchangeComponent::setSourceMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "SourceMetaboliteKey", "");
	auto* propTargetRegion = new SimulationControlString(
			std::bind(&CompartmentExchangeComponent::getTargetRegion, this),
			std::bind(&CompartmentExchangeComponent::setTargetRegion, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "TargetRegion", "");
	auto* propTargetKey = new SimulationControlString(
			std::bind(&CompartmentExchangeComponent::getTargetMetaboliteKey, this),
			std::bind(&CompartmentExchangeComponent::setTargetMetaboliteKey, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "TargetMetaboliteKey", "");
	auto* propExchangeFraction = new SimulationControlDouble(
			std::bind(&CompartmentExchangeComponent::getExchangeFraction, this),
			std::bind(&CompartmentExchangeComponent::setExchangeFraction, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "ExchangeFraction", "");
	auto* propBaselineAmount = new SimulationControlDouble(
			std::bind(&CompartmentExchangeComponent::getBaselineAmount, this),
			std::bind(&CompartmentExchangeComponent::setBaselineAmount, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "BaselineAmount", "");
	auto* propDriverPathwayKey = new SimulationControlString(
			std::bind(&CompartmentExchangeComponent::getDriverPathwayKey, this),
			std::bind(&CompartmentExchangeComponent::setDriverPathwayKey, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "DriverPathwayKey", "");
	auto* propDriverScale = new SimulationControlDouble(
			std::bind(&CompartmentExchangeComponent::getDriverScale, this),
			std::bind(&CompartmentExchangeComponent::setDriverScale, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "DriverScale", "");
	auto* propMaxTransfer = new SimulationControlDouble(
			std::bind(&CompartmentExchangeComponent::getMaxTransferAmount, this),
			std::bind(&CompartmentExchangeComponent::setMaxTransferAmount, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "MaxTransferAmount", "");
	auto* propConserveMass = new SimulationControlBool(
			std::bind(&CompartmentExchangeComponent::getConserveMass, this),
			std::bind(&CompartmentExchangeComponent::setConserveMass, this, std::placeholders::_1),
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "ConserveMass", "");
	auto* propLastTransfer = new SimulationControlDouble(
			std::bind(&CompartmentExchangeComponent::getLastTransferAmount, this),
			nullptr,
			Util::TypeOf<CompartmentExchangeComponent>(), getName(), "LastTransferAmount", "");

	_parentModel->getControls()->insert(propState);
	_parentModel->getControls()->insert(propSourceRegion);
	_parentModel->getControls()->insert(propSourceKey);
	_parentModel->getControls()->insert(propTargetRegion);
	_parentModel->getControls()->insert(propTargetKey);
	_parentModel->getControls()->insert(propExchangeFraction);
	_parentModel->getControls()->insert(propBaselineAmount);
	_parentModel->getControls()->insert(propDriverPathwayKey);
	_parentModel->getControls()->insert(propDriverScale);
	_parentModel->getControls()->insert(propMaxTransfer);
	_parentModel->getControls()->insert(propConserveMass);
	_parentModel->getControls()->insert(propLastTransfer);

	_addSimulationControl(propState);
	_addSimulationControl(propSourceRegion);
	_addSimulationControl(propSourceKey);
	_addSimulationControl(propTargetRegion);
	_addSimulationControl(propTargetKey);
	_addSimulationControl(propExchangeFraction);
	_addSimulationControl(propBaselineAmount);
	_addSimulationControl(propDriverPathwayKey);
	_addSimulationControl(propDriverScale);
	_addSimulationControl(propMaxTransfer);
	_addSimulationControl(propConserveMass);
	_addSimulationControl(propLastTransfer);
}

PluginInformation* CompartmentExchangeComponent::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CompartmentExchangeComponent>(), &CompartmentExchangeComponent::LoadInstance, &CompartmentExchangeComponent::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setMinimumInputs(1);
	info->setMaximumInputs(1);
	info->setMinimumOutputs(1);
	info->setMaximumOutputs(1);
	info->insertDynamicLibFileDependence("wholecellstate.so");
	info->setDescriptionHelp(
		"Moves metabolite proxies between global and compartment-scoped WholeCellState pools, "
		"optionally driven by a pathway activity proxy.");
	return info;
}

ModelComponent* CompartmentExchangeComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	CompartmentExchangeComponent* newComponent = new CompartmentExchangeComponent(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load CompartmentExchangeComponent: " + std::string(e.what()));
	}
	return newComponent;
}

std::string CompartmentExchangeComponent::show() {
	return ModelComponent::show() +
			",wholeCellState=\"" + (_wholeCellState != nullptr ? _wholeCellState->getName() : std::string()) + "\"" +
			",sourceRegion=\"" + _sourceRegion + "\"" +
			",sourceMetaboliteKey=\"" + _sourceMetaboliteKey + "\"" +
			",targetRegion=\"" + _targetRegion + "\"" +
			",targetMetaboliteKey=\"" + _targetMetaboliteKey + "\"" +
			",exchangeFraction=" + std::to_string(_exchangeFraction) +
			",baselineAmount=" + std::to_string(_baselineAmount) +
			",driverPathwayKey=\"" + _driverPathwayKey + "\"" +
			",driverScale=" + std::to_string(_driverScale) +
			",maxTransferAmount=" + std::to_string(_maxTransferAmount) +
			",conserveMass=" + std::string(_conserveMass ? "true" : "false") +
			",exchangeRuleCount=" + std::to_string(_exchangeRules.size()) +
			",lastTransferAmount=" + std::to_string(_lastTransferAmount);
}

bool CompartmentExchangeComponent::_loadInstance(PersistenceRecord* fields) {
	const bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		const std::string stateName = fields->loadField("wholeCellState", DEFAULT.wholeCellStateName);
		_wholeCellState = nullptr;
		if (!stateName.empty()) {
			auto* def = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<WholeCellState>(), stateName);
			_wholeCellState = dynamic_cast<WholeCellState*>(def);
		}
		_sourceRegion = fields->loadField("sourceRegion", DEFAULT.sourceRegion);
		_sourceMetaboliteKey = fields->loadField("sourceMetaboliteKey", DEFAULT.sourceMetaboliteKey);
		_targetRegion = fields->loadField("targetRegion", DEFAULT.targetRegion);
		_targetMetaboliteKey = fields->loadField("targetMetaboliteKey", DEFAULT.targetMetaboliteKey);
		_exchangeFraction = fields->loadField("exchangeFraction", DEFAULT.exchangeFraction);
		_baselineAmount = fields->loadField("baselineAmount", DEFAULT.baselineAmount);
		_driverPathwayKey = fields->loadField("driverPathwayKey", DEFAULT.driverPathwayKey);
		_driverScale = fields->loadField("driverScale", DEFAULT.driverScale);
		_maxTransferAmount = fields->loadField("maxTransferAmount", DEFAULT.maxTransferAmount);
		_conserveMass = fields->loadField("conserveMass", DEFAULT.conserveMass);
		_lastTransferAmount = fields->loadField("lastTransferAmount", DEFAULT.lastTransferAmount);
		_exchangeRules.clear();
		const unsigned int exchangeRuleCount = fields->loadField("exchangeRuleCount", 0u);
		for (unsigned int i = 0; i < exchangeRuleCount; ++i) {
			ExchangeRule rule;
			rule.label = fields->loadField("exchangeRuleLabel" + Util::StrIndex(i), "");
			rule.sourceRegion = fields->loadField("exchangeRuleSourceRegion" + Util::StrIndex(i), "");
			rule.sourceMetaboliteKey = fields->loadField("exchangeRuleSourceMetaboliteKey" + Util::StrIndex(i), "");
			rule.targetRegion = fields->loadField("exchangeRuleTargetRegion" + Util::StrIndex(i), "");
			rule.targetMetaboliteKey = fields->loadField("exchangeRuleTargetMetaboliteKey" + Util::StrIndex(i), "");
			rule.exchangeFraction = fields->loadField("exchangeRuleFraction" + Util::StrIndex(i), 0.0);
			rule.baselineAmount = fields->loadField("exchangeRuleBaselineAmount" + Util::StrIndex(i), 0.0);
			rule.driverPathwayKey = fields->loadField("exchangeRuleDriverPathwayKey" + Util::StrIndex(i), "");
			rule.driverScale = fields->loadField("exchangeRuleDriverScale" + Util::StrIndex(i), 1.0);
			rule.maxTransferAmount = fields->loadField("exchangeRuleMaxTransferAmount" + Util::StrIndex(i), 0.0);
			rule.conserveMass = fields->loadField("exchangeRuleConserveMass" + Util::StrIndex(i), 1u) != 0u;
			_exchangeRules.push_back(std::move(rule));
		}
	}
	_createEditableDataDefinitions();
	return res;
}

void CompartmentExchangeComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("wholeCellState", _wholeCellState != nullptr ? _wholeCellState->getName() : DEFAULT.wholeCellStateName, DEFAULT.wholeCellStateName, saveDefaultValues);
	fields->saveField("sourceRegion", _sourceRegion, DEFAULT.sourceRegion, saveDefaultValues);
	fields->saveField("sourceMetaboliteKey", _sourceMetaboliteKey, DEFAULT.sourceMetaboliteKey, saveDefaultValues);
	fields->saveField("targetRegion", _targetRegion, DEFAULT.targetRegion, saveDefaultValues);
	fields->saveField("targetMetaboliteKey", _targetMetaboliteKey, DEFAULT.targetMetaboliteKey, saveDefaultValues);
	fields->saveField("exchangeFraction", _exchangeFraction, DEFAULT.exchangeFraction, saveDefaultValues);
	fields->saveField("baselineAmount", _baselineAmount, DEFAULT.baselineAmount, saveDefaultValues);
	fields->saveField("driverPathwayKey", _driverPathwayKey, DEFAULT.driverPathwayKey, saveDefaultValues);
	fields->saveField("driverScale", _driverScale, DEFAULT.driverScale, saveDefaultValues);
	fields->saveField("maxTransferAmount", _maxTransferAmount, DEFAULT.maxTransferAmount, saveDefaultValues);
	fields->saveField("conserveMass", _conserveMass, DEFAULT.conserveMass, saveDefaultValues);
	fields->saveField("lastTransferAmount", _lastTransferAmount, DEFAULT.lastTransferAmount, saveDefaultValues);
	fields->saveField("exchangeRuleCount", static_cast<unsigned int>(_exchangeRules.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _exchangeRules.size(); ++i) {
		const ExchangeRule& rule = _exchangeRules[i];
		fields->saveField("exchangeRuleLabel" + Util::StrIndex(i), rule.label, "", saveDefaultValues);
		fields->saveField("exchangeRuleSourceRegion" + Util::StrIndex(i), rule.sourceRegion, "", saveDefaultValues);
		fields->saveField("exchangeRuleSourceMetaboliteKey" + Util::StrIndex(i), rule.sourceMetaboliteKey, "", saveDefaultValues);
		fields->saveField("exchangeRuleTargetRegion" + Util::StrIndex(i), rule.targetRegion, "", saveDefaultValues);
		fields->saveField("exchangeRuleTargetMetaboliteKey" + Util::StrIndex(i), rule.targetMetaboliteKey, "", saveDefaultValues);
		fields->saveField("exchangeRuleFraction" + Util::StrIndex(i), rule.exchangeFraction, 0.0, saveDefaultValues);
		fields->saveField("exchangeRuleBaselineAmount" + Util::StrIndex(i), rule.baselineAmount, 0.0, saveDefaultValues);
		fields->saveField("exchangeRuleDriverPathwayKey" + Util::StrIndex(i), rule.driverPathwayKey, "", saveDefaultValues);
		fields->saveField("exchangeRuleDriverScale" + Util::StrIndex(i), rule.driverScale, 1.0, saveDefaultValues);
		fields->saveField("exchangeRuleMaxTransferAmount" + Util::StrIndex(i), rule.maxTransferAmount, 0.0, saveDefaultValues);
		fields->saveField("exchangeRuleConserveMass" + Util::StrIndex(i), rule.conserveMass ? 1u : 0u, 1u, saveDefaultValues);
	}
}

bool CompartmentExchangeComponent::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_wholeCellState == nullptr) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" requires a WholeCellState reference. ";
		resultAll = false;
	}
	const std::vector<ExchangeRule> rules = _getEffectiveRules();
	if (rules.empty()) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" requires at least one valid exchange rule. ";
		resultAll = false;
	}
	for (unsigned int i = 0; i < rules.size(); ++i) {
		resultAll = _checkRule(rules[i], errorMessage, i) && resultAll;
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void CompartmentExchangeComponent::_initBetweenReplications() {
	_lastTransferAmount = DEFAULT.lastTransferAmount;
}

void CompartmentExchangeComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	if (_wholeCellState == nullptr) {
		traceSimulation(this, TraceManager::Level::L1_errorFatal,
			"CompartmentExchangeComponent \"" + getName() + "\": WholeCellState not set.");
		_forwardEntity(entity);
		return;
	}

	_lastTransferAmount = 0.0;
	for (const ExchangeRule& rule : _getEffectiveRules()) {
		_applyRule(rule);
	}

	_forwardEntity(entity);
}

void CompartmentExchangeComponent::_createEditableDataDefinitions() {
	if (_wholeCellState != nullptr) {
		_optionalEditableDataDefinitionInsert("WholeCellState", _wholeCellState);
	} else {
		_optionalEditableDataDefinitionRemove("WholeCellState");
	}
}

double CompartmentExchangeComponent::_getPoolAmount(const std::string& region, const std::string& key) const {
	if (region.empty()) {
		return _wholeCellState->getMetaboliteAmount(key);
	}
	return _wholeCellState->getCompartmentMetaboliteAmount(region, key);
}

void CompartmentExchangeComponent::_setPoolAmount(const std::string& region, const std::string& key, double amount) {
	if (region.empty()) {
		_wholeCellState->setMetaboliteAmount(key, amount);
		return;
	}
	_wholeCellState->setCompartmentMetaboliteAmount(region, key, amount);
}

void CompartmentExchangeComponent::_forwardEntity(Entity* entity) {
	if (entity == nullptr) return;
	Connection* conn = this->getConnectionManager()->getFrontConnection();
	if (conn == nullptr || conn->component == nullptr) {
		traceSimulation(this, "CompartmentExchangeComponent dispatch skipped: invalid front connection");
		_parentModel->removeEntity(entity);
		return;
	}
	_parentModel->sendEntityToComponent(entity, conn);
}

void CompartmentExchangeComponent::setWholeCellState(WholeCellState* state) { _wholeCellState = state; _createEditableDataDefinitions(); }
WholeCellState* CompartmentExchangeComponent::getWholeCellState() const { return _wholeCellState; }
void CompartmentExchangeComponent::setSourceRegion(std::string region) { _sourceRegion = std::move(region); }
std::string CompartmentExchangeComponent::getSourceRegion() const { return _sourceRegion; }
void CompartmentExchangeComponent::setSourceMetaboliteKey(std::string key) { _sourceMetaboliteKey = std::move(key); }
std::string CompartmentExchangeComponent::getSourceMetaboliteKey() const { return _sourceMetaboliteKey; }
void CompartmentExchangeComponent::setTargetRegion(std::string region) { _targetRegion = std::move(region); }
std::string CompartmentExchangeComponent::getTargetRegion() const { return _targetRegion; }
void CompartmentExchangeComponent::setTargetMetaboliteKey(std::string key) { _targetMetaboliteKey = std::move(key); }
std::string CompartmentExchangeComponent::getTargetMetaboliteKey() const { return _targetMetaboliteKey; }
void CompartmentExchangeComponent::setExchangeFraction(double fraction) { _exchangeFraction = fraction; }
double CompartmentExchangeComponent::getExchangeFraction() const { return _exchangeFraction; }
void CompartmentExchangeComponent::setBaselineAmount(double amount) { _baselineAmount = amount; }
double CompartmentExchangeComponent::getBaselineAmount() const { return _baselineAmount; }
void CompartmentExchangeComponent::setDriverPathwayKey(std::string key) { _driverPathwayKey = std::move(key); }
std::string CompartmentExchangeComponent::getDriverPathwayKey() const { return _driverPathwayKey; }
void CompartmentExchangeComponent::setDriverScale(double scale) { _driverScale = scale; }
double CompartmentExchangeComponent::getDriverScale() const { return _driverScale; }
void CompartmentExchangeComponent::setMaxTransferAmount(double amount) { _maxTransferAmount = amount; }
double CompartmentExchangeComponent::getMaxTransferAmount() const { return _maxTransferAmount; }
void CompartmentExchangeComponent::setConserveMass(bool conserve) { _conserveMass = conserve; }
bool CompartmentExchangeComponent::getConserveMass() const { return _conserveMass; }
double CompartmentExchangeComponent::getLastTransferAmount() const { return _lastTransferAmount; }

void CompartmentExchangeComponent::clearExchangeRules() {
	_exchangeRules.clear();
}

void CompartmentExchangeComponent::addExchangeRule(const std::string& label,
                                                   const std::string& sourceRegion,
                                                   const std::string& sourceMetaboliteKey,
                                                   const std::string& targetRegion,
                                                   const std::string& targetMetaboliteKey,
                                                   double exchangeFraction,
                                                   double baselineAmount,
                                                   const std::string& driverPathwayKey,
                                                   double driverScale,
                                                   double maxTransferAmount,
                                                   bool conserveMass) {
	ExchangeRule rule;
	rule.label = label;
	rule.sourceRegion = sourceRegion;
	rule.sourceMetaboliteKey = sourceMetaboliteKey;
	rule.targetRegion = targetRegion;
	rule.targetMetaboliteKey = targetMetaboliteKey;
	rule.exchangeFraction = exchangeFraction;
	rule.baselineAmount = baselineAmount;
	rule.driverPathwayKey = driverPathwayKey;
	rule.driverScale = driverScale;
	rule.maxTransferAmount = maxTransferAmount;
	rule.conserveMass = conserveMass;
	_exchangeRules.push_back(std::move(rule));
}

unsigned int CompartmentExchangeComponent::getExchangeRuleCount() const {
	return static_cast<unsigned int>(_exchangeRules.size());
}

const std::vector<CompartmentExchangeComponent::ExchangeRule>& CompartmentExchangeComponent::getExchangeRules() const {
	return _exchangeRules;
}

CompartmentExchangeComponent::ExchangeRule CompartmentExchangeComponent::_buildLegacyRule() const {
	ExchangeRule rule;
	rule.label = "legacy";
	rule.sourceRegion = _sourceRegion;
	rule.sourceMetaboliteKey = _sourceMetaboliteKey;
	rule.targetRegion = _targetRegion;
	rule.targetMetaboliteKey = _targetMetaboliteKey;
	rule.exchangeFraction = _exchangeFraction;
	rule.baselineAmount = _baselineAmount;
	rule.driverPathwayKey = _driverPathwayKey;
	rule.driverScale = _driverScale;
	rule.maxTransferAmount = _maxTransferAmount;
	rule.conserveMass = _conserveMass;
	return rule;
}

std::vector<CompartmentExchangeComponent::ExchangeRule> CompartmentExchangeComponent::_getEffectiveRules() const {
	if (!_exchangeRules.empty()) {
		return _exchangeRules;
	}
	if (_sourceMetaboliteKey.empty() && _targetMetaboliteKey.empty()) {
		return {};
	}
	return {_buildLegacyRule()};
}

bool CompartmentExchangeComponent::_checkRule(const ExchangeRule& rule, std::string& errorMessage, unsigned int index) const {
	bool resultAll = true;
	const std::string label = !rule.label.empty() ? rule.label : ("rule#" + std::to_string(index));
	if (rule.sourceMetaboliteKey.empty()) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " sourceMetaboliteKey must be non-empty. ";
		resultAll = false;
	}
	if (rule.targetMetaboliteKey.empty()) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " targetMetaboliteKey must be non-empty. ";
		resultAll = false;
	}
	if (rule.exchangeFraction < 0.0) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " exchangeFraction must be >= 0. ";
		resultAll = false;
	}
	if (rule.baselineAmount < 0.0) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " baselineAmount must be >= 0. ";
		resultAll = false;
	}
	if (rule.driverScale < 0.0) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " driverScale must be >= 0. ";
		resultAll = false;
	}
	if (rule.maxTransferAmount < 0.0) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label + " maxTransferAmount must be >= 0. ";
		resultAll = false;
	}
	if (rule.conserveMass && rule.sourceRegion == rule.targetRegion &&
	    rule.sourceMetaboliteKey == rule.targetMetaboliteKey) {
		errorMessage += "CompartmentExchangeComponent \"" + getName() + "\" " + label +
		                " source and target cannot be identical when conserveMass=true. ";
		resultAll = false;
	}
	return resultAll;
}

double CompartmentExchangeComponent::_computeTransferAmount(const ExchangeRule& rule) const {
	const double sourceAmount = _getPoolAmount(rule.sourceRegion, rule.sourceMetaboliteKey);
	double transferAmount = sourceAmount * rule.exchangeFraction + rule.baselineAmount;
	if (!rule.driverPathwayKey.empty()) {
		const double driverValue = std::max(0.0, _wholeCellState->getPathwayActivity(rule.driverPathwayKey));
		transferAmount *= driverValue * rule.driverScale;
	}
	if (rule.maxTransferAmount > 0.0) {
		transferAmount = std::min(transferAmount, rule.maxTransferAmount);
	}
	if (!std::isfinite(transferAmount)) {
		return 0.0;
	}
	return std::max(0.0, std::min(transferAmount, sourceAmount));
}

void CompartmentExchangeComponent::_applyRule(const ExchangeRule& rule) {
	const double transferAmount = _computeTransferAmount(rule);
	const double sourceAmount = _getPoolAmount(rule.sourceRegion, rule.sourceMetaboliteKey);
	if (rule.conserveMass) {
		_setPoolAmount(rule.sourceRegion, rule.sourceMetaboliteKey, std::max(0.0, sourceAmount - transferAmount));
	}
	const double targetAmount = _getPoolAmount(rule.targetRegion, rule.targetMetaboliteKey);
	_setPoolAmount(rule.targetRegion, rule.targetMetaboliteKey, targetAmount + transferAmount);
	_lastTransferAmount += transferAmount;
}
