#ifndef BIOSIMULATIONANALYSIS_H
#define BIOSIMULATIONANALYSIS_H

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "tools/BioSimulationResult.h"
#include "tools/MassActionOdeSystem.h"

struct BioStoichiometryMatrix {
	std::vector<std::string> speciesNames;
	std::vector<std::string> reactionNames;
	std::vector<std::vector<double>> coefficients;

	double coefficient(unsigned int speciesIndex, unsigned int reactionIndex) const {
		if (speciesIndex >= coefficients.size() || reactionIndex >= reactionNames.size()) {
			return 0.0;
		}
		return coefficients[speciesIndex][reactionIndex];
	}
};

enum class BioReactionRateKind {
	Forward,
	Reverse,
	Net
};

struct BioReactionRateSample {
	double time = 0.0;
	std::vector<double> forwardRates;
	std::vector<double> reverseRates;
	std::vector<double> netRates;
};

struct BioReactionRateTimeCourse {
	std::vector<std::string> reactionNames;
	std::vector<BioReactionRateSample> samples;

	bool hasReaction(const std::string& reactionName) const {
		return std::find(reactionNames.begin(), reactionNames.end(), reactionName) != reactionNames.end();
	}

	bool toDataset(const std::string& reactionName, BioReactionRateKind kind, SimulationResultsDataset* dataset,
	               std::string* errorMessage = nullptr) const {
		if (dataset == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid dataset output parameter.";
			}
			return false;
		}

		const auto it = std::find(reactionNames.begin(), reactionNames.end(), reactionName);
		if (it == reactionNames.end()) {
			if (errorMessage != nullptr) {
				*errorMessage = "BioReactionRateTimeCourse does not contain reaction \"" + reactionName + "\".";
			}
			return false;
		}
		const unsigned int reactionIndex = static_cast<unsigned int>(it - reactionNames.begin());
		const std::string suffix = rateKindName(kind);

		SimulationResultsDataset converted;
		converted.sourceDescription = "BioReactionRateTimeCourse";
		converted.expression = reactionName + "." + suffix;
		converted.expressionName = reactionName + "." + suffix;
		converted.recordFile = false;
		converted.timeDependent = true;
		for (unsigned int i = 0; i < samples.size(); ++i) {
			const std::vector<double>* values = valuesForKind(samples[i], kind);
			if (reactionIndex >= values->size()) {
				if (errorMessage != nullptr) {
					*errorMessage = "BioReactionRateTimeCourse sample is missing reaction \"" + reactionName + "\".";
				}
				return false;
			}
			SimulationResultsObservation observation;
			observation.replication = 1;
			observation.time = samples[i].time;
			observation.hasTime = true;
			observation.value = (*values)[reactionIndex];
			observation.sourceLine = i + 1;
			converted.observations.push_back(observation);
		}

		*dataset = converted;
		return true;
	}

private:
	static std::string rateKindName(BioReactionRateKind kind) {
		switch (kind) {
			case BioReactionRateKind::Forward:
				return "forwardRate";
			case BioReactionRateKind::Reverse:
				return "reverseRate";
			case BioReactionRateKind::Net:
				return "netRate";
		}
		return "netRate";
	}

	static const std::vector<double>* valuesForKind(const BioReactionRateSample& sample, BioReactionRateKind kind) {
		switch (kind) {
			case BioReactionRateKind::Forward:
				return &sample.forwardRates;
			case BioReactionRateKind::Reverse:
				return &sample.reverseRates;
			case BioReactionRateKind::Net:
				return &sample.netRates;
		}
		return &sample.netRates;
	}
};

struct BioSpeciesDerivative {
	std::string speciesName;
	double derivative = 0.0;
};

struct BioSteadyStateCheck {
	bool steady = false;
	double maxAbsoluteDerivative = 0.0;
	double tolerance = 0.0;
	std::vector<BioSpeciesDerivative> derivatives;
};

struct BioParameterSensitivityEntry {
	std::string parameterName;
	double baseValue = 0.0;
	double delta = 0.0;
	double maxAbsoluteSensitivity = 0.0;
	std::vector<BioSpeciesDerivative> derivativeSensitivities;
};

struct BioSensitivityScan {
	double time = 0.0;
	std::vector<std::string> speciesNames;
	std::vector<BioParameterSensitivityEntry> entries;
};

class BioSimulationAnalysis {
public:
	static BioStoichiometryMatrix buildStoichiometryMatrix(const MassActionOdeSystem& system) {
		BioStoichiometryMatrix matrix;
		matrix.speciesNames.reserve(system.species().size());
		matrix.reactionNames.reserve(system.reactions().size());
		matrix.coefficients.assign(system.species().size(), std::vector<double>(system.reactions().size(), 0.0));

		for (const MassActionOdeSystem::Species& species : system.species()) {
			matrix.speciesNames.push_back(species.name);
		}
		for (unsigned int reactionIndex = 0; reactionIndex < system.reactions().size(); ++reactionIndex) {
			const MassActionOdeSystem::Reaction& reaction = system.reactions()[reactionIndex];
			matrix.reactionNames.push_back(reaction.name);
			for (const MassActionOdeSystem::StoichiometricTerm& reactant : reaction.reactants) {
				if (reactant.speciesIndex < matrix.coefficients.size()) {
					matrix.coefficients[reactant.speciesIndex][reactionIndex] -= reactant.stoichiometry;
				}
			}
			for (const MassActionOdeSystem::StoichiometricTerm& product : reaction.products) {
				if (product.speciesIndex < matrix.coefficients.size()) {
					matrix.coefficients[product.speciesIndex][reactionIndex] += product.stoichiometry;
				}
			}
		}
		return matrix;
	}

