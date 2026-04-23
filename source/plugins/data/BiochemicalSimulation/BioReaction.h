#ifndef BIOREACTION_H
#define BIOREACTION_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Irreversible biochemical reaction definition.
 *
 * Reactants, products, and modifiers are stored by species name so the reaction
 * can be persisted independently from runtime pointers. Modifiers participate in
 * kinetic laws without being consumed or produced. Reactions may omit reactants
 * for synthesis or omit products for degradation, but must define at least one
 * reactant or product. The current execution contract supports direct rate
 * constants, a BioParameter reference, or an optional kinetic-law expression that
 * resolves BioSpecies/BioParameter names at BioNetwork execution time. Reversible
 * reactions use either a separate reverse kinetic-law expression or a reverse
 * mass-action rate constant over the product side of the reaction.
 */
class BioReaction : public ModelDataDefinition {
public:
	struct StoichiometricTerm {
		std::string speciesName;
		double stoichiometry = 1.0;
	};

public:
	BioReaction(Model* model, std::string name = "");
	virtual ~BioReaction() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addReactant(std::string speciesName, double stoichiometry = 1.0);
	void addProduct(std::string speciesName, double stoichiometry = 1.0);
	void addModifier(std::string speciesName);
	void clearReactants();
	void clearProducts();
	void clearModifiers();
	const std::vector<StoichiometricTerm>& getReactants() const;
	const std::vector<StoichiometricTerm>& getProducts() const;
	const std::vector<std::string>& getModifiers() const;
	void setRateConstant(double rateConstant);
	double getRateConstant() const;
	void setRateConstantParameterName(std::string rateConstantParameterName);
	std::string getRateConstantParameterName() const;
	double resolveRateConstant() const;
	void setReverseRateConstant(double reverseRateConstant);
	double getReverseRateConstant() const;
	void setReverseRateConstantParameterName(std::string reverseRateConstantParameterName);
	std::string getReverseRateConstantParameterName() const;
	double resolveReverseRateConstant() const;
	void setKineticLawExpression(std::string kineticLawExpression);
	std::string getKineticLawExpression() const;
	void setReverseKineticLawExpression(std::string reverseKineticLawExpression);
	std::string getReverseKineticLawExpression() const;
	void setReversible(bool reversible);
	bool isReversible() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	bool checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const;
	bool checkModifiers(std::string& errorMessage) const;
	bool validateKineticLawExpression(std::string& errorMessage) const;
	bool validateKineticLawExpression(const std::string& expression, const std::string& label, std::string& errorMessage) const;
	bool hasParticipantSpecies(const std::string& speciesName) const;
	bool resolveKineticLawSymbol(const std::string& symbolName, double& value) const;

private:
	const struct DEFAULT_VALUES {
		double rateConstant = 0.0;
		std::string rateConstantParameterName = "";
		double reverseRateConstant = 0.0;
		std::string reverseRateConstantParameterName = "";
		std::string kineticLawExpression = "";
		std::string reverseKineticLawExpression = "";
		bool reversible = false;
	} DEFAULT;

	std::vector<StoichiometricTerm> _reactants;
	std::vector<StoichiometricTerm> _products;
	std::vector<std::string> _modifiers;
	double _rateConstant = DEFAULT.rateConstant;
	std::string _rateConstantParameterName = DEFAULT.rateConstantParameterName;
	double _reverseRateConstant = DEFAULT.reverseRateConstant;
	std::string _reverseRateConstantParameterName = DEFAULT.reverseRateConstantParameterName;
	std::string _kineticLawExpression = DEFAULT.kineticLawExpression;
	std::string _reverseKineticLawExpression = DEFAULT.reverseKineticLawExpression;
	bool _reversible = DEFAULT.reversible;
};

#endif /* BIOREACTION_H */
