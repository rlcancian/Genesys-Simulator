#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"

#include <functional>

#include "kernel/simulator/Model.h"
#include "kernel/simulator/ModelDataManager.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MetabolicReaction::GetPluginInformation;
}
#endif

namespace {

std::string termsToString(const std::vector<MetabolicReaction::StoichiometricTerm>& terms) {
	std::string text = "{";
	for (const MetabolicReaction::StoichiometricTerm& term : terms) {
		text += term.speciesName + ":" + Util::StrTruncIfInt(std::to_string(term.stoichiometry)) + ",";
	}
	if (!terms.empty()) {
		text.pop_back();
	}
	text += "}";
	return text;
}

} // namespace

ModelDataDefinition* MetabolicReaction::NewInstance(Model* model, std::string name) {
	return new MetabolicReaction(model, name);
}

MetabolicReaction::MetabolicReaction(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<MetabolicReaction>(), name) {
	auto* propLowerBound = new SimulationControlDouble(
			std::bind(&MetabolicReaction::getLowerBound, this), std::bind(&MetabolicReaction::setLowerBound, this, std::placeholders::_1),
			Util::TypeOf<MetabolicReaction>(), getName(), "LowerBound", "");
	auto* propUpperBound = new SimulationControlDouble(
			std::bind(&MetabolicReaction::getUpperBound, this), std::bind(&MetabolicReaction::setUpperBound, this, std::placeholders::_1),
			Util::TypeOf<MetabolicReaction>(), getName(), "UpperBound", "");
	auto* propObjectiveCoefficient = new SimulationControlDouble(
			std::bind(&MetabolicReaction::getObjectiveCoefficient, this), std::bind(&MetabolicReaction::setObjectiveCoefficient, this, std::placeholders::_1),
			Util::TypeOf<MetabolicReaction>(), getName(), "ObjectiveCoefficient", "");
	auto* propReversible = new SimulationControlGeneric<bool>(
			std::bind(&MetabolicReaction::isReversible, this), std::bind(&MetabolicReaction::setReversible, this, std::placeholders::_1),
			Util::TypeOf<MetabolicReaction>(), getName(), "Reversible", "");
	auto* propGeneRule = new SimulationControlGeneric<std::string>(
			std::bind(&MetabolicReaction::getGeneRule, this), std::bind(&MetabolicReaction::setGeneRule, this, std::placeholders::_1),
			Util::TypeOf<MetabolicReaction>(), getName(), "GeneRule", "");

	_parentModel->getControls()->insert(propLowerBound);
	_parentModel->getControls()->insert(propUpperBound);
	_parentModel->getControls()->insert(propObjectiveCoefficient);
	_parentModel->getControls()->insert(propReversible);
	_parentModel->getControls()->insert(propGeneRule);

	_addSimulationControl(propLowerBound);
	_addSimulationControl(propUpperBound);
	_addSimulationControl(propObjectiveCoefficient);
	_addSimulationControl(propReversible);
	_addSimulationControl(propGeneRule);
}

PluginInformation* MetabolicReaction::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MetabolicReaction>(), &MetabolicReaction::LoadInstance, &MetabolicReaction::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Metabolic reaction for constraint-based models with stoichiometry, flux bounds, and optional gene rule.");
	return info;
}

ModelDataDefinition* MetabolicReaction::LoadInstance(Model* model, PersistenceRecord* fields) {
	MetabolicReaction* newElement = new MetabolicReaction(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string MetabolicReaction::show() {
	return ModelDataDefinition::show() +
	       ",reactants=" + termsToString(_reactants) +
	       ",products=" + termsToString(_products) +
	       ",lowerBound=" + Util::StrTruncIfInt(std::to_string(_lowerBound)) +
	       ",upperBound=" + Util::StrTruncIfInt(std::to_string(_upperBound)) +
	       ",objectiveCoefficient=" + Util::StrTruncIfInt(std::to_string(_objectiveCoefficient)) +
	       ",reversible=" + std::to_string(_reversible ? 1 : 0) +
	       ",geneRule=\"" + _geneRule + "\"";
}

bool MetabolicReaction::_loadInstance(PersistenceRecord* fields) {
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
		_lowerBound = fields->loadField("lowerBound", DEFAULT.lowerBound);
		_upperBound = fields->loadField("upperBound", DEFAULT.upperBound);
		_objectiveCoefficient = fields->loadField("objectiveCoefficient", DEFAULT.objectiveCoefficient);
		_reversible = fields->loadField("reversible", DEFAULT.reversible ? 1u : 0u) != 0u;
		_geneRule = fields->loadField("geneRule", DEFAULT.geneRule);
	}
	return res;
}

void MetabolicReaction::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
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
	fields->saveField("lowerBound", _lowerBound, DEFAULT.lowerBound, saveDefaultValues);
	fields->saveField("upperBound", _upperBound, DEFAULT.upperBound, saveDefaultValues);
	fields->saveField("objectiveCoefficient", _objectiveCoefficient, DEFAULT.objectiveCoefficient, saveDefaultValues);
	fields->saveField("reversible", _reversible ? 1u : 0u, DEFAULT.reversible ? 1u : 0u, saveDefaultValues);
	fields->saveField("geneRule", _geneRule, DEFAULT.geneRule, saveDefaultValues);
}

