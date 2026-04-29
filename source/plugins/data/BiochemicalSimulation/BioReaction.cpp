#include "plugins/data/BiochemicalSimulation/BioReaction.h"

#include <functional>
#include <sstream>

#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "tools/BioKineticLawExpression.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioReaction::GetPluginInformation;
}
#endif

namespace {

constexpr const char* kReactantAttachmentPrefix = "ReactantSpecies";
constexpr const char* kProductAttachmentPrefix = "ProductSpecies";
constexpr const char* kModifierAttachmentPrefix = "ModifierSpecies";

std::string termsToString(const std::vector<BioReaction::StoichiometricTerm>& terms) {
	std::string text = "{";
	for (const BioReaction::StoichiometricTerm& term : terms) {
		text += term.speciesName + ":" + Util::StrTruncIfInt(std::to_string(term.stoichiometry)) + ",";
	}
	if (!terms.empty()) {
		text.pop_back();
	}
	text += "}";
	return text;
}

std::string namesToString(const std::vector<std::string>& names) {
	std::string text = "{";
	for (const std::string& name : names) {
		text += name + ",";
	}
	if (!names.empty()) {
		text.pop_back();
	}
	text += "}";
	return text;
}

} // namespace

ModelDataDefinition* BioReaction::NewInstance(Model* model, std::string name) {
	return new BioReaction(model, name);
}

