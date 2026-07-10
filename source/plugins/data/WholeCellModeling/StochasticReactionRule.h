#ifndef STOCHASTICREACTIONRULE_H
#define STOCHASTICREACTIONRULE_H

#include <string>
#include <vector>
#include <map>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

/**
 * Stochastic reaction rule for Gillespie SSA whole-cell simulations.
 *
 * Reactants and products are named by MolecularSpecies with integer
 * stoichiometry. The propensity is computed as k * combinatorial(counts),
 * where the combinatorial term is the product of C(x_i, n_i) for each
 * reactant species i with stoichiometry n_i.
 *
 * The rate constant k uses stochastic (microscopic) units: s^-1 for
 * unimolecular, molecules^-1 * s^-1 for bimolecular reactions.
 */
class StochasticReactionRule : public ModelDataDefinition {
public:
	struct StoichiometricTerm {
		std::string speciesName;
		int stoichiometry = 1;
	};

public:
	StochasticReactionRule(Model* model, std::string name = "");
	virtual ~StochasticReactionRule() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void addReactant(std::string speciesName, int stoichiometry = 1);
	void addProduct(std::string speciesName, int stoichiometry = 1);
	void clearReactants();
	void clearProducts();
	const std::vector<StoichiometricTerm>& getReactants() const;
	const std::vector<StoichiometricTerm>& getProducts() const;

	void setRateConstant(double rateConstant);
	double getRateConstant() const;
	void setCompartment(std::string compartment);
	std::string getCompartment() const;

	double computePropensity(const std::map<std::string, int>& counts) const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	static double _combinatorial(int n, int k);

private:
	const struct DEFAULT_VALUES {
		double rateConstant = 0.0;
		std::string compartment = "cytoplasm";
	} DEFAULT;

	std::vector<StoichiometricTerm> _reactants;
	std::vector<StoichiometricTerm> _products;
	double _rateConstant = DEFAULT.rateConstant;
	std::string _compartment = DEFAULT.compartment;
};

#endif /* STOCHASTICREACTIONRULE_H */
