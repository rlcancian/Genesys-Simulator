#ifndef BIOREACTION_H
#define BIOREACTION_H

#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
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
	/*!
	 * \brief Creates an empty biochemical reaction definition.
	 * \param model Parent model.
	 * \param name Initial reaction name.
	 */
	BioReaction(Model* model, std::string name = "");
	/*!
	 * \brief Releases reaction-owned resources.
	 */
	virtual ~BioReaction() override = default;

public:
	/*!
	 * \brief Loads a biochemical reaction from serialized fields.
	 * \param model Parent model.
	 * \param fields Serialized fields.
	 * \return Newly created reaction instance.
	 */
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	/*!
	 * \brief Returns plugin metadata for this reaction type.
	 * \return Plugin information block.
	 */
	static PluginInformation* GetPluginInformation();
	/*!
	 * \brief Creates a new empty reaction instance.
	 * \param model Parent model.
	 * \param name Initial reaction name.
	 * \return Newly created reaction instance.
	 */
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	/*!
	 * \brief Adds a reactant species to the reaction.
	 * \param speciesName Reactant species name.
	 * \param stoichiometry Reactant stoichiometry.
	 */
	void addReactant(std::string speciesName, double stoichiometry = 1.0);
	/*!
	 * \brief Adds a product species to the reaction.
	 * \param speciesName Product species name.
	 * \param stoichiometry Product stoichiometry.
	 */
	void addProduct(std::string speciesName, double stoichiometry = 1.0);
	/*!
	 * \brief Adds a modifier species that influences the reaction rate.
	 * \param speciesName Modifier species name.
	 */
	void addModifier(std::string speciesName);
	/*!
	 * \brief Removes all reactants from the reaction.
	 */
	void clearReactants();
	/*!
	 * \brief Removes all products from the reaction.
	 */
	void clearProducts();
	/*!
	 * \brief Removes all modifiers from the reaction.
	 */
	void clearModifiers();
	/*!
	 * \brief Returns the registered reactants.
	 * \return Constant reference to the reactant terms.
	 */
	const std::vector<StoichiometricTerm>& getReactants() const;
	/*!
	 * \brief Returns the registered products.
	 * \return Constant reference to the product terms.
	 */
	const std::vector<StoichiometricTerm>& getProducts() const;
	/*!
	 * \brief Returns the registered modifiers.
	 * \return Constant reference to the modifier names.
	 */
	const std::vector<std::string>& getModifiers() const;
	/*!
	 * \brief Sets the forward rate constant.
	 * \param rateConstant Forward rate constant value.
	 */
	void setRateConstant(double rateConstant);
	/*!
	 * \brief Returns the forward rate constant.
	 * \return Forward rate constant.
	 */
	double getRateConstant() const;
	/*!
	 * \brief Stores the name of the parameter used for the forward rate constant.
	 * \param rateConstantParameterName Parameter name.
	 */
	void setRateConstantParameterName(std::string rateConstantParameterName);
	/*!
	 * \brief Returns the forward-rate parameter name.
	 * \return Parameter name used to resolve the forward rate constant.
	 */
	std::string getRateConstantParameterName() const;
	/*!
	 * \brief Resolves the effective forward rate constant.
	 * \return Forward rate constant after parameter resolution.
	 */
	double resolveRateConstant() const;
	/*!
	 * \brief Sets the reverse rate constant.
	 * \param reverseRateConstant Reverse rate constant value.
	 */
	void setReverseRateConstant(double reverseRateConstant);
	/*!
	 * \brief Returns the reverse rate constant.
	 * \return Reverse rate constant.
	 */
	double getReverseRateConstant() const;
	/*!
	 * \brief Stores the name of the parameter used for the reverse rate constant.
	 * \param reverseRateConstantParameterName Parameter name.
	 */
	void setReverseRateConstantParameterName(std::string reverseRateConstantParameterName);
	/*!
	 * \brief Returns the reverse-rate parameter name.
	 * \return Parameter name used to resolve the reverse rate constant.
	 */
	std::string getReverseRateConstantParameterName() const;
	/*!
	 * \brief Resolves the effective reverse rate constant.
	 * \return Reverse rate constant after parameter resolution.
	 */
	double resolveReverseRateConstant() const;
	/*!
	 * \brief Sets the forward kinetic-law expression.
	 * \param kineticLawExpression Expression text.
	 */
	void setKineticLawExpression(std::string kineticLawExpression);
	/*!
	 * \brief Returns the forward kinetic-law expression.
	 * \return Forward kinetic-law expression.
	 */
	std::string getKineticLawExpression() const;
	/*!
	 * \brief Sets the reverse kinetic-law expression.
	 * \param reverseKineticLawExpression Expression text.
	 */
	void setReverseKineticLawExpression(std::string reverseKineticLawExpression);
	/*!
	 * \brief Returns the reverse kinetic-law expression.
	 * \return Reverse kinetic-law expression.
	 */
	std::string getReverseKineticLawExpression() const;
	/*!
	 * \brief Marks the reaction as reversible or irreversible.
	 * \param reversible New reversible flag.
	 */
	void setReversible(bool reversible);
	/*!
	 * \brief Indicates whether the reaction is reversible.
	 * \return \c true when the reaction has a reverse branch.
	 */
	bool isReversible() const;

	/*!
	 * \brief Returns a textual representation of the reaction.
	 * \return Human-readable reaction summary.
	 */
	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private:
	bool checkTerms(const std::vector<StoichiometricTerm>& terms, const std::string& side, std::string& errorMessage) const;
	bool checkModifiers(std::string& errorMessage) const;
	bool validateKineticLawExpression(std::string& errorMessage) const;
	bool validateKineticLawExpression(const std::string& expression, const std::string& label, std::string& errorMessage) const;
	bool hasParticipantSpecies(const std::string& speciesName) const;
	bool resolveKineticLawSymbol(const std::string& symbolName, double& value) const;
	void syncParticipantAttachedSpecies();

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
