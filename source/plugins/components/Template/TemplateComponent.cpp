#include "plugins/components/Template/TemplateComponent.h"

#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"

#include <exception>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &TemplateComponent::GetPluginInformation;
}
#endif

namespace {
// Older persisted models used this suffix for the associated data definition.
constexpr const char* kLegacyDefinitionSuffix = ".DummyElement";
constexpr const char* kDefinitionSuffix = ".TemplateDataDefinition";
}

TemplateComponent::TemplateComponent(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<TemplateComponent>(), name) {
	auto* property = new SimulationControlGenericClass<TemplateDataDefinition*, Model*, TemplateDataDefinition>(
		_parentModel,
		std::bind(&TemplateComponent::getInternalDataDefinition, this),
		nullptr,
		Util::TypeOf<TemplateComponent>(), getName(), "TemplateDataDefinition",
		"Associated model-managed data definition used by this template component.",
		false, true, false, nullptr, false, true);
	_parentModel->getControls()->insert(property);
	_addSimulationControl(property);
}

TemplateDataDefinition* TemplateComponent::getInternalDataDefinition() const {
	return _internalDataDefinition;
}

std::string TemplateComponent::show() {
	return ModelComponent::show();
}

/** Metadata identifies this plugin; the factories create and restore instances. */
PluginInformation* TemplateComponent::GetPluginInformation() {
	auto* info = new PluginInformation(Util::TypeOf<TemplateComponent>(),
	                                  &TemplateComponent::LoadInstance,
	                                  &TemplateComponent::NewInstance);
	info->setCategory("Template");
	info->setDescriptionHelp("Template for a process-flow plugin derived from ModelComponent.");
	info->setObservation("Demonstrates component factories, persistence, entity dispatch and a model-managed associated definition.");
	return info;
}

ModelComponent* TemplateComponent::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* component = new TemplateComponent(model);
	try {
		component->_loadInstance(fields);
	} catch (const std::exception& e) {
		component->traceError("Failed to load TemplateComponent instance: " + std::string(e.what()));
	}
	return component;
}

ModelDataDefinition* TemplateComponent::NewInstance(Model* model, std::string name) {
	return new TemplateComponent(model, name);
}

/** Loads common fields, resolves the associated definition, and ensures registration. */
bool TemplateComponent::_loadInstance(PersistenceRecord* fields) {
	const bool loaded = ModelComponent::_loadInstance(fields);
	if (loaded) _createEditableDataDefinitions();
	return loaded;
}

/** Persists ModelComponent state; derived templates add their own fields here. */
void TemplateComponent::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	// Add fields introduced by a concrete component here, with matching load logic.
}

/** Demonstrates process dispatch by forwarding the same entity to the next component. */
void TemplateComponent::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	traceSimulation(this, "Template component forwards the arriving entity");
	_parentModel->sendEntityToComponent(entity, getConnectionManager()->getFrontConnection());
}

/** Finds a persisted associated definition or creates one under the new suffix. */
void TemplateComponent::_createEditableDataDefinitions() {
	if (_internalDataDefinition == nullptr) {
		auto* manager = _parentModel->getDataManager();
		_internalDataDefinition = dynamic_cast<TemplateDataDefinition*>(manager->getDataDefinition(
			Util::TypeOf<TemplateDataDefinition>(), getName() + kDefinitionSuffix));
		if (_internalDataDefinition == nullptr) {
			_internalDataDefinition = dynamic_cast<TemplateDataDefinition*>(manager->getDataDefinition(
				Util::TypeOf<TemplateDataDefinition>(), getName() + kLegacyDefinitionSuffix));
		}
		if (_internalDataDefinition == nullptr) {
			_internalDataDefinition = new TemplateDataDefinition(_parentModel, getName() + kDefinitionSuffix);
		}
	}
	_mandatoryNonEditableDataDefinitionInsert("TemplateDataDefinition", _internalDataDefinition);
}
