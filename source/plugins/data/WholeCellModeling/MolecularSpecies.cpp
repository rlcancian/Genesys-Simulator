#include "plugins/data/WholeCellModeling/MolecularSpecies.h"

#include <functional>

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MolecularSpecies::GetPluginInformation;
}
#endif

ModelDataDefinition* MolecularSpecies::NewInstance(Model* model, std::string name) {
	return new MolecularSpecies(model, name);
}

MolecularSpecies::MolecularSpecies(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<MolecularSpecies>(), name) {
	auto* propInitialCount = new SimulationControlInt(
			std::bind(&MolecularSpecies::getInitialCount, this),
			std::bind(&MolecularSpecies::setInitialCount, this, std::placeholders::_1),
			Util::TypeOf<MolecularSpecies>(), getName(), "InitialCount", "");
	auto* propCount = new SimulationControlInt(
			std::bind(&MolecularSpecies::getCount, this),
			std::bind(&MolecularSpecies::setCount, this, std::placeholders::_1),
			Util::TypeOf<MolecularSpecies>(), getName(), "Count", "");
	auto* propCompartment = new SimulationControlGeneric<std::string>(
			std::bind(&MolecularSpecies::getCompartment, this),
			std::bind(&MolecularSpecies::setCompartment, this, std::placeholders::_1),
			Util::TypeOf<MolecularSpecies>(), getName(), "Compartment", "");
	auto* propUnit = new SimulationControlGeneric<std::string>(
			std::bind(&MolecularSpecies::getUnit, this),
			std::bind(&MolecularSpecies::setUnit, this, std::placeholders::_1),
			Util::TypeOf<MolecularSpecies>(), getName(), "Unit", "");

	_parentModel->getControls()->insert(propInitialCount);
	_parentModel->getControls()->insert(propCount);
	_parentModel->getControls()->insert(propCompartment);
	_parentModel->getControls()->insert(propUnit);

	_addSimulationControl(propInitialCount);
	_addSimulationControl(propCount);
	_addSimulationControl(propCompartment);
	_addSimulationControl(propUnit);
}

PluginInformation* MolecularSpecies::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MolecularSpecies>(), &MolecularSpecies::LoadInstance, &MolecularSpecies::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setDescriptionHelp("Discrete molecular species with integer copy count for Gillespie SSA whole-cell simulations.");
	return info;
}

ModelDataDefinition* MolecularSpecies::LoadInstance(Model* model, PersistenceRecord* fields) {
	MolecularSpecies* newElement = new MolecularSpecies(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load MolecularSpecies instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string MolecularSpecies::show() {
	return ModelDataDefinition::show() +
			",initialCount=" + std::to_string(_initialCount) +
			",count=" + std::to_string(_count) +
			",compartment=\"" + _compartment + "\"" +
			",unit=\"" + _unit + "\"";
}

bool MolecularSpecies::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_initialCount = static_cast<int>(fields->loadField("initialCount", static_cast<unsigned int>(DEFAULT.initialCount)));
		_count        = static_cast<int>(fields->loadField("count",         static_cast<unsigned int>(_initialCount)));
		_compartment  = fields->loadField("compartment", DEFAULT.compartment);
		_unit         = fields->loadField("unit",        DEFAULT.unit);
	}
	return res;
}

void MolecularSpecies::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("initialCount", static_cast<unsigned int>(_initialCount), static_cast<unsigned int>(DEFAULT.initialCount), saveDefaultValues);
	fields->saveField("count",        static_cast<unsigned int>(_count),        static_cast<unsigned int>(DEFAULT.count),         saveDefaultValues);
	fields->saveField("compartment",  _compartment, DEFAULT.compartment, saveDefaultValues);
	fields->saveField("unit",         _unit,        DEFAULT.unit,        saveDefaultValues);
}

bool MolecularSpecies::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "MolecularSpecies must define a non-empty name. ";
		resultAll = false;
	}
	if (_initialCount < 0) {
		errorMessage += "MolecularSpecies \"" + getName() + "\" initialCount must be >= 0. ";
		resultAll = false;
	}
	if (_count < 0) {
		errorMessage += "MolecularSpecies \"" + getName() + "\" count must be >= 0. ";
		resultAll = false;
	}
	return resultAll;
}

void MolecularSpecies::_initBetweenReplications() {
	_count = _initialCount;
}

void MolecularSpecies::setInitialCount(int initialCount) { _initialCount = initialCount; }
int  MolecularSpecies::getInitialCount() const           { return _initialCount; }
void MolecularSpecies::setCount(int count)               { _count = count; }
int  MolecularSpecies::getCount() const                  { return _count; }
void MolecularSpecies::setCompartment(std::string compartment) { _compartment = compartment; }
std::string MolecularSpecies::getCompartment() const     { return _compartment; }
void MolecularSpecies::setUnit(std::string unit)         { _unit = unit; }
std::string MolecularSpecies::getUnit() const            { return _unit; }
