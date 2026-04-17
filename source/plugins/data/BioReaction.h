#ifndef BIOREACTION_H
#define BIOREACTION_H

#include <string>
#include <vector>

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/PluginInformation.h"

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
	void clearReactants();
	void clearProducts();
	const std::vector<StoichiometricTerm>& getReactants() const;
	const std::vector<StoichiometricTerm>& getProducts() const;
	void setRateConstant(double rateConstant);
	double getRateConstant() const;
	void setRateConstantParameterName(std::string rateConstantParameterName);
	std::string getRateConstantParameterName() const;
	double resolveRateConstant() const;
	void setReversible(bool reversible);
	bool isReversible() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;

private:
	bool checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const;

private:
	const struct DEFAULT_VALUES {
		double rateConstant = 0.0;
		std::string rateConstantParameterName = "";
		bool reversible = false;
	} DEFAULT;

	std::vector<StoichiometricTerm> _reactants;
	std::vector<StoichiometricTerm> _products;
	double _rateConstant = DEFAULT.rateConstant;
	std::string _rateConstantParameterName = DEFAULT.rateConstantParameterName;
	bool _reversible = DEFAULT.reversible;
};

#endif /* BIOREACTION_H */