	static bool buildReactionRateTimeCourse(const MassActionOdeSystem& system, const BioSimulationResult& result,
	                                        BioReactionRateTimeCourse* timeCourse,
	                                        std::string* errorMessage = nullptr) {
		if (timeCourse == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid reaction rate time-course output parameter.";
			}
			return false;
		}

		BioReactionRateTimeCourse converted;
		converted.reactionNames.reserve(system.reactions().size());
		for (const MassActionOdeSystem::Reaction& reaction : system.reactions()) {
			converted.reactionNames.push_back(reaction.name);
		}

		for (const BioSimulationSample& sample : result.getSamples()) {
			std::vector<double> y;
			if (!stateFromSample(system, sample, &y, errorMessage)) {
				return false;
			}
			BioReactionRateSample rateSample;
			rateSample.time = sample.time;
			rateSample.forwardRates.reserve(system.reactions().size());
			rateSample.reverseRates.reserve(system.reactions().size());
			rateSample.netRates.reserve(system.reactions().size());
			for (unsigned int reactionIndex = 0; reactionIndex < system.reactions().size(); ++reactionIndex) {
				const double forwardRate = system.evaluateForwardRate(reactionIndex, y.data());
				const double reverseRate = system.evaluateReverseRate(reactionIndex, y.data());
				rateSample.forwardRates.push_back(forwardRate);
				rateSample.reverseRates.push_back(reverseRate);
				rateSample.netRates.push_back(forwardRate - reverseRate);
			}
			converted.samples.push_back(rateSample);
		}

		*timeCourse = converted;
		return true;
	}

	static bool checkSteadyState(const MassActionOdeSystem& system, const BioSimulationSample& sample, double tolerance,
	                             BioSteadyStateCheck* check, std::string* errorMessage = nullptr) {
		if (check == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid steady-state output parameter.";
			}
			return false;
		}
		if (tolerance < 0.0) {
			if (errorMessage != nullptr) {
				*errorMessage = "Steady-state tolerance must be non-negative.";
			}
			return false;
		}

		std::vector<double> y;
		if (!stateFromSample(system, sample, &y, errorMessage)) {
			return false;
		}
		std::vector<double> dydt(system.dimension(), 0.0);
		system.evaluate(sample.time, y.data(), dydt.data());

		BioSteadyStateCheck converted;
		converted.tolerance = tolerance;
		converted.derivatives.reserve(system.species().size());
		for (unsigned int i = 0; i < system.species().size(); ++i) {
			const double absoluteDerivative = std::abs(dydt[i]);
			converted.maxAbsoluteDerivative = std::max(converted.maxAbsoluteDerivative, absoluteDerivative);
			converted.derivatives.push_back({system.species()[i].name, dydt[i]});
		}
		converted.steady = converted.maxAbsoluteDerivative <= tolerance;

		*check = converted;
		return true;
	}

	static bool scanLocalParameterSensitivity(const MassActionOdeSystem& system, const BioSimulationSample& sample,
	                                          double relativeStep, double absoluteStep, BioSensitivityScan* scan,
	                                          std::string* errorMessage = nullptr) {
		if (scan == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid sensitivity scan output parameter.";
			}
			return false;
		}
		if (relativeStep < 0.0 || absoluteStep <= 0.0) {
			if (errorMessage != nullptr) {
				*errorMessage = "Sensitivity scan steps must satisfy relativeStep >= 0 and absoluteStep > 0.";
			}
			return false;
		}

		std::vector<double> y;
		if (!stateFromSample(system, sample, &y, errorMessage)) {
			return false;
		}
		std::vector<double> baseDerivatives;
		evaluateDerivatives(system, sample.time, y, &baseDerivatives);

		BioSensitivityScan converted;
		converted.time = sample.time;
		converted.speciesNames.reserve(system.species().size());
		for (const MassActionOdeSystem::Species& species : system.species()) {
			converted.speciesNames.push_back(species.name);
		}

		const std::vector<ParameterTarget> targets = parameterTargets(system);
		for (const ParameterTarget& target : targets) {
			std::vector<MassActionOdeSystem::Reaction> reactions = system.reactions();
			double baseValue = 0.0;
			if (!applyPerturbation(target, relativeStep, absoluteStep, &reactions, &baseValue)) {
				continue;
			}

			MassActionOdeSystem perturbed(system.species(), reactions);
			std::vector<double> perturbedDerivatives;
			evaluateDerivatives(perturbed, sample.time, y, &perturbedDerivatives);

			const double delta = deltaForValue(baseValue, relativeStep, absoluteStep);
			BioParameterSensitivityEntry entry;
			entry.parameterName = target.name;
			entry.baseValue = baseValue;
			entry.delta = delta;
			entry.derivativeSensitivities.reserve(system.species().size());
			for (unsigned int i = 0; i < system.species().size(); ++i) {
				const double sensitivity = (perturbedDerivatives[i] - baseDerivatives[i]) / delta;
				entry.maxAbsoluteSensitivity = std::max(entry.maxAbsoluteSensitivity, std::abs(sensitivity));
				entry.derivativeSensitivities.push_back({system.species()[i].name, sensitivity});
			}
			converted.entries.push_back(entry);
		}

		*scan = converted;
		return true;
	}

