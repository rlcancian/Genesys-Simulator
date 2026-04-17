#ifndef MASSACTIONODESYSTEM_H
#define MASSACTIONODESYSTEM_H

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

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
			double rate = reaction.rateConstant;
			for (const StoichiometricTerm& reactant : reaction.reactants) {
				if (reactant.speciesIndex >= n) {
					rate = 0.0;
					break;
				}
				const double amount = std::max(0.0, y[reactant.speciesIndex]);
				rate *= std::pow(amount, reactant.stoichiometry);
			}

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
