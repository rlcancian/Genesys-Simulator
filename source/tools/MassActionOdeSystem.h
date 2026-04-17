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
			double rate = evaluateRate(reaction, y);

			for (const StoichiometricTerm& reactant : reaction.reactants) {
				applyDerivative(reactant.speciesIndex, -reactant.stoichiometry * rate, dydt);
			}
			for (const StoichiometricTerm& product : reaction.products) {
				applyDerivative(product.speciesIndex, product.stoichiometry * rate, dydt);
			}
		}
	}

	const std::vector<Species>& species() const {
		return _species;
	}

	const std::vector<Reaction>& reactions() const {
		return _reactions;
	}

private:
	double evaluateRate(const Reaction& reaction, const double* y) const {
		if (reaction.kineticLawExpression.empty()) {
			double rate = reaction.rateConstant;
			for (const StoichiometricTerm& reactant : reaction.reactants) {
				if (reactant.speciesIndex >= _species.size()) {
					return 0.0;
				}
				const double amount = std::max(0.0, y[reactant.speciesIndex]);
				rate *= std::pow(amount, reactant.stoichiometry);
			}
			return std::max(0.0, rate);
		}

		BioKineticLawExpression expression;
		double rate = 0.0;
		std::string errorMessage;
		const bool ok = expression.evaluate(reaction.kineticLawExpression,
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
