#ifndef BIOSIMULATIONRESULT_H
#define BIOSIMULATIONRESULT_H

#include <algorithm>
#include <string>
#include <vector>

#include "tools/SimulationResultsDataset.h"

struct BioSimulationSpeciesAmount {
	std::string speciesName;
	double amount = 0.0;
};

struct BioSimulationSample {
	double time = 0.0;
	std::vector<BioSimulationSpeciesAmount> species;
};

class BioSimulationResult {
public:
	void clear() {
		_networkName.clear();
		_startTime = 0.0;
		_stopTime = 0.0;
		_stepSize = 0.0;
		_speciesNames.clear();
		_samples.clear();
	}

	void configure(std::string networkName, double startTime, double stopTime, double stepSize,
	               const std::vector<std::string>& speciesNames) {
		_networkName = networkName;
		_startTime = startTime;
		_stopTime = stopTime;
		_stepSize = stepSize;
		_speciesNames = speciesNames;
		_samples.clear();
	}

	void appendSample(double time, const std::vector<BioSimulationSpeciesAmount>& speciesAmounts) {
		BioSimulationSample sample;
		sample.time = time;
		sample.species = speciesAmounts;
		_samples.push_back(sample);
	}

	bool empty() const {
		return _samples.empty();
	}

	unsigned int sampleCount() const {
		return static_cast<unsigned int>(_samples.size());
	}

	const std::string& getNetworkName() const {
		return _networkName;
	}

	double getStartTime() const {
		return _startTime;
	}

	double getStopTime() const {
		return _stopTime;
	}

	double getStepSize() const {
		return _stepSize;
	}

	const std::vector<std::string>& getSpeciesNames() const {
		return _speciesNames;
	}

	const std::vector<BioSimulationSample>& getSamples() const {
		return _samples;
	}

	bool hasSpecies(const std::string& speciesName) const {
		return std::find(_speciesNames.begin(), _speciesNames.end(), speciesName) != _speciesNames.end();
	}

	bool toDataset(const std::string& speciesName, SimulationResultsDataset* dataset,
	               std::string* errorMessage = nullptr) const {
		if (dataset == nullptr) {
			if (errorMessage != nullptr) {
				*errorMessage = "Invalid dataset output parameter.";
			}
			return false;
		}
		if (!hasSpecies(speciesName)) {
			if (errorMessage != nullptr) {
				*errorMessage = "BioSimulationResult does not contain species \"" + speciesName + "\".";
			}
			return false;
		}

		SimulationResultsDataset converted;
		converted.sourceDescription = _networkName;
		converted.expression = speciesName;
		converted.expressionName = speciesName;
		converted.recordFile = false;
		converted.timeDependent = true;
		for (unsigned int i = 0; i < _samples.size(); ++i) {
			bool found = false;
			const double value = valueForSpecies(_samples[i], speciesName, &found);
			if (!found) {
				if (errorMessage != nullptr) {
					*errorMessage = "BioSimulationResult sample is missing species \"" + speciesName + "\".";
				}
				return false;
			}
			SimulationResultsObservation observation;
			observation.replication = 1;
			observation.time = _samples[i].time;
			observation.hasTime = true;
			observation.value = value;
			observation.sourceLine = i + 1;
			converted.observations.push_back(observation);
		}

		*dataset = converted;
		return true;
	}

	std::vector<SimulationResultsDataset> toDatasets() const {
		std::vector<SimulationResultsDataset> datasets;
		for (const std::string& speciesName : _speciesNames) {
			SimulationResultsDataset dataset;
			if (toDataset(speciesName, &dataset)) {
				datasets.push_back(dataset);
			}
		}
		return datasets;
	}

private:
	double valueForSpecies(const BioSimulationSample& sample, const std::string& speciesName, bool* found) const {
		for (const BioSimulationSpeciesAmount& amount : sample.species) {
			if (amount.speciesName == speciesName) {
				if (found != nullptr) {
					*found = true;
				}
				return amount.amount;
			}
		}
		if (found != nullptr) {
			*found = false;
		}
		return 0.0;
	}

private:
	std::string _networkName = "";
	double _startTime = 0.0;
	double _stopTime = 0.0;
	double _stepSize = 0.0;
	std::vector<std::string> _speciesNames;
	std::vector<BioSimulationSample> _samples;
};

#endif /* BIOSIMULATIONRESULT_H */
