#pragma once

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "plugins/data/Template/TemplateDataDefinition.h"

/**
 * @brief Didactic starting point for plugins derived from ModelComponent.
 *
 * A ModelComponent participates in process flow: `_onDispatchEvent` receives
 * an Entity and implements the component's dispatch behavior. This example
 * demonstrates the required plugin metadata/factories, persistence hooks and
 * an associated model-managed TemplateDataDefinition. It intentionally keeps
 * its example dispatch behavior simple.
 *
 * The Model and its ModelDataManager own the component and associated data
 * definition. `_internalDataDefinition` is a non-owning observer; registering
 * the relationship with the component does not transfer ownership.
 */
class TemplateComponent : public ModelComponent {
public:
	TemplateComponent(Model* model, std::string name = "");
	~TemplateComponent() override = default;

	/** @brief Returns the associated definition managed by the parent Model. */
	TemplateDataDefinition* getInternalDataDefinition() const;
	std::string show() override;

	/** @brief Returns plugin ID, metadata and the load/create factory functions. */
	static PluginInformation* GetPluginInformation();
	/** @brief Reconstructs a component from its persistence record. */
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	/** @brief Creates a new component instance through the Plugin Manager. */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	/** @brief Loads common component data and reconnects/creates its associated definition. */
	bool _loadInstance(PersistenceRecord* fields) override;
	/** @brief Saves the common component record and any component-specific fields. */
	void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	/** @brief Implements this component's entity-arrival and forwarding behavior. */
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	/** @brief Ensures the associated definition exists before model checking/simulation. */
	void _createEditableDataDefinitions() override;

private:
	// Retained as a starting point for custom persistent component properties.
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
	} DEFAULT;
	std::string _someString = DEFAULT.someString;
	unsigned int _someUint = DEFAULT.someUint;

	// Non-owning reference; ModelDataManager controls the definition's lifetime.
	TemplateDataDefinition* _internalDataDefinition = nullptr;
};
