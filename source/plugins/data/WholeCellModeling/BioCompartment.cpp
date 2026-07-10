#include "plugins/data/WholeCellModeling/BioCompartment.h"

#include <functional>
#include <utility>

#include "../../../kernel/simulator/model/Model.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioCompartment::GetPluginInformation;
}
#endif

ModelDataDefinition* BioCompartment::NewInstance(Model* model, std::string name) {
	return new BioCompartment(model, name);
}

BioCompartment::BioCompartment(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<BioCompartment>(), name) {
	auto* propType = new SimulationControlString(
			std::bind(&BioCompartment::getCompartmentType, this),
			std::bind(&BioCompartment::setCompartmentType, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "CompartmentType", "");
	auto* propParent = new SimulationControlString(
			std::bind(&BioCompartment::getParentCompartmentName, this),
			std::bind(&BioCompartment::setParentCompartmentName, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "ParentCompartmentName", "");
	auto* propMembraneBounded = new SimulationControlBool(
			std::bind(&BioCompartment::isMembraneBounded, this),
			std::bind(&BioCompartment::setMembraneBounded, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "MembraneBounded", "");
	auto* propVolumeFraction = new SimulationControlDouble(
			std::bind(&BioCompartment::getVolumeFraction, this),
			std::bind(&BioCompartment::setVolumeFraction, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "VolumeFraction", "");
	auto* propCopyNumber = new SimulationControlUInt(
			std::bind(&BioCompartment::getCopyNumber, this),
			std::bind(&BioCompartment::setCopyNumber, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "CopyNumber", "");
	auto* propRole = new SimulationControlString(
			std::bind(&BioCompartment::getRole, this),
			std::bind(&BioCompartment::setRole, this, std::placeholders::_1),
			Util::TypeOf<BioCompartment>(), getName(), "Role", "");

	_parentModel->getControls()->insert(propType);
	_parentModel->getControls()->insert(propParent);
	_parentModel->getControls()->insert(propMembraneBounded);
	_parentModel->getControls()->insert(propVolumeFraction);
	_parentModel->getControls()->insert(propCopyNumber);
	_parentModel->getControls()->insert(propRole);

	_addSimulationControl(propType);
	_addSimulationControl(propParent);
	_addSimulationControl(propMembraneBounded);
	_addSimulationControl(propVolumeFraction);
	_addSimulationControl(propCopyNumber);
	_addSimulationControl(propRole);
}

PluginInformation* BioCompartment::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioCompartment>(), &BioCompartment::LoadInstance, &BioCompartment::NewInstance);
	info->setCategory("Biologic/WholeCellModeling");
	info->setDescriptionHelp(
		"Defines a named biological compartment such as cytosol, nucleus, mitochondrion, bud, or extracellular medium for biochemical and whole-cell models.");
	return info;
}

ModelDataDefinition* BioCompartment::LoadInstance(Model* model, PersistenceRecord* fields) {
	BioCompartment* newElement = new BioCompartment(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load BioCompartment instance: " + std::string(e.what()));
	}
	return newElement;
}

std::string BioCompartment::show() {
	return ModelDataDefinition::show() +
			",compartmentType=\"" + _compartmentType + "\"" +
			",parentCompartmentName=\"" + _parentCompartmentName + "\"" +
			",membraneBounded=" + std::string(_membraneBounded ? "true" : "false") +
			",volumeFraction=" + std::to_string(_volumeFraction) +
			",copyNumber=" + std::to_string(_copyNumber) +
			",role=\"" + _role + "\"";
}

bool BioCompartment::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_compartmentType = fields->loadField("compartmentType", DEFAULT.compartmentType);
		_parentCompartmentName = fields->loadField("parentCompartmentName", DEFAULT.parentCompartmentName);
		_membraneBounded = fields->loadField("membraneBounded", DEFAULT.membraneBounded);
		_volumeFraction = fields->loadField("volumeFraction", DEFAULT.volumeFraction);
		_copyNumber = fields->loadField("copyNumber", DEFAULT.copyNumber);
		_role = fields->loadField("role", DEFAULT.role);
	}
	return res;
}

void BioCompartment::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("compartmentType", _compartmentType, DEFAULT.compartmentType, saveDefaultValues);
	fields->saveField("parentCompartmentName", _parentCompartmentName, DEFAULT.parentCompartmentName, saveDefaultValues);
	fields->saveField("membraneBounded", _membraneBounded, DEFAULT.membraneBounded, saveDefaultValues);
	fields->saveField("volumeFraction", _volumeFraction, DEFAULT.volumeFraction, saveDefaultValues);
	fields->saveField("copyNumber", _copyNumber, DEFAULT.copyNumber, saveDefaultValues);
	fields->saveField("role", _role, DEFAULT.role, saveDefaultValues);
}

bool BioCompartment::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "BioCompartment must define a non-empty name. ";
		resultAll = false;
	}
	if (_compartmentType.empty()) {
		errorMessage += "BioCompartment \"" + getName() + "\" compartmentType must be non-empty. ";
		resultAll = false;
	}
	if (_volumeFraction < 0.0) {
		errorMessage += "BioCompartment \"" + getName() + "\" volumeFraction must be >= 0. ";
		resultAll = false;
	}
	if (_copyNumber == 0u) {
		errorMessage += "BioCompartment \"" + getName() + "\" copyNumber must be >= 1. ";
		resultAll = false;
	}
	if (!_parentCompartmentName.empty()) {
		if (_parentCompartmentName == getName()) {
			errorMessage += "BioCompartment \"" + getName() + "\" cannot reference itself as parent. ";
			resultAll = false;
		} else {
			auto* parent = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioCompartment>(), _parentCompartmentName);
			if (parent == nullptr) {
				errorMessage += "BioCompartment \"" + getName() + "\" references missing parent compartment \"" + _parentCompartmentName + "\". ";
				resultAll = false;
			}
		}
	}
	_createEditableDataDefinitions();
	return resultAll;
}

void BioCompartment::_createEditableDataDefinitions() {
	if (_parentCompartmentName.empty()) {
		_optionalEditableDataDefinitionRemove("ParentCompartment");
		return;
	}
	auto* parent = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<BioCompartment>(), _parentCompartmentName);
	if (parent != nullptr) {
		_optionalEditableDataDefinitionInsert("ParentCompartment", parent);
	} else {
		_optionalEditableDataDefinitionRemove("ParentCompartment");
	}
}

void BioCompartment::setCompartmentType(std::string type) { _compartmentType = std::move(type); }
std::string BioCompartment::getCompartmentType() const { return _compartmentType; }
void BioCompartment::setParentCompartmentName(std::string parentName) { _parentCompartmentName = std::move(parentName); }
std::string BioCompartment::getParentCompartmentName() const { return _parentCompartmentName; }
void BioCompartment::setMembraneBounded(bool membraneBounded) { _membraneBounded = membraneBounded; }
bool BioCompartment::isMembraneBounded() const { return _membraneBounded; }
void BioCompartment::setVolumeFraction(double volumeFraction) { _volumeFraction = volumeFraction; }
double BioCompartment::getVolumeFraction() const { return _volumeFraction; }
void BioCompartment::setCopyNumber(unsigned int copyNumber) { _copyNumber = copyNumber; }
unsigned int BioCompartment::getCopyNumber() const { return _copyNumber; }
void BioCompartment::setRole(std::string role) { _role = std::move(role); }
std::string BioCompartment::getRole() const { return _role; }
