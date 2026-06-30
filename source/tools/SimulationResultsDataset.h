#ifndef SIMULATIONRESULTSDATASET_H
#define SIMULATIONRESULTSDATASET_H

#include <string>
#include <vector>

enum class SimulationResultsDatasetFormat {
	Unknown,
	RawNumeric,
	RecordLegacy,
	RecordEnriched,
	GuiTabular
};

struct SimulationResultsObservation {
	unsigned int replication = 1;
	double time = 0.0;
	bool hasTime = false;
	double value = 0.0;
	unsigned int sourceLine = 0;
};

struct SimulationResultsDataset {
	std::string datasetName;
	std::string randomVariableName;
	std::string description;
	std::string variableType = "Continuous numeric";
	std::string source;
	std::string sourceDescription;
	std::string expression;
	std::string expressionName;
	std::string formatVersion;
	SimulationResultsDatasetFormat formatKind = SimulationResultsDatasetFormat::Unknown;
	bool recordFile = false;
	bool timeDependent = false;
	std::vector<SimulationResultsObservation> observations;
	std::vector<std::string> previewLines;

	/*!
	 Returns the display name for the detected dataset format.
	 */
	std::string formatKindName() const;

	/*!
	 Returns true when this dataset was imported from a Genesys Record file.
	 */
	bool isRecordFormat() const;

	/*!
	 Returns the observation values without replication or time metadata.
	 */
	std::vector<double> values() const;

	/*!
	 Returns the distinct replication identifiers present in the observations.
	 */
	std::vector<unsigned int> replications() const;

	/*!
	 Returns all observation values for a single replication.
	 */
	std::vector<double> valuesForReplication(unsigned int replication) const;
};

class SimulationResultsDatasetParser {
public:
	/*!
	 Loads a simulation-results dataset from a text file.

	 The parser intentionally detects formats in this order: GUI tabular,
	 Genesys Record, then raw numeric. Once a format is detected, no residual
	 heuristic reinterprets the same file as another format.
	 */
	static bool loadFromTextFile(const std::string& fileName, SimulationResultsDataset* dataset,
	                             std::string* errorMessage = nullptr);
};

#endif /* SIMULATIONRESULTSDATASET_H */
