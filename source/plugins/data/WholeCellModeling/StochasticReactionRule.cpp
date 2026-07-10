#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"

#include <functional>
#include <sstream>

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &StochasticReactionRule::GetPluginInformation;
}
#endif

ModelDataDefinition* StochasticReactionRule::NewInstance(Model* model, std::string name) {
	return new StochasticReactionRule(model, name);
}

StochasticReactionRule::StochasticReactionRule(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<StochasticReactionRule>(), name) {
	auto* propRateConstant = new SimulationControlDouble(
			std::bind(&StochasticReactionRule::getRateConstant, this),
			std::bind(&StochasticReactionRule::setRateConstant, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionRule>(), getName(), "RateConstant", "");
	auto* propCompartment = new SimulationControlGeneric<std::string>(
			std::bind(&StochasticReactionRule::getCompartment, this),
			std::bind(&StochasticReactionRule::setCompartment, this, std::placeholders::_1),
			Util::TypeOf<StochasticReactionRule>(), getName(), "Compartment", "");

	_parentModel->getControls()->insert(propRateConstant);
	_parentModel->getControls()->insert(propCompartment);

	_addSimulationControl(propRateConstant);
	_addSimulationControl(propCompartment);
}

PluginInformation* StochasticReactionRule::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<StochasticReactionRule>(), &StochasticReactionRule::LoadInstance, &StochasticReactionRule::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setDescriptionHelp("Stochastic reaction rule with integer stoichiometry and microscopic rate constant for Gillespie SSA simulations.");
	return info;
}

ModelDataDefinition* StochasticReactionRule::LoadInstance(Model* model, PersistenceRecord* fields) {
	StochasticReactionRule* newElement = new StochasticReactionRule(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load StochasticReactionRule instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string StochasticReactionRule::show() {
	std::ostringstream reactants;
	reactants << "{";
	for (std::size_t i = 0; i < _reactants.size(); ++i) {
		if (i > 0) reactants << ",";
		reactants << _reactants[i].speciesName << ":" << _reactants[i].stoichiometry;
	}
	reactants << "}";

	std::ostringstream products;
	products << "{";
	for (std::size_t i = 0; i < _products.size(); ++i) {
		if (i > 0) products << ",";
		products << _products[i].speciesName << ":" << _products[i].stoichiometry;
	}
	products << "}";

	return ModelDataDefinition::show() +
			",reactants=" + reactants.str() +
			",products=" + products.str() +
			",rateConstant=" + std::to_string(_rateConstant) +
			",compartment=\"" + _compartment + "\"";
}

bool StochasticReactionRule::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_rateConstant = fields->loadField("rateConstant", DEFAULT.rateConstant);
		_compartment  = fields->loadField("compartment",  DEFAULT.compartment);

		_reactants.clear();
		const unsigned int reactantCount = fields->loadField("reactants", 0u);
		for (unsigned int i = 0; i < reactantCount; ++i) {
			StoichiometricTerm term;
			term.speciesName   = fields->loadField("reactantSpecies"       + Util::StrIndex(i), std::string(""));
			term.stoichiometry = static_cast<int>(fields->loadField("reactantStoichiometry" + Util::StrIndex(i), 1u));
			_reactants.push_back(term);
		}

		_products.clear();
		const unsigned int productCount = fields->loadField("products", 0u);
		for (unsigned int i = 0; i < productCount; ++i) {
			StoichiometricTerm term;
			term.speciesName   = fields->loadField("productSpecies"       + Util::StrIndex(i), std::string(""));
			term.stoichiometry = static_cast<int>(fields->loadField("productStoichiometry" + Util::StrIndex(i), 1u));
			_products.push_back(term);
		}
	}
	return res;
}

