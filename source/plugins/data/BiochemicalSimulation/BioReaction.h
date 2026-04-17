#ifndef BIOREACTION_H
#define BIOREACTION_H

#include <string>
#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Irreversible mass-action reaction definition.
 *
 * Reactants, products, and modifiers are stored by species name so the reaction
 * can be persisted independently from runtime pointers. Modifiers participate in
 * kinetic laws without being consumed or produced. The current execution contract
 * supports direct rate constants, a BioParameter reference, or an optional
 * kinetic-law expression that resolves BioSpecies/BioParameter names at
 * BioNetwork execution time. Reversible reactions are represented for future
 * compatibility but are rejected by validation and by BioNetwork execution until
 * reverse kinetics are defined.
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
	void setKineticLawExpression(std::string kineticLawExpression);
	std::string getKineticLawExpression() const;
	void setReversible(bool reversible);
	bool isReversible() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	bool checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const;
	bool checkModifiers(std::string& errorMessage) const;
	bool validateKineticLawExpression(std::string& errorMessage) const;
	bool resolveKineticLawSymbol(const std::string& symbolName, double& value) const;

private:
	const struct DEFAULT_VALUES {
		double rateConstant = 0.0;
		std::string rateConstantParameterName = "";
		std::string kineticLawExpression = "";
		bool reversible = false;
	} DEFAULT;

	std::vector<StoichiometricTerm> _reactants;
	std::vector<StoichiometricTerm> _products;
	std::vector<std::string> _modifiers;
	double _rateConstant = DEFAULT.rateConstant;
	std::string _rateConstantParameterName = DEFAULT.rateConstantParameterName;
	std::string _kineticLawExpression = DEFAULT.kineticLawExpression;
	bool _reversible = DEFAULT.reversible;
};

#endif /* BIOREACTION_H */
