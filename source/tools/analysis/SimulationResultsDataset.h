#ifndef SIMULATIONRESULTSDATASET_H
#define SIMULATIONRESULTSDATASET_H

#include "DatasetLoader.h"
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Format kind
// ---------------------------------------------------------------------------

enum class SimulationResultsDatasetFormat {
	Unknown,
	RawNumeric,
	RecordLegacy,
	RecordEnriched,
	GuiTabular
};

// ---------------------------------------------------------------------------
// Single observation (one row of a simulation output file)
// ---------------------------------------------------------------------------

struct SimulationResultsObservation {
	unsigned int replication = 1;
	double time = 0.0;
	bool hasTime = false;
	double value = 0.0;
	unsigned int sourceLine = 0;
};

// ---------------------------------------------------------------------------
// Dataset container
// ---------------------------------------------------------------------------

/**
 * @brief Container for a parsed simulation-results dataset.
 *
 * Holds simulation-specific metadata (format kind, replication, time-
 * dependency) and the full observation list. After parsing, the internal
 * DatasetLoader is built from the observation values (via buildLoader()) and
 * provides all numeric statistics, sorted access and analysis helpers.
 *
 * Responsibilities:
 *  - Own and expose simulation metadata and the observation list.
 *  - Delegate all numeric operations to the internal DatasetLoader.
 *
 * The parser (SimulationResultsParser) calls buildLoader() automatically
 * after a successful parse. Call it manually when observations are modified
 * after construction.
 */
class SimulationResultsDataset {
public:
	// -- Simulation metadata (public for direct access by parsers/consumers) --
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

public:
	// -- Format utilities --

	/** @brief Returns a human-readable name for the detected file format. */
	std::string formatKindName() const;
	/** @brief Returns whether the dataset was parsed from a GenESyS Record file. */
	bool isRecordFormat() const;

	// -- Simulation-specific data access --

	/** @brief Returns all observation values without replication/time metadata. */
	std::vector<double> values() const;
	/** @brief Returns the unique replication numbers present in the dataset. */
	std::vector<unsigned int> replications() const;
	/** @brief Returns observation values for one replication. */
	std::vector<double> valuesForReplication(unsigned int replication) const;

	// -- Numeric loader --

	/*!
	 * Builds or rebuilds the internal DatasetLoader from the current observation
	 * values. Must be called after observations are modified manually.
	 * Called automatically by SimulationResultsParser after a successful parse.
	 */
	void buildLoader();

	/*!
	 * Returns the internal DatasetLoader for numeric access and statistics.
	 * The loader is valid only if buildLoader() has been called after the last
	 * observation change.
	 */
	const DatasetLoader& loader() const;

	// -- Numeric statistics (delegates to internal DatasetLoader) --

	/** @brief Returns whether the internal loader has usable numeric data. */
	bool hasNumericData() const;
	/** @brief Returns the number of numeric observations. */
	std::size_t count() const;
	/** @brief Returns the minimum observation value. */
	double min() const;
	/** @brief Returns the maximum observation value. */
	double max() const;
	/** @brief Returns the arithmetic mean of observation values. */
	double mean() const;
	/** @brief Returns the sample variance of observation values. */
	double variance() const;
	/** @brief Returns the sample standard deviation of observation values. */
	double stddev() const;
	/** @brief Returns whether any observation value is negative. */
	bool hasNegativeData() const;

	// -- Sorted and unsorted data vectors (delegates to internal DatasetLoader) --

	/** @brief Returns values in parsed order. */
	const std::vector<double>& data() const;
	/** @brief Returns values sorted in nondecreasing order. */
	const std::vector<double>& sortedData() const;

private:
	DatasetLoader _loader;
};

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------

/**
 * @brief Parser for simulation-results dataset text files.
 *
 * Detects format and dispatches to the appropriate sub-parser. Format
 * detection order (by contract): GUI tabular first, then Genesys Record,
 * then raw numeric as fallback. On success, builds the dataset's internal
 * DatasetLoader automatically.
 *
 * Supported formats:
 *  - GuiTabular  : semantic CSV with at least a "value" column header.
 *  - RecordEnriched / RecordLegacy : '#'-prefixed metadata + numeric rows.
 *  - RawNumeric  : one finite value per line, no metadata.
 */
class SimulationResultsParser {
public:
	/** @brief Loads a simulation-results text file into a structured dataset. */
	static bool loadFromTextFile(
	    const std::string& fileName,
	    SimulationResultsDataset* dataset,
	    std::string* errorMessage = nullptr
	);
};

// Backward-compatible alias retained for existing call sites.
using SimulationResultsDatasetParser = SimulationResultsParser;

#endif /* SIMULATIONRESULTSDATASET_H */