void StochasticReactionRule::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("rateConstant", _rateConstant, DEFAULT.rateConstant, saveDefaultValues);
	fields->saveField("compartment",  _compartment,  DEFAULT.compartment,  saveDefaultValues);

	fields->saveField("reactants", static_cast<unsigned int>(_reactants.size()), 0u, saveDefaultValues);
	for (std::size_t i = 0; i < _reactants.size(); ++i) {
		fields->saveField("reactantSpecies"       + Util::StrIndex(i), _reactants[i].speciesName,                  std::string(""), saveDefaultValues);
		fields->saveField("reactantStoichiometry" + Util::StrIndex(i), static_cast<unsigned int>(_reactants[i].stoichiometry), 1u, saveDefaultValues);
	}

	fields->saveField("products", static_cast<unsigned int>(_products.size()), 0u, saveDefaultValues);
	for (std::size_t i = 0; i < _products.size(); ++i) {
		fields->saveField("productSpecies"       + Util::StrIndex(i), _products[i].speciesName,                  std::string(""), saveDefaultValues);
		fields->saveField("productStoichiometry" + Util::StrIndex(i), static_cast<unsigned int>(_products[i].stoichiometry), 1u, saveDefaultValues);
	}
}

bool StochasticReactionRule::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "StochasticReactionRule must define a non-empty name. ";
		resultAll = false;
	}
	if (_rateConstant < 0.0) {
		errorMessage += "StochasticReactionRule \"" + getName() + "\" rateConstant must be >= 0. ";
		resultAll = false;
	}
	if (_reactants.empty() && _products.empty()) {
		errorMessage += "StochasticReactionRule \"" + getName() + "\" must have at least one reactant or product. ";
		resultAll = false;
	}
	for (const StoichiometricTerm& term : _reactants) {
		if (term.speciesName.empty()) {
			errorMessage += "StochasticReactionRule \"" + getName() + "\" has a reactant with an empty species name. ";
			resultAll = false;
		}
		if (term.stoichiometry <= 0) {
			errorMessage += "StochasticReactionRule \"" + getName() + "\" reactant stoichiometry must be > 0. ";
			resultAll = false;
		}
	}
	for (const StoichiometricTerm& term : _products) {
		if (term.speciesName.empty()) {
			errorMessage += "StochasticReactionRule \"" + getName() + "\" has a product with an empty species name. ";
			resultAll = false;
		}
		if (term.stoichiometry <= 0) {
			errorMessage += "StochasticReactionRule \"" + getName() + "\" product stoichiometry must be > 0. ";
			resultAll = false;
		}
	}
	return resultAll;
}

double StochasticReactionRule::_combinatorial(int n, int k) {
	if (k > n || n < 0 || k < 0) return 0.0;
	if (k == 0 || k == n) return 1.0;
	double result = 1.0;
	for (int i = 0; i < k; ++i) {
		result *= static_cast<double>(n - i);
		result /= static_cast<double>(i + 1);
	}
	return result;
}

double StochasticReactionRule::computePropensity(const std::map<std::string, int>& counts) const {
	if (_rateConstant <= 0.0) return 0.0;
	double propensity = _rateConstant;
	for (const StoichiometricTerm& reactant : _reactants) {
		auto it = counts.find(reactant.speciesName);
		const int count = (it != counts.end()) ? it->second : 0;
		if (count < reactant.stoichiometry) return 0.0;
		propensity *= _combinatorial(count, reactant.stoichiometry);
	}
	return propensity;
}

void StochasticReactionRule::addReactant(std::string speciesName, int stoichiometry) {
	_reactants.push_back({std::move(speciesName), stoichiometry});
}

void StochasticReactionRule::addProduct(std::string speciesName, int stoichiometry) {
	_products.push_back({std::move(speciesName), stoichiometry});
}

void StochasticReactionRule::clearReactants()                                    { _reactants.clear(); }
void StochasticReactionRule::clearProducts()                                     { _products.clear(); }
const std::vector<StochasticReactionRule::StoichiometricTerm>& StochasticReactionRule::getReactants() const { return _reactants; }
const std::vector<StochasticReactionRule::StoichiometricTerm>& StochasticReactionRule::getProducts() const  { return _products; }
void StochasticReactionRule::setRateConstant(double rateConstant)               { _rateConstant = rateConstant; }
double StochasticReactionRule::getRateConstant() const                          { return _rateConstant; }
void StochasticReactionRule::setCompartment(std::string compartment)            { _compartment = compartment; }
std::string StochasticReactionRule::getCompartment() const                      { return _compartment; }