BioReaction::BioReaction(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<BioReaction>(), name) {
	auto* propRateConstant = new SimulationControlDouble(
			std::bind(&BioReaction::getRateConstant, this), std::bind(&BioReaction::setRateConstant, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "RateConstant", "");
	auto* propRateConstantParameterName = new SimulationControlGeneric<std::string>(
			std::bind(&BioReaction::getRateConstantParameterName, this), std::bind(&BioReaction::setRateConstantParameterName, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "RateConstantParameterName", "");
	auto* propReverseRateConstant = new SimulationControlDouble(
			std::bind(&BioReaction::getReverseRateConstant, this), std::bind(&BioReaction::setReverseRateConstant, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "ReverseRateConstant", "");
	auto* propReverseRateConstantParameterName = new SimulationControlGeneric<std::string>(
			std::bind(&BioReaction::getReverseRateConstantParameterName, this), std::bind(&BioReaction::setReverseRateConstantParameterName, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "ReverseRateConstantParameterName", "");
	auto* propKineticLawExpression = new SimulationControlGeneric<std::string>(
			std::bind(&BioReaction::getKineticLawExpression, this), std::bind(&BioReaction::setKineticLawExpression, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "KineticLawExpression", "");
	auto* propReverseKineticLawExpression = new SimulationControlGeneric<std::string>(
			std::bind(&BioReaction::getReverseKineticLawExpression, this), std::bind(&BioReaction::setReverseKineticLawExpression, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "ReverseKineticLawExpression", "");
	auto* propReversible = new SimulationControlGeneric<bool>(
			std::bind(&BioReaction::isReversible, this), std::bind(&BioReaction::setReversible, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "Reversible", "");

	_parentModel->getControls()->insert(propRateConstant);
	_parentModel->getControls()->insert(propRateConstantParameterName);
	_parentModel->getControls()->insert(propReverseRateConstant);
	_parentModel->getControls()->insert(propReverseRateConstantParameterName);
	_parentModel->getControls()->insert(propKineticLawExpression);
	_parentModel->getControls()->insert(propReverseKineticLawExpression);
	_parentModel->getControls()->insert(propReversible);

	_addSimulationControl(propRateConstant);
	_addSimulationControl(propRateConstantParameterName);
	_addSimulationControl(propReverseRateConstant);
	_addSimulationControl(propReverseRateConstantParameterName);
	_addSimulationControl(propKineticLawExpression);
	_addSimulationControl(propReverseKineticLawExpression);
	_addSimulationControl(propReversible);
}

PluginInformation* BioReaction::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioReaction>(), &BioReaction::LoadInstance, &BioReaction::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Biochemical reaction with reactants, products, modifiers, stoichiometry, forward/reverse mass-action rate constants, and optional kinetic-law expressions.");
	return info;
}

ModelDataDefinition* BioReaction::LoadInstance(Model* model, PersistenceRecord *fields) {
	BioReaction* newElement = new BioReaction(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string BioReaction::show() {
	return ModelDataDefinition::show() +
			",reactants=" + termsToString(_reactants) +
			",products=" + termsToString(_products) +
			",modifiers=" + namesToString(_modifiers) +
			",rateConstant=" + Util::StrTruncIfInt(std::to_string(resolveRateConstant())) +
			",rateConstantParameterName=\"" + _rateConstantParameterName + "\"" +
			",reverseRateConstant=" + Util::StrTruncIfInt(std::to_string(resolveReverseRateConstant())) +
			",reverseRateConstantParameterName=\"" + _reverseRateConstantParameterName + "\"" +
			",kineticLawExpression=\"" + _kineticLawExpression + "\"" +
			",reverseKineticLawExpression=\"" + _reverseKineticLawExpression + "\"" +
			",reversible=" + std::to_string(_reversible ? 1 : 0);
}

bool BioReaction::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_reactants.clear();
		_products.clear();
		_modifiers.clear();
		const unsigned int reactants = fields->loadField("reactants", 0u);
		for (unsigned int i = 0; i < reactants; ++i) {
			addReactant(fields->loadField("reactantSpecies" + Util::StrIndex(i), ""),
					fields->loadField("reactantStoichiometry" + Util::StrIndex(i), 1.0));
		}
		const unsigned int products = fields->loadField("products", 0u);
		for (unsigned int i = 0; i < products; ++i) {
			addProduct(fields->loadField("productSpecies" + Util::StrIndex(i), ""),
					fields->loadField("productStoichiometry" + Util::StrIndex(i), 1.0));
		}
		const unsigned int modifiers = fields->loadField("modifiers", 0u);
		for (unsigned int i = 0; i < modifiers; ++i) {
			addModifier(fields->loadField("modifierSpecies" + Util::StrIndex(i), ""));
		}
		_rateConstant = fields->loadField("rateConstant", DEFAULT.rateConstant);
		_rateConstantParameterName = fields->loadField("rateConstantParameterName", DEFAULT.rateConstantParameterName);
		_reverseRateConstant = fields->loadField("reverseRateConstant", DEFAULT.reverseRateConstant);
		_reverseRateConstantParameterName = fields->loadField("reverseRateConstantParameterName", DEFAULT.reverseRateConstantParameterName);
		_kineticLawExpression = fields->loadField("kineticLawExpression", DEFAULT.kineticLawExpression);
		_reverseKineticLawExpression = fields->loadField("reverseKineticLawExpression", DEFAULT.reverseKineticLawExpression);
		_reversible = fields->loadField("reversible", DEFAULT.reversible ? 1u : 0u) != 0u;
		// Keep participant attachments aligned with the persisted names so canvas links can be rebuilt later.
		syncParticipantAttachedSpecies();
	}
	return res;
}

void BioReaction::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("reactants", static_cast<unsigned int>(_reactants.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _reactants.size(); ++i) {
		fields->saveField("reactantSpecies" + Util::StrIndex(i), _reactants[i].speciesName, "", saveDefaultValues);
		fields->saveField("reactantStoichiometry" + Util::StrIndex(i), _reactants[i].stoichiometry, 1.0, saveDefaultValues);
	}
	fields->saveField("products", static_cast<unsigned int>(_products.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _products.size(); ++i) {
		fields->saveField("productSpecies" + Util::StrIndex(i), _products[i].speciesName, "", saveDefaultValues);
		fields->saveField("productStoichiometry" + Util::StrIndex(i), _products[i].stoichiometry, 1.0, saveDefaultValues);
	}
	fields->saveField("modifiers", static_cast<unsigned int>(_modifiers.size()), 0u, saveDefaultValues);
	for (unsigned int i = 0; i < _modifiers.size(); ++i) {
		fields->saveField("modifierSpecies" + Util::StrIndex(i), _modifiers[i], "", saveDefaultValues);
	}
	fields->saveField("rateConstant", _rateConstant, DEFAULT.rateConstant, saveDefaultValues);
	fields->saveField("rateConstantParameterName", _rateConstantParameterName, DEFAULT.rateConstantParameterName, saveDefaultValues);
	fields->saveField("reverseRateConstant", _reverseRateConstant, DEFAULT.reverseRateConstant, saveDefaultValues);
	fields->saveField("reverseRateConstantParameterName", _reverseRateConstantParameterName, DEFAULT.reverseRateConstantParameterName, saveDefaultValues);
	fields->saveField("kineticLawExpression", _kineticLawExpression, DEFAULT.kineticLawExpression, saveDefaultValues);
	fields->saveField("reverseKineticLawExpression", _reverseKineticLawExpression, DEFAULT.reverseKineticLawExpression, saveDefaultValues);
	fields->saveField("reversible", _reversible ? 1u : 0u, DEFAULT.reversible ? 1u : 0u, saveDefaultValues);
}

bool BioReaction::_check(std::string& errorMessage) {
	bool resultAll = true;
	// Refresh non-owning species attachments before validation so GUI link rebuilds use current participants.
	syncParticipantAttachedSpecies();
	if (getName().empty()) {
		errorMessage += "BioReaction must define a non-empty name. ";
		resultAll = false;
	}
	if (_reactants.empty() && _products.empty()) {
		errorMessage += "BioReaction \"" + getName() + "\" must define at least one reactant or product. ";
		resultAll = false;
	}
	resultAll = checkTerms(_reactants, "reactant", errorMessage) && resultAll;
	resultAll = checkTerms(_products, "product", errorMessage) && resultAll;
	resultAll = checkModifiers(errorMessage) && resultAll;
	if (!_kineticLawExpression.empty()) {
		resultAll = validateKineticLawExpression(errorMessage) && resultAll;
	} else if (!_rateConstantParameterName.empty()) {
		auto* parameter = dynamic_cast<BioParameter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), _rateConstantParameterName));
		if (parameter == nullptr) {
			errorMessage += "BioReaction \"" + getName() + "\" references missing BioParameter \"" + _rateConstantParameterName + "\". ";
			resultAll = false;
		}
	} else if (_rateConstant < 0.0) {
		errorMessage += "BioReaction \"" + getName() + "\" must define rateConstant >= 0. ";
		resultAll = false;
	}
	if (_reversible) {
		if (!_reverseKineticLawExpression.empty()) {
			resultAll = validateKineticLawExpression(_reverseKineticLawExpression, "reverseKineticLawExpression", errorMessage) && resultAll;
		} else if (!_reverseRateConstantParameterName.empty()) {
			auto* parameter = dynamic_cast<BioParameter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), _reverseRateConstantParameterName));
			if (parameter == nullptr) {
				errorMessage += "BioReaction \"" + getName() + "\" references missing reverse BioParameter \"" + _reverseRateConstantParameterName + "\". ";
				resultAll = false;
			}
		} else if (_reverseRateConstant < 0.0) {
			errorMessage += "BioReaction \"" + getName() + "\" must define reverseRateConstant >= 0 when reversible=true. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void BioReaction::_createInternalAndAttachedData() {
	// BioReaction does not own internal children yet, but it exposes participant links as attached species.
	syncParticipantAttachedSpecies();
}

bool BioReaction::validateKineticLawExpression(std::string& errorMessage) const {
	return validateKineticLawExpression(_kineticLawExpression, "kineticLawExpression", errorMessage);
}

bool BioReaction::validateKineticLawExpression(const std::string& expression, const std::string& label, std::string& errorMessage) const {
	double value = 0.0;
	std::string kineticLawError;
	std::string nonParticipantSpecies;
	BioKineticLawExpression evaluator;
	const bool ok = evaluator.evaluate(expression,
			[this, &nonParticipantSpecies](const std::string& symbolName, double& symbolValue) {
				auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), symbolName));
				if (species != nullptr && !hasParticipantSpecies(symbolName)) {
					nonParticipantSpecies = symbolName;
					return false;
				}
				return resolveKineticLawSymbol(symbolName, symbolValue);
			},
			value, kineticLawError);
	if (!ok) {
		if (!nonParticipantSpecies.empty()) {
			errorMessage += "BioReaction \"" + getName() + "\" " + label + " references BioSpecies \"" + nonParticipantSpecies + "\" that is not a reactant, product, or modifier. ";
			return false;
		}
		errorMessage += "BioReaction \"" + getName() + "\" has invalid " + label + " \"" + expression + "\": " + kineticLawError + " ";
		return false;
	}
	if (value < 0.0) {
		errorMessage += "BioReaction \"" + getName() + "\" " + label + " must evaluate to a non-negative rate. ";
		return false;
	}
	return true;
}

