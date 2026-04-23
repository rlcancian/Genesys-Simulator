#ifndef MASSACTIONODESYSTEM_H
#define MASSACTIONODESYSTEM_H

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "BioKineticLawExpression.h"
#include "OdeSystem_if.h"

/**
 * @brief ODE system generated from biochemical reactions using mass-action kinetics.
 */
class MassActionOdeSystem : public OdeSystem_if {
public:
	struct Species {
		std::string name;
		double initialAmount = 0.0;
		bool boundaryCondition = false;
		bool constant = false;
	};

	struct StoichiometricTerm {
		unsigned int speciesIndex = 0;
		double stoichiometry = 1.0;
	};

	struct Reaction {
		std::string name;
		double rateConstant = 0.0;
		std::vector<StoichiometricTerm> reactants;
		std::vector<StoichiometricTerm> products;
		std::string kineticLawExpression;
		std::vector<std::pair<std::string, double>> parameters;
		std::string reverseKineticLawExpression;
		double reverseRateConstant = 0.0;
		bool reversible = false;
	};

	MassActionOdeSystem() = default;

	MassActionOdeSystem(std::vector<Species> species, std::vector<Reaction> reactions)
			: _species(std::move(species)), _reactions(std::move(reactions)) {
	}

	unsigned int dimension() const override {
		return static_cast<unsigned int>(_species.size());
	}

	void evaluate(double, const double* y, double* dydt) const override {
		const unsigned int n = dimension();
		if (y == nullptr || dydt == nullptr) {
			return;
		}

		for (unsigned int i = 0; i < n; ++i) {
			dydt[i] = 0.0;
		}

		for (const Reaction& reaction : _reactions) {
			const double forwardRate = evaluateRate(reaction, reaction.kineticLawExpression, reaction.rateConstant, reaction.reactants, y);
			const double reverseRate = reaction.reversible ? evaluateRate(reaction, reaction.reverseKineticLawExpression, reaction.reverseRateConstant, reaction.products, y) : 0.0;

			for (const StoichiometricTerm& reactant : reaction.reactants) {
				applyDerivative(reactant.speciesIndex, reactant.stoichiometry * (reverseRate - forwardRate), dydt);
			}
			for (const StoichiometricTerm& product : reaction.products) {
				applyDerivative(product.speciesIndex, product.stoichiometry * (forwardRate - reverseRate), dydt);
			}
		}
	}

	const std::vector<Species>& species() const {
		return _species;
	}

	const std::vector<Reaction>& reactions() const {
		return _reactions;
	}

	double evaluateForwardRate(unsigned int reactionIndex, const double* y) const {
		if (reactionIndex >= _reactions.size() || y == nullptr) {
			return 0.0;
		}
		const Reaction& reaction = _reactions[reactionIndex];
		return evaluateRate(reaction, reaction.kineticLawExpression, reaction.rateConstant, reaction.reactants, y);
	}

	double evaluateReverseRate(unsigned int reactionIndex, const double* y) const {
		if (reactionIndex >= _reactions.size() || y == nullptr) {
			return 0.0;
		}
		const Reaction& reaction = _reactions[reactionIndex];
		if (!reaction.reversible) {
			return 0.0;
		}
		return evaluateRate(reaction, reaction.reverseKineticLawExpression, reaction.reverseRateConstant, reaction.products, y);
	}

	double evaluateNetRate(unsigned int reactionIndex, const double* y) const {
		return evaluateForwardRate(reactionIndex, y) - evaluateReverseRate(reactionIndex, y);
	}

private:
	double evaluateRate(const Reaction& reaction, const std::string& kineticLawExpression, double rateConstant, const std::vector<StoichiometricTerm>& massActionTerms, const double* y) const {
		if (kineticLawExpression.empty()) {
			return evaluateMassActionRate(rateConstant, massActionTerms, y);
		}

		BioKineticLawExpression expression;
		double rate = 0.0;
		std::string errorMessage;
		const bool ok = expression.evaluate(kineticLawExpression,
				[this, &reaction, y](const std::string& name, double& value) {
					for (unsigned int i = 0; i < _species.size(); ++i) {
						if (_species[i].name == name) {
							value = std::max(0.0, y[i]);
							return true;
						}
					}
					for (const auto& parameter : reaction.parameters) {
						if (parameter.first == name) {
							value = parameter.second;
							return true;
						}
					}
					return false;
				},
				rate, errorMessage);
		return ok ? std::max(0.0, rate) : 0.0;
	}

	double evaluateMassActionRate(double rateConstant, const std::vector<StoichiometricTerm>& terms, const double* y) const {
		double rate = rateConstant;
		for (const StoichiometricTerm& term : terms) {
			if (term.speciesIndex >= _species.size()) {
				return 0.0;
			}
			const double amount = std::max(0.0, y[term.speciesIndex]);
			rate *= std::pow(amount, term.stoichiometry);
		}
		return std::max(0.0, rate);
	}

	void applyDerivative(unsigned int speciesIndex, double value, double* dydt) const {
		if (speciesIndex >= _species.size() || _species[speciesIndex].boundaryCondition || _species[speciesIndex].constant) {
			return;
		}
		dydt[speciesIndex] += value;
	}

private:
	std::vector<Species> _species;
	std::vector<Reaction> _reactions;
};

#endif /* MASSACTIONODESYSTEM_H */