bool MetabolicReaction::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "MetabolicReaction must define a non-empty name. ";
		resultAll = false;
	}
	if (_reactants.empty() && _products.empty()) {
		errorMessage += "MetabolicReaction \"" + getName() + "\" must define at least one reactant or product. ";
		resultAll = false;
	}
	resultAll = _checkTerms(_reactants, "reactant", errorMessage) && resultAll;
	resultAll = _checkTerms(_products, "product", errorMessage) && resultAll;
	if (_upperBound < _lowerBound) {
		errorMessage += "MetabolicReaction \"" + getName() + "\" must define upperBound >= lowerBound. ";
		resultAll = false;
	}
	if (!_reversible && _lowerBound < 0.0) {
		errorMessage += "MetabolicReaction \"" + getName() + "\" must define lowerBound >= 0 when reversible=false. ";
		resultAll = false;
	}
	return resultAll;
}

bool MetabolicReaction::_checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const {
	bool resultAll = true;
	for (const StoichiometricTerm& term : terms) {
		if (term.speciesName.empty()) {
			errorMessage += "MetabolicReaction \"" + getName() + "\" has an empty " + side + " species name. ";
			resultAll = false;
		}
		if (term.stoichiometry <= 0.0) {
			errorMessage += "MetabolicReaction \"" + getName() + "\" has " + side + " \"" + term.speciesName + "\" with stoichiometry <= 0. ";
			resultAll = false;
		}
		auto* species = dynamic_cast<BioSpecies*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioSpecies>(), term.speciesName));
		if (species == nullptr) {
			errorMessage += "MetabolicReaction \"" + getName() + "\" references missing BioSpecies \"" + term.speciesName + "\". ";
			resultAll = false;
		}
	}
	return resultAll;
}

void MetabolicReaction::addReactant(std::string speciesName, double stoichiometry) {
	_reactants.push_back({speciesName, stoichiometry});
}

void MetabolicReaction::addProduct(std::string speciesName, double stoichiometry) {
	_products.push_back({speciesName, stoichiometry});
}

void MetabolicReaction::clearReactants() {
	_reactants.clear();
}

void MetabolicReaction::clearProducts() {
	_products.clear();
}

const std::vector<MetabolicReaction::StoichiometricTerm>& MetabolicReaction::getReactants() const {
	return _reactants;
}

const std::vector<MetabolicReaction::StoichiometricTerm>& MetabolicReaction::getProducts() const {
	return _products;
}

void MetabolicReaction::setLowerBound(double lowerBound) {
	_lowerBound = lowerBound;
}

double MetabolicReaction::getLowerBound() const {
	return _lowerBound;
}

void MetabolicReaction::setUpperBound(double upperBound) {
	_upperBound = upperBound;
}

double MetabolicReaction::getUpperBound() const {
	return _upperBound;
}

void MetabolicReaction::setObjectiveCoefficient(double objectiveCoefficient) {
	_objectiveCoefficient = objectiveCoefficient;
}

double MetabolicReaction::getObjectiveCoefficient() const {
	return _objectiveCoefficient;
}

void MetabolicReaction::setReversible(bool reversible) {
	_reversible = reversible;
}

bool MetabolicReaction::isReversible() const {
	return _reversible;
}

void MetabolicReaction::setGeneRule(std::string geneRule) {
	_geneRule = geneRule;
}

std::string MetabolicReaction::getGeneRule() const {
	return _geneRule;
}

void MetabolicReaction::_createReportStatisticsDataDefinitions() {
}

void MetabolicReaction::_createEditableDataDefinitions() {
}

void MetabolicReaction::_createOthersDataDefinitions() {
}