bool BioReaction::hasParticipantSpecies(const std::string& speciesName) const {
	for (const StoichiometricTerm& term : _reactants) {
		if (term.speciesName == speciesName) {
			return true;
		}
	}
	for (const StoichiometricTerm& term : _products) {
		if (term.speciesName == speciesName) {
			return true;
		}
	}
	for (const std::string& modifierName : _modifiers) {
		if (modifierName == speciesName) {
			return true;
		}
	}
	return false;
}

bool BioReaction::resolveKineticLawSymbol(const std::string& symbolName, double& value) const {
	auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), symbolName));
	if (species != nullptr) {
		value = species->getAmount();
		return true;
	}
	auto* parameter = dynamic_cast<BioParameter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), symbolName));
	if (parameter != nullptr) {
		value = parameter->getValue();
		return true;
	}
	return false;
}

bool BioReaction::checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const {
	bool resultAll = true;
	for (const StoichiometricTerm& term : terms) {
		if (term.speciesName.empty()) {
			errorMessage += "BioReaction \"" + getName() + "\" has an empty " + side + " species name. ";
			resultAll = false;
		}
		if (term.stoichiometry <= 0.0) {
			errorMessage += "BioReaction \"" + getName() + "\" has " + side + " \"" + term.speciesName + "\" with stoichiometry <= 0. ";
			resultAll = false;
		}
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), term.speciesName));
		if (species == nullptr) {
			errorMessage += "BioReaction \"" + getName() + "\" references missing BioSpecies \"" + term.speciesName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

bool BioReaction::checkModifiers(std::string& errorMessage) const {
	bool resultAll = true;
	for (const std::string& speciesName : _modifiers) {
		if (speciesName.empty()) {
			errorMessage += "BioReaction \"" + getName() + "\" has an empty modifier species name. ";
			resultAll = false;
			continue;
		}
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), speciesName));
		if (species == nullptr) {
			errorMessage += "BioReaction \"" + getName() + "\" references missing modifier BioSpecies \"" + speciesName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

void BioReaction::addReactant(std::string speciesName, double stoichiometry) {
	_reactants.push_back({speciesName, stoichiometry});
	syncParticipantAttachedSpecies();
}

void BioReaction::addProduct(std::string speciesName, double stoichiometry) {
	_products.push_back({speciesName, stoichiometry});
	syncParticipantAttachedSpecies();
}

void BioReaction::addModifier(std::string speciesName) {
	_modifiers.push_back(speciesName);
	syncParticipantAttachedSpecies();
}

void BioReaction::clearReactants() {
	_reactants.clear();
	syncParticipantAttachedSpecies();
}

void BioReaction::clearProducts() {
	_products.clear();
	syncParticipantAttachedSpecies();
}

void BioReaction::clearModifiers() {
	_modifiers.clear();
	syncParticipantAttachedSpecies();
}

const std::vector<BioReaction::StoichiometricTerm>& BioReaction::getReactants() const {
	return _reactants;
}

const std::vector<BioReaction::StoichiometricTerm>& BioReaction::getProducts() const {
	return _products;
}

const std::vector<std::string>& BioReaction::getModifiers() const {
	return _modifiers;
}

void BioReaction::setRateConstant(double rateConstant) {
	_rateConstant = rateConstant;
}

double BioReaction::getRateConstant() const {
	return _rateConstant;
}

void BioReaction::setRateConstantParameterName(std::string rateConstantParameterName) {
	_rateConstantParameterName = rateConstantParameterName;
}

std::string BioReaction::getRateConstantParameterName() const {
	return _rateConstantParameterName;
}

double BioReaction::resolveRateConstant() const {
	if (_rateConstantParameterName.empty()) {
		return _rateConstant;
	}
	auto* parameter = dynamic_cast<BioParameter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), _rateConstantParameterName));
	return parameter == nullptr ? _rateConstant : parameter->getValue();
}

