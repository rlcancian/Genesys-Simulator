#ifndef SIMULATIONRESULTSDATASET_H
#define SIMULATIONRESULTSDATASET_H

#include <string>
#include <vector>

struct SimulationResultsObservation {
	unsigned int replication = 1;
	double time = 0.0;
	bool hasTime = false;
	double value = 0.0;
	unsigned int sourceLine = 0;
};

struct SimulationResultsDataset {
	std::string sourceDescription;
	std::string expression;
	std::string expressionName;
	bool recordFile = false;
	bool timeDependent = false;
	std::vector<SimulationResultsObservation> observations;
	std::vector<std::string> previewLines;

	std::vector<double> values() const;
	std::vector<unsigned int> replications() const;
	std::vector<double> valuesForReplication(unsigned int replication) const;
};

class SimulationResultsDatasetParser {
public:
	static bool loadFromTextFile(const std::string& fileName, SimulationResultsDataset* dataset,
	                             std::string* errorMessage = nullptr);
};

#endif /* SIMULATIONRESULTSDATASET_H */
