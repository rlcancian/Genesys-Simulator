#pragma once

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * @brief Didactic starting point for plugins derived from ModelDataDefinition.
 *
 * A data definition stores model-wide configuration or state that components
 * can reference. Unlike a ModelComponent, it does not receive entities or
 * participate in process-flow dispatch. This example demonstrates controls,
 * factories, metadata, persistence and model-check validation.
 *
 * The Model owns instances registered in its ModelDataManager. Simulation
 * controls are registered with the model and attached to this definition;
 * callers observing the definition must not delete it.
 */
class TemplateDataDefinition : public ModelDataDefinition {
public: /// constructors
	TemplateDataDefinition(Model* model, std::string name = "");
	~TemplateDataDefinition() override = default;

public: /// new public user methods for this component
	void setSomeString(const std::string& someString);
	std::string getSomeString() const;
	void setSomeUint(unsigned int someUint);
	unsigned int getSomeUint() const;

public: /// virtual public methods
	virtual std::string show() override;

	/** @brief Supplies the stable plugin ID and the load/create factories. */
	static PluginInformation* GetPluginInformation();
	/** @brief Constructs and loads one persisted data definition. */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	/** @brief Constructs and registers a fresh definition with the model. */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

	/** @brief Restores base fields and this example's configurable properties. */
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	/** @brief Persists base fields and this example's configurable properties. */
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	/** @brief Rejects empty SomeString and zero SomeUint configurations. */
	virtual bool _check(std::string& errorMessage) override;

private:
	/// Default values are also used by loading and save-defaults persistence.
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
	} DEFAULT;
	std::string _someString = DEFAULT.someString;
	unsigned int _someUint = DEFAULT.someUint;
};
