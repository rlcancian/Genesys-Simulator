#ifndef METABOLICREACTION_H
#define METABOLICREACTION_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Constraint-based metabolic reaction definition.
 */
class MetabolicReaction : public ModelDataDefinition {
public:
	struct StoichiometricTerm {
		std::string speciesName;
		double stoichiometry = 1.0;
	};

public:
	MetabolicReaction(Model* model, std::string name = "");
	virtual ~MetabolicReaction() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addReactant(std::string speciesName, double stoichiometry = 1.0);
	void addProduct(std::string speciesName, double stoichiometry = 1.0);
	void clearReactants();
	void clearProducts();
	const std::vector<StoichiometricTerm>& getReactants() const;
	const std::vector<StoichiometricTerm>& getProducts() const;
	void setLowerBound(double lowerBound);
	double getLowerBound() const;
	void setUpperBound(double upperBound);
	double getUpperBound() const;
	void setObjectiveCoefficient(double objectiveCoefficient);
	double getObjectiveCoefficient() const;
	void setReversible(bool reversible);
	bool isReversible() const;
	void setGeneRule(std::string geneRule);
	std::string getGeneRule() const;

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
	bool _checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		double lowerBound = 0.0;
		double upperBound = 1000.0;
		double objectiveCoefficient = 0.0;
		bool reversible = false;
		std::string geneRule = "";
	} DEFAULT;

	std::vector<StoichiometricTerm> _reactants;
	std::vector<StoichiometricTerm> _products;
	double _lowerBound = DEFAULT.lowerBound;
	double _upperBound = DEFAULT.upperBound;
	double _objectiveCoefficient = DEFAULT.objectiveCoefficient;
	bool _reversible = DEFAULT.reversible;
	std::string _geneRule = DEFAULT.geneRule;
};

#endif /* METABOLICREACTION_H */