void BioReaction::setReverseRateConstant(double reverseRateConstant) {
	_reverseRateConstant = reverseRateConstant;
}

double BioReaction::getReverseRateConstant() const {
	return _reverseRateConstant;
}

void BioReaction::setReverseRateConstantParameterName(std::string reverseRateConstantParameterName) {
	_reverseRateConstantParameterName = reverseRateConstantParameterName;
}

std::string BioReaction::getReverseRateConstantParameterName() const {
	return _reverseRateConstantParameterName;
}

double BioReaction::resolveReverseRateConstant() const {
	if (_reverseRateConstantParameterName.empty()) {
		return _reverseRateConstant;
	}
	auto* parameter = dynamic_cast<BioParameter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioParameter>(), _reverseRateConstantParameterName));
	return parameter == nullptr ? _reverseRateConstant : parameter->getValue();
}

void BioReaction::setKineticLawExpression(std::string kineticLawExpression) {
	_kineticLawExpression = kineticLawExpression;
}

std::string BioReaction::getKineticLawExpression() const {
	return _kineticLawExpression;
}

void BioReaction::setReverseKineticLawExpression(std::string reverseKineticLawExpression) {
	_reverseKineticLawExpression = reverseKineticLawExpression;
}

std::string BioReaction::getReverseKineticLawExpression() const {
	return _reverseKineticLawExpression;
}

