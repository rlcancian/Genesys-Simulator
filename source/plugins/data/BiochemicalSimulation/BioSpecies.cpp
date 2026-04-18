#include "plugins/data/BiochemicalSimulation/BioSpecies.h"

#include <functional>

#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioSpecies::GetPluginInformation;
}
#endif

ModelDataDefinition* BioSpecies::NewInstance(Model* model, std::string name) {
	return new BioSpecies(model, name);
}

BioSpecies::BioSpecies(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<BioSpecies>(), name) {
	auto* propInitialAmount = new SimulationControlDouble(
			std::bind(&BioSpecies::getInitialAmount, this), std::bind(&BioSpecies::setInitialAmount, this, std::placeholders::_1),
			Util::TypeOf<BioSpecies>(), getName(), "InitialAmount", "");
	auto* propAmount = new SimulationControlDouble(
			std::bind(&BioSpecies::getAmount, this), std::bind(&BioSpecies::setAmount, this, std::placeholders::_1),
			Util::TypeOf<BioSpecies>(), getName(), "Amount", "");
	auto* propConstant = new SimulationControlGeneric<bool>(
			std::bind(&BioSpecies::isConstant, this), std::bind(&BioSpecies::setConstant, this, std::placeholders::_1),
			Util::TypeOf<BioSpecies>(), getName(), "Constant", "");
	auto* propBoundaryCondition = new SimulationControlGeneric<bool>(
			std::bind(&BioSpecies::isBoundaryCondition, this), std::bind(&BioSpecies::setBoundaryCondition, this, std::placeholders::_1),
			Util::TypeOf<BioSpecies>(), getName(), "BoundaryCondition", "");
	auto* propUnit = new SimulationControlGeneric<std::string>(
			std::bind(&BioSpecies::getUnit, this), std::bind(&BioSpecies::setUnit, this, std::placeholders::_1),
			Util::TypeOf<BioSpecies>(), getName(), "Unit", "");

	_parentModel->getControls()->insert(propInitialAmount);
	_parentModel->getControls()->insert(propAmount);
	_parentModel->getControls()->insert(propConstant);
	_parentModel->getControls()->insert(propBoundaryCondition);
	_parentModel->getControls()->insert(propUnit);

	_addProperty(propInitialAmount);
	_addProperty(propAmount);
	_addProperty(propConstant);
	_addProperty(propBoundaryCondition);
	_addProperty(propUnit);
}

PluginInformation* BioSpecies::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioSpecies>(), &BioSpecies::LoadInstance, &BioSpecies::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Biochemical species with initial amount, current amount, units, and SBML-like constant/boundary flags.");
	return info;
}

ModelDataDefinition* BioSpecies::LoadInstance(Model* model, PersistenceRecord *fields) {
	BioSpecies* newElement = new BioSpecies(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string BioSpecies::show() {
	return ModelDataDefinition::show() +
			",initialAmount=" + Util::StrTruncIfInt(std::to_string(_initialAmount)) +
			",amount=" + Util::StrTruncIfInt(std::to_string(_amount)) +
			",constant=" + std::to_string(_constant ? 1 : 0) +
			",boundaryCondition=" + std::to_string(_boundaryCondition ? 1 : 0) +
			",unit=\"" + _unit + "\"";
}

bool BioSpecies::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_initialAmount = fields->loadField("initialAmount", DEFAULT.initialAmount);
		_amount = fields->loadField("amount", _initialAmount);
		_constant = fields->loadField("constant", DEFAULT.constant ? 1u : 0u) != 0u;
		_boundaryCondition = fields->loadField("boundaryCondition", DEFAULT.boundaryCondition ? 1u : 0u) != 0u;
		_unit = fields->loadField("unit", DEFAULT.unit);
	}
	return res;
}

void BioSpecies::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("initialAmount", _initialAmount, DEFAULT.initialAmount, saveDefaultValues);
	fields->saveField("amount", _amount, DEFAULT.amount, saveDefaultValues);
	fields->saveField("constant", _constant ? 1u : 0u, DEFAULT.constant ? 1u : 0u, saveDefaultValues);
	fields->saveField("boundaryCondition", _boundaryCondition ? 1u : 0u, DEFAULT.boundaryCondition ? 1u : 0u, saveDefaultValues);
	fields->saveField("unit", _unit, DEFAULT.unit, saveDefaultValues);
}

bool BioSpecies::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "BioSpecies must define a non-empty name. ";
		resultAll = false;
	}
	if (_initialAmount < 0.0) {
		errorMessage += "BioSpecies \"" + getName() + "\" must define initialAmount >= 0. ";
		resultAll = false;
	}
	if (_amount < 0.0) {
		errorMessage += "BioSpecies \"" + getName() + "\" must define amount >= 0. ";
		resultAll = false;
	}
	return resultAll;
}

void BioSpecies::_initBetweenReplications() {
	_amount = _initialAmount;
}

void BioSpecies::setInitialAmount(double initialAmount) {
	_initialAmount = initialAmount;
}

double BioSpecies::getInitialAmount() const {
	return _initialAmount;
}

void BioSpecies::setAmount(double amount) {
	_amount = amount;
}

double BioSpecies::getAmount() const {
	return _amount;
}

void BioSpecies::setConstant(bool constant) {
	_constant = constant;
}

bool BioSpecies::isConstant() const {
	return _constant;
}

void BioSpecies::setBoundaryCondition(bool boundaryCondition) {
	_boundaryCondition = boundaryCondition;
}

bool BioSpecies::isBoundaryCondition() const {
	return _boundaryCondition;
}

void BioSpecies::setUnit(std::string unit) {
	_unit = unit;
}

std::string BioSpecies::getUnit() const {
	return _unit;
}
