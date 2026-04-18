#include "plugins/data/BiochemicalSimulation/BioParameter.h"

#include <functional>

#include "kernel/simulator/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioParameter::GetPluginInformation;
}
#endif

ModelDataDefinition* BioParameter::NewInstance(Model* model, std::string name) {
	return new BioParameter(model, name);
}

BioParameter::BioParameter(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<BioParameter>(), name) {
	auto* propValue = new SimulationControlDouble(
			std::bind(&BioParameter::getValue, this), std::bind(&BioParameter::setValue, this, std::placeholders::_1),
			Util::TypeOf<BioParameter>(), getName(), "Value", "");
	auto* propUnit = new SimulationControlGeneric<std::string>(
			std::bind(&BioParameter::getUnit, this), std::bind(&BioParameter::setUnit, this, std::placeholders::_1),
			Util::TypeOf<BioParameter>(), getName(), "Unit", "");

	_parentModel->getControls()->insert(propValue);
	_parentModel->getControls()->insert(propUnit);

	_addProperty(propValue);
	_addProperty(propUnit);
}

PluginInformation* BioParameter::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioParameter>(), &BioParameter::LoadInstance, &BioParameter::NewInstance);
	info->setCategory("BiochemicalSimulation");
	info->setDescriptionHelp("Scalar biochemical parameter, commonly used as a kinetic constant by BioReaction.");
	return info;
}

ModelDataDefinition* BioParameter::LoadInstance(Model* model, PersistenceRecord *fields) {
	BioParameter* newElement = new BioParameter(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string BioParameter::show() {
	return ModelDataDefinition::show() +
			",value=" + Util::StrTruncIfInt(std::to_string(_value)) +
			",unit=\"" + _unit + "\"";
}

bool BioParameter::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_value = fields->loadField("value", DEFAULT.value);
		_unit = fields->loadField("unit", DEFAULT.unit);
	}
	return res;
}

void BioParameter::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("value", _value, DEFAULT.value, saveDefaultValues);
	fields->saveField("unit", _unit, DEFAULT.unit, saveDefaultValues);
}

bool BioParameter::_check(std::string& errorMessage) {
	if (getName().empty()) {
		errorMessage += "BioParameter must define a non-empty name. ";
		return false;
	}
	return true;
}

void BioParameter::setValue(double value) {
	_value = value;
}

double BioParameter::getValue() const {
	return _value;
}

void BioParameter::setUnit(std::string unit) {
	_unit = unit;
}

std::string BioParameter::getUnit() const {
	return _unit;
}