void BioReaction::setReversible(bool reversible) {
	_reversible = reversible;
}

bool BioReaction::isReversible() const {
	return _reversible;
}

void BioReaction::syncParticipantAttachedSpecies() {
	auto removeAttachedDataWithPrefix = [this](const std::string& prefix) {
		std::vector<std::string> keysToRemove;
		for (const auto& attachedEntry : *getAttachedData()) {
			if (attachedEntry.first.rfind(prefix, 0) == 0) {
				keysToRemove.push_back(attachedEntry.first);
			}
		}
		for (const std::string& key : keysToRemove) {
			_attachedDataRemove(key);
		}
	};

	removeAttachedDataWithPrefix(kReactantAttachmentPrefix);
	removeAttachedDataWithPrefix(kProductAttachmentPrefix);
	removeAttachedDataWithPrefix(kModifierAttachmentPrefix);

	if (_parentModel == nullptr || _parentModel->getDataManager() == nullptr) {
		return;
	}

	auto attachSpeciesTerms = [this](const std::vector<StoichiometricTerm>& terms, const std::string& prefix) {
		for (unsigned int i = 0; i < terms.size(); ++i) {
			auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(
				Util::TypeOf<BioSpecies>(), terms[i].speciesName));
			// Only existing species become visual links; missing names remain validated by _check().
			_attachedDataInsert(prefix + Util::StrIndex(i), species);
		}
	};

	attachSpeciesTerms(_reactants, kReactantAttachmentPrefix);
	attachSpeciesTerms(_products, kProductAttachmentPrefix);
	for (unsigned int i = 0; i < _modifiers.size(); ++i) {
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(
			Util::TypeOf<BioSpecies>(), _modifiers[i]));
		// Modifier links are attached the same way as stoichiometric participants so the generic builder can render them.
		_attachedDataInsert(std::string(kModifierAttachmentPrefix) + Util::StrIndex(i), species);
	}
}

void BioReaction::_createReportStatisticsDataDefinitions() {
}

void BioReaction::_createEditableDataDefinitions() {
}

void BioReaction::_createOthersDataDefinitions() {
}
