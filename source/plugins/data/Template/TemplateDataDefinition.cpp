#include "plugins/data/Template/TemplateDataDefinition.h"

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &TemplateDataDefinition::GetPluginInformation;
}
#endif

/** Builds the example properties and registers their controls with the model. */
TemplateDataDefinition::TemplateDataDefinition(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<TemplateDataDefinition>(), name) {
	auto* propSomeString = new SimulationControlString(
		std::bind(&TemplateDataDefinition::getSomeString, this),
		std::bind(&TemplateDataDefinition::setSomeString, this, std::placeholders::_1),
		Util::TypeOf<TemplateDataDefinition>(), getName(), "SomeString");
	auto* propSomeUint = new SimulationControlDouble(
		[this]() { return static_cast<double>(getSomeUint()); },
		[this](double value) { setSomeUint(value < 0.0 ? 0u : static_cast<unsigned int>(value)); },
		Util::TypeOf<TemplateDataDefinition>(), getName(), "SomeUint");
	_parentModel->getControls()->insert(propSomeString);
	_parentModel->getControls()->insert(propSomeUint);
	_addSimulationControl(propSomeString);
	_addSimulationControl(propSomeUint);
}

void TemplateDataDefinition::setSomeString(const std::string& someString) { _someString = someString; }
std::string TemplateDataDefinition::getSomeString() const { return _someString; }
void TemplateDataDefinition::setSomeUint(unsigned int someUint) { _someUint = someUint; }
unsigned int TemplateDataDefinition::getSomeUint() const { return _someUint; }

std::string TemplateDataDefinition::show() {
	return ModelDataDefinition::show() + ", someString=\"" + _someString + "\", someUint=" + std::to_string(_someUint);
}

/** Returns plugin metadata plus factories used for loading and creating instances. */
PluginInformation* TemplateDataDefinition::GetPluginInformation() {
	auto* info = new PluginInformation(Util::TypeOf<TemplateDataDefinition>(),
	                                  &TemplateDataDefinition::LoadInstance,
	                                  &TemplateDataDefinition::NewInstance);
	info->setCategory("Template");
	info->setDescriptionHelp("Template for a model-wide data definition derived from ModelDataDefinition.");
	info->setObservation("Demonstrates controls, factories, metadata, persistence and validation; not a domain-specific data type.");
	return info;
}

/** LoadInstance is the persistence factory; NewInstance serves interactive/model creation. */
ModelDataDefinition* TemplateDataDefinition::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* element = new TemplateDataDefinition(model);
	try {
		element->_loadInstance(fields);
	} catch (const std::exception& e) {
		element->traceError("Failed to load TemplateDataDefinition instance: " + std::string(e.what()));
	}
	return element;
}

ModelDataDefinition* TemplateDataDefinition::NewInstance(Model* model, std::string name) {
	return new TemplateDataDefinition(model, name);
}

/** Restores base identity/common fields, then this example's custom fields. */
bool TemplateDataDefinition::_loadInstance(PersistenceRecord* fields) {
	const bool loaded = ModelDataDefinition::_loadInstance(fields);
	if (loaded) {
		try {
			_someString = fields->loadField("someString", DEFAULT.someString);
			_someUint = fields->loadField("someUint", DEFAULT.someUint);
		} catch (...) {
			// Preserve the base loader's tolerant behavior for absent optional fields.
		}
	}
	return loaded;
}

/** Persists common fields and custom fields, respecting save-defaults policy. */
void TemplateDataDefinition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("someUint", _someUint, DEFAULT.someUint);
	fields->saveField("someString", _someString, DEFAULT.someString);
}

/** Shows how a data definition can reject an invalid model configuration. */
bool TemplateDataDefinition::_check(std::string& errorMessage) {
	if (_someString.empty()) errorMessage += "SomeString must not be empty. ";
	if (_someUint == 0u) errorMessage += "SomeUint must be greater than zero. ";
	return errorMessage.empty();
}
