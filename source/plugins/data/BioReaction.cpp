#include "BioReaction.h"

#include <functional>
#include <sstream>

#include "BioParameter.h"
#include "BioSpecies.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/ModelDataManager.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioReaction::GetPluginInformation;
}
#endif

namespace {

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
	auto* propReversible = new SimulationControlGeneric<bool>(
			std::bind(&BioReaction::isReversible, this), std::bind(&BioReaction::setReversible, this, std::placeholders::_1),
			Util::TypeOf<BioReaction>(), getName(), "Reversible", "");

	_parentModel->getControls()->insert(propRateConstant);
	_parentModel->getControls()->insert(propRateConstantParameterName);
	_parentModel->getControls()->insert(propReversible);

	_addProperty(propRateConstant);
	_addProperty(propRateConstantParameterName);
	_addProperty(propReversible);
}

PluginInformation* BioReaction::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioReaction>(), &BioReaction::LoadInstance, &BioReaction::NewInstance);
	info->setCategory("Biochemical simulation");
	info->setDescriptionHelp("Irreversible biochemical reaction with reactants, products, stoichiometry, and mass-action rate constant.");
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
			",rateConstant=" + Util::StrTruncIfInt(std::to_string(resolveRateConstant())) +
			",rateConstantParameterName=\"" + _rateConstantParameterName + "\"" +
			",reversible=" + std::to_string(_reversible ? 1 : 0);
}

bool BioReaction::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_reactants.clear();
		_products.clear();
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
		_rateConstant = fields->loadField("rateConstant", DEFAULT.rateConstant);
		_rateConstantParameterName = fields->loadField("rateConstantParameterName", DEFAULT.rateConstantParameterName);
		_reversible = fields->loadField("reversible", DEFAULT.reversible ? 1u : 0u) != 0u;
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
	fields->saveField("rateConstant", _rateConstant, DEFAULT.rateConstant, saveDefaultValues);
	fields->saveField("rateConstantParameterName", _rateConstantParameterName, DEFAULT.rateConstantParameterName, saveDefaultValues);
	fields->saveField("reversible", _reversible ? 1u : 0u, DEFAULT.reversible ? 1u : 0u, saveDefaultValues);
}

bool BioReaction::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "BioReaction must define a non-empty name. ";
		resultAll = false;
	}
	if (_reactants.empty()) {
		errorMessage += "BioReaction \"" + getName() + "\" must define at least one reactant. ";
		resultAll = false;
	}
	if (_products.empty()) {
		errorMessage += "BioReaction \"" + getName() + "\" must define at least one product. ";
		resultAll = false;
	}
	resultAll = checkTerms(_reactants, "reactant", errorMessage) && resultAll;
	resultAll = checkTerms(_products, "product", errorMessage) && resultAll;
	if (!_rateConstantParameterName.empty()) {
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
		errorMessage += "BioReaction \"" + getName() + "\" has reversible=true, but reversible mass-action pairs are not implemented yet. ";
		resultAll = false;
	}
	return resultAll;
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

void BioReaction::addReactant(std::string speciesName, double stoichiometry) {
	_reactants.push_back({speciesName, stoichiometry});
}

void BioReaction::addProduct(std::string speciesName, double stoichiometry) {
	_products.push_back({speciesName, stoichiometry});
}

void BioReaction::clearReactants() {
	_reactants.clear();
}

void BioReaction::clearProducts() {
	_products.clear();
}

const std::vector<BioReaction::StoichiometricTerm>& BioReaction::getReactants() const {
	return _reactants;
}

const std::vector<BioReaction::StoichiometricTerm>& BioReaction::getProducts() const {
	return _products;
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

void BioReaction::setReversible(bool reversible) {
	_reversible = reversible;
}

bool BioReaction::isReversible() const {
	return _reversible;
}
