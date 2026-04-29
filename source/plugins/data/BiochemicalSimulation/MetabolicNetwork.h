#ifndef METABOLICNETWORK_H
#define METABOLICNETWORK_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Aggregate metabolic network definition.
 *
 * A MetabolicNetwork stores the reaction membership and objective metadata for
 * future flux-balance style components.
 */
class MetabolicNetwork : public ModelDataDefinition {
public:
	MetabolicNetwork(Model* model, std::string name = "");
	virtual ~MetabolicNetwork() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addReaction(std::string reactionName);
	void addExchangeSpecies(std::string speciesName);
	void clearReactions();
	void clearExchangeSpecies();
	const std::vector<std::string>& getReactionNames() const;
	const std::vector<std::string>& getExchangeSpeciesNames() const;
	void setObjectiveReactionName(std::string objectiveReactionName);
	std::string getObjectiveReactionName() const;
	void setObjectiveSense(std::string objectiveSense);
	std::string getObjectiveSense() const;
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
	bool _checkReactionNames(std::string& errorMessage) const;
	bool _checkExchangeSpeciesNames(std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		std::string objectiveReactionName = "";
		std::string objectiveSense = "Maximize";
		std::string compartment = "";
		bool enabled = true;
	} DEFAULT;

	std::vector<std::string> _reactionNames;
	std::vector<std::string> _exchangeSpeciesNames;
	std::string _objectiveReactionName = DEFAULT.objectiveReactionName;
	std::string _objectiveSense = DEFAULT.objectiveSense;
	std::string _compartment = DEFAULT.compartment;
	bool _enabled = DEFAULT.enabled;
};

#endif /* METABOLICNETWORK_H */
