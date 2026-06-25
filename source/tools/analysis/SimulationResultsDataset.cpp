#include "SimulationResultsDataset.h"

#include <set>

// ---------------------------------------------------------------------------
// SimulationResultsDataset – format utilities
// ---------------------------------------------------------------------------

std::string SimulationResultsDataset::formatKindName() const {
	switch (formatKind) {
		case SimulationResultsDatasetFormat::RawNumeric:     return "Raw Numeric Dataset";
		case SimulationResultsDatasetFormat::RecordLegacy:   return "Genesys Record Dataset (legacy)";
		case SimulationResultsDatasetFormat::RecordEnriched: return "Enriched Genesys Record Dataset";
		case SimulationResultsDatasetFormat::GuiTabular:     return "GUI Tabular Dataset";
		case SimulationResultsDatasetFormat::Unknown:        return "Unknown Dataset";
	}
	return "Unknown Dataset";
}

bool SimulationResultsDataset::isRecordFormat() const {
	return formatKind == SimulationResultsDatasetFormat::RecordLegacy
	    || formatKind == SimulationResultsDatasetFormat::RecordEnriched
	    || recordFile;
}

// ---------------------------------------------------------------------------
// SimulationResultsDataset – simulation-specific data access
// ---------------------------------------------------------------------------

std::vector<double> SimulationResultsDataset::values() const {
	std::vector<double> result;
	result.reserve(observations.size());
	for (const SimulationResultsObservation& obs : observations) {
		result.push_back(obs.value);
	}
	return result;
}

std::vector<unsigned int> SimulationResultsDataset::replications() const {
	std::set<unsigned int> unique;
	for (const SimulationResultsObservation& obs : observations) {
		unique.insert(obs.replication);
	}
	return std::vector<unsigned int>(unique.begin(), unique.end());
}

std::vector<double> SimulationResultsDataset::valuesForReplication(unsigned int replication) const {
	std::vector<double> result;
	for (const SimulationResultsObservation& obs : observations) {
		if (obs.replication == replication) {
			result.push_back(obs.value);
		}
	}
	return result;
}

// ---------------------------------------------------------------------------
// SimulationResultsDataset – numeric loader
// ---------------------------------------------------------------------------

void SimulationResultsDataset::buildLoader() {
	_loader.loadFromVector(values());
}

const DatasetLoader& SimulationResultsDataset::loader() const {
	return _loader;
}

// ---------------------------------------------------------------------------
// SimulationResultsDataset – numeric statistics (delegates to DatasetLoader)
// ---------------------------------------------------------------------------

bool SimulationResultsDataset::hasNumericData() const {
	return _loader.isUsable();
}

std::size_t SimulationResultsDataset::count() const {
	return _loader.count();
}

double SimulationResultsDataset::min() const {
	return _loader.min();
}

double SimulationResultsDataset::max() const {
	return _loader.max();
}

double SimulationResultsDataset::mean() const {
	return _loader.mean();
}

double SimulationResultsDataset::variance() const {
	return _loader.variance();
}

double SimulationResultsDataset::stddev() const {
	return _loader.stddev();
}

bool SimulationResultsDataset::hasNegativeData() const {
	return _loader.hasNegativeData();
}

const std::vector<double>& SimulationResultsDataset::data() const {
	return _loader.data();
}

const std::vector<double>& SimulationResultsDataset::sortedData() const {
	return _loader.sortedData();
}
