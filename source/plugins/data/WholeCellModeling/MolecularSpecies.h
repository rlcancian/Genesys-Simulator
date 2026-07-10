#ifndef MOLECULARSPECIES_H
#define MOLECULARSPECIES_H

#include <string>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Discrete molecular species with integer copy count for Gillespie SSA.
 *
 * Unlike BioSpecies (continuous double amounts for ODE), MolecularSpecies
 * tracks integer molecule counts. Compartment and unit annotate the biological
 * context without affecting simulation logic.
 */
class MolecularSpecies : public ModelDataDefinition {
public:
	MolecularSpecies(Model* model, std::string name = "");
	virtual ~MolecularSpecies() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setInitialCount(int initialCount);
	int getInitialCount() const;
	void setCount(int count);
	int getCount() const;
	void setCompartment(std::string compartment);
	std::string getCompartment() const;
	void setUnit(std::string unit);
	std::string getUnit() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;

private:
	const struct DEFAULT_VALUES {
		int initialCount = 0;
		int count = 0;
		std::string compartment = "cytoplasm";
		std::string unit = "molecules";
	} DEFAULT;

	int _initialCount = DEFAULT.initialCount;
	int _count = DEFAULT.count;
	std::string _compartment = DEFAULT.compartment;
	std::string _unit = DEFAULT.unit;
};

#endif /* MOLECULARSPECIES_H */
