#ifndef GENETICCIRCUIT_H
#define GENETICCIRCUIT_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Aggregate genetic circuit definition.
 *
 * A GeneticCircuit stores an ordered sequence of genetic parts and a set of
 * regulatory relationships by name so it can be persisted independently from
 * runtime object pointers.
 */
class GeneticCircuit : public ModelDataDefinition {
public:
	GeneticCircuit(Model* model, std::string name = "");
	virtual ~GeneticCircuit() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addPart(std::string partName);
	void addRegulation(std::string regulationName);
	void clearParts();
	void clearRegulations();
	const std::vector<std::string>& getPartNames() const;
	const std::vector<std::string>& getRegulationNames() const;
	void setHostOrganism(std::string hostOrganism);
	std::string getHostOrganism() const;
	void setCompartment(std::string compartment);
	std::string getCompartment() const;
	void setEnabled(bool enabled);
	bool isEnabled() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	bool _checkPartNames(std::string& errorMessage) const;
	bool _checkRegulationNames(std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		std::string hostOrganism = "";
		std::string compartment = "";
		bool enabled = true;
	} DEFAULT;

	std::vector<std::string> _partNames;
	std::vector<std::string> _regulationNames;
	std::string _hostOrganism = DEFAULT.hostOrganism;
	std::string _compartment = DEFAULT.compartment;
	bool _enabled = DEFAULT.enabled;
};

#endif /* GENETICCIRCUIT_H */