private:
	enum class ParameterTargetKind {
		ForwardRateConstant,
		ReverseRateConstant,
		NamedParameter
	};

	struct ParameterTarget {
		ParameterTargetKind kind = ParameterTargetKind::NamedParameter;
		unsigned int reactionIndex = 0;
		std::string name;
	};

	static void evaluateDerivatives(const MassActionOdeSystem& system, double time, const std::vector<double>& y,
	                                std::vector<double>* derivatives) {
		derivatives->assign(system.dimension(), 0.0);
		system.evaluate(time, y.data(), derivatives->data());
	}

	static std::vector<ParameterTarget> parameterTargets(const MassActionOdeSystem& system) {
		std::vector<ParameterTarget> targets;
		std::vector<std::string> namedParameters;
		for (unsigned int reactionIndex = 0; reactionIndex < system.reactions().size(); ++reactionIndex) {
			const MassActionOdeSystem::Reaction& reaction = system.reactions()[reactionIndex];
			if (reaction.kineticLawExpression.empty()) {
				targets.push_back({ParameterTargetKind::ForwardRateConstant, reactionIndex, reaction.name + ".rateConstant"});
			}
			if (reaction.reversible && reaction.reverseKineticLawExpression.empty()) {
				targets.push_back({ParameterTargetKind::ReverseRateConstant, reactionIndex, reaction.name + ".reverseRateConstant"});
			}
			for (const auto& parameter : reaction.parameters) {
				if (std::find(namedParameters.begin(), namedParameters.end(), parameter.first) == namedParameters.end()) {
					namedParameters.push_back(parameter.first);
					targets.push_back({ParameterTargetKind::NamedParameter, reactionIndex, parameter.first});
				}
			}
		}
		return targets;
	}

	static double deltaForValue(double value, double relativeStep, double absoluteStep) {
		return std::max(std::abs(value) * relativeStep, absoluteStep);
	}

	static bool applyPerturbation(const ParameterTarget& target, double relativeStep, double absoluteStep,
	                              std::vector<MassActionOdeSystem::Reaction>* reactions, double* baseValue) {
		if (reactions == nullptr || baseValue == nullptr || target.reactionIndex >= reactions->size()) {
			return false;
		}
		if (target.kind == ParameterTargetKind::ForwardRateConstant) {
			*baseValue = (*reactions)[target.reactionIndex].rateConstant;
			(*reactions)[target.reactionIndex].rateConstant += deltaForValue(*baseValue, relativeStep, absoluteStep);
			return true;
		}
		if (target.kind == ParameterTargetKind::ReverseRateConstant) {
			*baseValue = (*reactions)[target.reactionIndex].reverseRateConstant;
			(*reactions)[target.reactionIndex].reverseRateConstant += deltaForValue(*baseValue, relativeStep, absoluteStep);
			return true;
		}

		bool found = false;
		for (MassActionOdeSystem::Reaction& reaction : *reactions) {
			for (auto& parameter : reaction.parameters) {
				if (parameter.first == target.name) {
					if (!found) {
						*baseValue = parameter.second;
					}
					parameter.second += deltaForValue(parameter.second, relativeStep, absoluteStep);
					found = true;
				}
			}
		}
		return found;
	}

	static bool stateFromSample(const MassActionOdeSystem& system, const BioSimulationSample& sample,
	                            std::vector<double>* y, std::string* errorMessage) {
		if (y == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid state output parameter.";
			}
			return false;
		}
		y->assign(system.species().size(), 0.0);
		for (unsigned int speciesIndex = 0; speciesIndex < system.species().size(); ++speciesIndex) {
			const std::string& speciesName = system.species()[speciesIndex].name;
			bool found = false;
			for (const BioSimulationSpeciesAmount& amount : sample.species) {
				if (amount.speciesName == speciesName) {
					(*y)[speciesIndex] = amount.amount;
					found = true;
					break;
				}
			}
			if (!found) {
				if (errorMessage != nullptr) {
					*errorMessage = "BioSimulationSample is missing species \"" + speciesName + "\".";
				}
				return false;
			}
		}
		return true;
	}
};

#endif /* BIOSIMULATIONANALYSIS_H */
