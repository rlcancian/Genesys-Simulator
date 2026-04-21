#include "tools/SimulationResultsDataset.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>

namespace {
	struct TextLine {
		std::string text;
		unsigned int number = 0;
	};

	static std::string trim(const std::string& text) {
		const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		});
		const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		}).base();
		if (first >= last) {
			return "";
		}
		return std::string(first, last);
	}

	static std::string lower(std::string text) {
		std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		return text;
	}

	static std::string fileNameOnly(const std::string& path) {
		const std::size_t slash = path.find_last_of("/\\");
		return slash == std::string::npos ? path : path.substr(slash + 1);
	}

	static std::string fileBaseName(const std::string& path) {
		const std::string name = fileNameOnly(path);
		const std::size_t dot = name.find_last_of('.');
		return dot == std::string::npos ? name : name.substr(0, dot);
	}

	static bool parseDoubleToken(const std::string& token, double* value) {
		if (token.empty()) {
			return false;
		}
		char* end = nullptr;
		errno = 0;
		const double parsed = std::strtod(token.c_str(), &end);
		if (end == token.c_str() || errno == ERANGE) {
			return false;
		}
		while (end != nullptr && *end != '\0') {
			if (std::isspace(static_cast<unsigned char>(*end)) == 0) {
				return false;
			}
			++end;
		}
		if (!std::isfinite(parsed)) {
			return false;
		}
		if (value != nullptr) {
			*value = parsed;
		}
		return true;
	}

	static std::vector<std::string> tokenizeDataLine(std::string line) {
		for (char& ch : line) {
			if (ch == ',' || ch == ';' || ch == '\t') {
				ch = ' ';
			}
		}
		std::istringstream stream(line);
		std::vector<std::string> tokens;
		std::string token;
		while (stream >> token) {
			tokens.push_back(token);
		}
		return tokens;
	}

	static std::vector<std::string> parseDelimitedLine(const std::string& line) {
		std::vector<std::string> fields;
		std::string current;
		bool quoted = false;
		for (std::size_t i = 0; i < line.size(); ++i) {
			const char ch = line.at(i);
			if (ch == '"') {
				if (quoted && i + 1 < line.size() && line.at(i + 1) == '"') {
					current.push_back('"');
					++i;
				} else {
					quoted = !quoted;
				}
				continue;
			}
			if (!quoted && (ch == ',' || ch == ';' || ch == '\t')) {
				fields.push_back(trim(current));
				current.clear();
				continue;
			}
			current.push_back(ch);
		}
		fields.push_back(trim(current));
		return fields;
	}

	static std::map<std::string, std::string> parseRecordMetadataLine(const std::string& line) {
		std::map<std::string, std::string> metadata;
		std::string content = trim(line);
		if (!content.empty() && content.front() == '#') {
			content.erase(content.begin());
		}

		std::size_t pos = 0;
		while (pos < content.size()) {
			while (pos < content.size() && (std::isspace(static_cast<unsigned char>(content.at(pos))) != 0 || content.at(pos) == ',')) {
				++pos;
			}
			const std::size_t keyStart = pos;
			while (pos < content.size() && content.at(pos) != '=') {
				++pos;
			}
			if (pos >= content.size()) {
				break;
			}
			const std::string key = lower(trim(content.substr(keyStart, pos - keyStart)));
			++pos;
			while (pos < content.size() && std::isspace(static_cast<unsigned char>(content.at(pos))) != 0) {
				++pos;
			}
			std::string value;
			if (pos < content.size() && content.at(pos) == '"') {
				++pos;
				while (pos < content.size()) {
					const char ch = content.at(pos++);
					if (ch == '"') {
						if (pos < content.size() && content.at(pos) == '"') {
							value.push_back('"');
							++pos;
							continue;
						}
						break;
					}
					value.push_back(ch);
				}
			} else {
				const std::size_t valueStart = pos;
				while (pos < content.size() && content.at(pos) != ',') {
					++pos;
				}
				value = trim(content.substr(valueStart, pos - valueStart));
			}
			if (!key.empty()) {
				metadata[key] = value;
			}
		}
		return metadata;
	}

	static bool hasRecordMetadataKey(const std::map<std::string, std::string>& metadata) {
		return std::any_of(metadata.begin(), metadata.end(), [](const auto& entry) {
			static const std::set<std::string> keys = {
				"recordformat", "format", "formatversion", "datasetname", "randomvariablename", "variabletype",
				"description", "source", "expression", "expressionname", "timedependent", "columns", "replicationnumber"
			};
			return keys.find(entry.first) != keys.end();
		});
	}

	static bool metadataValue(const std::map<std::string, std::string>& metadata, const std::string& key, std::string* value) {
		const auto it = metadata.find(lower(key));
		if (it == metadata.end()) {
			return false;
		}
		if (value != nullptr) {
			*value = it->second;
		}
		return true;
	}

	static bool parseUnsigned(const std::string& text, unsigned int* value) {
		char* end = nullptr;
		const std::string stripped = trim(text);
		const unsigned long parsed = std::strtoul(stripped.c_str(), &end, 10);
		if (end == stripped.c_str() || parsed == 0 || parsed > std::numeric_limits<unsigned int>::max()) {
			return false;
		}
		while (end != nullptr && *end != '\0') {
			if (std::isspace(static_cast<unsigned char>(*end)) == 0) {
				return false;
			}
			++end;
		}
		if (value != nullptr) {
			*value = static_cast<unsigned int>(parsed);
		}
		return true;
	}

	static bool parseBool(const std::string& text, bool* value) {
		const std::string normalized = lower(trim(text));
		if (normalized == "true" || normalized == "1" || normalized == "yes") {
			if (value != nullptr) {
				*value = true;
			}
			return true;
		}
		if (normalized == "false" || normalized == "0" || normalized == "no") {
			if (value != nullptr) {
				*value = false;
			}
			return true;
		}
		return false;
	}

	static bool normalizeVariableType(const std::string& text, std::string* variableType, std::string* errorMessage) {
		const std::string normalized = lower(trim(text));
		if (normalized.empty() || normalized == "continuous numeric") {
			if (variableType != nullptr) {
				*variableType = "Continuous numeric";
			}
			return true;
		}
		if (normalized == "discrete numeric") {
			if (variableType != nullptr) {
				*variableType = "Discrete numeric";
			}
			return true;
		}
		if (errorMessage != nullptr) {
			*errorMessage = "Unsupported VariableType \"" + text + "\". Expected Continuous numeric or Discrete numeric.";
		}
		return false;
	}

	static bool firstUsefulLine(const std::vector<TextLine>& lines, TextLine* usefulLine) {
		for (const TextLine& line : lines) {
			if (!trim(line.text).empty()) {
				if (usefulLine != nullptr) {
					*usefulLine = line;
				}
				return true;
			}
		}
		return false;
	}

	static bool isGuiTabularHeader(const std::string& line, std::map<std::string, unsigned int>* columnIndexes) {
		const std::string stripped = trim(line);
		if (stripped.empty() || stripped.front() == '#') {
			return false;
		}
		const std::vector<std::string> columns = parseDelimitedLine(line);
		std::map<std::string, unsigned int> indexes;
		for (unsigned int i = 0; i < columns.size(); ++i) {
			indexes[lower(trim(columns.at(i)))] = i;
		}
		const bool hasValue = indexes.find("value") != indexes.end();
		const bool hasSemanticColumn = indexes.find("dataset") != indexes.end() || indexes.find("variable") != indexes.end()
				|| indexes.find("replication") != indexes.end() || indexes.find("time") != indexes.end();
		if (!hasValue || !hasSemanticColumn) {
			return false;
		}
		if (columnIndexes != nullptr) {
			*columnIndexes = indexes;
		}
		return true;
	}

	static SimulationResultsDatasetFormat detectFormat(const std::vector<TextLine>& lines) {
		TextLine first;
		if (firstUsefulLine(lines, &first) && isGuiTabularHeader(first.text, nullptr)) {
			return SimulationResultsDatasetFormat::GuiTabular;
		}

		for (const TextLine& line : lines) {
			const std::string stripped = trim(line.text);
			if (stripped.empty() || stripped.front() != '#') {
				continue;
			}
			if (hasRecordMetadataKey(parseRecordMetadataLine(stripped))) {
				return SimulationResultsDatasetFormat::RecordLegacy;
			}
		}

		return SimulationResultsDatasetFormat::RawNumeric;
	}

	static void addPreviewLines(const std::vector<TextLine>& lines, SimulationResultsDataset* dataset) {
		for (const TextLine& line : lines) {
			if (dataset->previewLines.size() >= 20) {
				return;
			}
			dataset->previewLines.push_back(line.text);
		}
	}

	static bool parseRawNumericDataset(const std::vector<TextLine>& lines, const std::string& fileName,
	                                  SimulationResultsDataset* dataset, std::string* errorMessage) {
		SimulationResultsDataset parsed;
		parsed.formatKind = SimulationResultsDatasetFormat::RawNumeric;
		parsed.datasetName = fileNameOnly(fileName);
		parsed.randomVariableName = fileBaseName(fileName);
		parsed.variableType = "Continuous numeric";
		parsed.description = "Raw observations loaded from " + fileNameOnly(fileName);
		parsed.source = fileName;
		parsed.sourceDescription = parsed.source;
		parsed.recordFile = false;
		parsed.timeDependent = false;
		addPreviewLines(lines, &parsed);

		for (const TextLine& line : lines) {
			const std::string stripped = trim(line.text);
			if (stripped.empty()) {
				continue;
			}
			double value = 0.0;
			if (!parseDoubleToken(stripped, &value)) {
				if (errorMessage != nullptr) {
					*errorMessage = "Invalid raw numeric observation at line " + std::to_string(line.number) + ".";
				}
				return false;
			}
			SimulationResultsObservation observation;
			observation.replication = 1;
			observation.hasTime = false;
			observation.value = value;
			observation.sourceLine = line.number;
			parsed.observations.push_back(observation);
		}

		if (parsed.observations.empty()) {
			if (errorMessage != nullptr) {
				*errorMessage = "The selected raw numeric file does not contain observations.";
			}
			return false;
		}
		*dataset = parsed;
		return true;
	}

	static bool parseGuiTabularDataset(const std::vector<TextLine>& lines, const std::string& fileName,
	                                  SimulationResultsDataset* dataset, std::string* errorMessage) {
		TextLine header;
		std::map<std::string, unsigned int> columns;
		if (!firstUsefulLine(lines, &header) || !isGuiTabularHeader(header.text, &columns)) {
			if (errorMessage != nullptr) {
				*errorMessage = "The selected tabular dataset does not contain a semantic header with a value column.";
			}
			return false;
		}

		SimulationResultsDataset parsed;
		parsed.formatKind = SimulationResultsDatasetFormat::GuiTabular;
		parsed.datasetName = fileNameOnly(fileName);
		parsed.randomVariableName = fileBaseName(fileName);
		parsed.variableType = "Continuous numeric";
		parsed.description = "GUI tabular observations loaded from " + fileNameOnly(fileName);
		parsed.source = fileName;
		parsed.sourceDescription = parsed.source;
		addPreviewLines(lines, &parsed);

		for (const TextLine& line : lines) {
			if (line.number <= header.number || trim(line.text).empty()) {
				continue;
			}
			const std::vector<std::string> fields = parseDelimitedLine(line.text);
			auto fieldAt = [&fields](const std::map<std::string, unsigned int>& indexes, const std::string& key) -> std::string {
				const auto it = indexes.find(key);
				if (it == indexes.end() || it->second >= fields.size()) {
					return "";
				}
				return trim(fields.at(it->second));
			};

			double value = 0.0;
			if (!parseDoubleToken(fieldAt(columns, "value"), &value)) {
				if (errorMessage != nullptr) {
					*errorMessage = "Invalid tabular dataset value at line " + std::to_string(line.number) + ".";
				}
				return false;
			}

			SimulationResultsObservation observation;
			observation.sourceLine = line.number;
			observation.value = value;
			const std::string replication = fieldAt(columns, "replication");
			if (!replication.empty() && !parseUnsigned(replication, &observation.replication)) {
				if (errorMessage != nullptr) {
					*errorMessage = "Invalid tabular dataset replication at line " + std::to_string(line.number) + ".";
				}
				return false;
			}
			const std::string time = fieldAt(columns, "time");
			if (!time.empty()) {
				if (!parseDoubleToken(time, &observation.time)) {
					if (errorMessage != nullptr) {
						*errorMessage = "Invalid tabular dataset time at line " + std::to_string(line.number) + ".";
					}
					return false;
				}
				observation.hasTime = true;
				parsed.timeDependent = true;
			}
			const std::string rowDataset = fieldAt(columns, "dataset");
			if (!rowDataset.empty() && (parsed.observations.empty() || parsed.datasetName == fileNameOnly(fileName))) {
				parsed.datasetName = rowDataset;
			}
			const std::string rowVariable = fieldAt(columns, "variable");
			if (!rowVariable.empty() && (parsed.observations.empty() || parsed.randomVariableName == fileBaseName(fileName))) {
				parsed.randomVariableName = rowVariable;
			}
			parsed.observations.push_back(observation);
		}

		if (parsed.observations.empty()) {
			if (errorMessage != nullptr) {
				*errorMessage = "The selected tabular dataset does not contain observations.";
			}
			return false;
		}
		*dataset = parsed;
		return true;
	}

	static bool parseRecordDataset(const std::vector<TextLine>& lines, const std::string& fileName,
	                              SimulationResultsDataset* dataset, std::string* errorMessage) {
		SimulationResultsDataset parsed;
		parsed.formatKind = SimulationResultsDatasetFormat::RecordLegacy;
		parsed.recordFile = true;
		parsed.variableType = "Continuous numeric";
		parsed.source = fileName;
		parsed.sourceDescription = parsed.source;
		addPreviewLines(lines, &parsed);

		unsigned int currentReplication = 1;
		std::string columns;
		bool hasColumns = false;
		bool hasTimeDependent = false;
		bool hasEnrichedMetadata = false;

		for (const TextLine& line : lines) {
			const std::string stripped = trim(line.text);
			if (stripped.empty()) {
				continue;
			}
			if (stripped.front() == '#') {
				const std::map<std::string, std::string> metadata = parseRecordMetadataLine(stripped);
				std::string value;
				if (metadataValue(metadata, "format", &value)) {
					parsed.formatKind = SimulationResultsDatasetFormat::RecordEnriched;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "formatversion", &value)) {
					parsed.formatVersion = value;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "datasetname", &value)) {
					parsed.datasetName = value;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "randomvariablename", &value)) {
					parsed.randomVariableName = value;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "variabletype", &value)) {
					if (!normalizeVariableType(value, &parsed.variableType, errorMessage)) {
						return false;
					}
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "description", &value)) {
					parsed.description = value;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "source", &value)) {
					parsed.source = value;
					parsed.sourceDescription = value;
					hasEnrichedMetadata = true;
				}
				if (metadataValue(metadata, "expression", &value)) {
					parsed.expression = value;
				}
				// ExpressionName remains a legacy field so old Record outputs keep their analytical name.
				if (metadataValue(metadata, "expressionname", &value)) {
					parsed.expressionName = value;
				}
				if (metadataValue(metadata, "timedependent", &value)) {
					if (!parseBool(value, &parsed.timeDependent)) {
						if (errorMessage != nullptr) {
							*errorMessage = "Invalid TimeDependent value at line " + std::to_string(line.number) + ".";
						}
						return false;
					}
					hasTimeDependent = true;
				}
				if (metadataValue(metadata, "columns", &value)) {
					columns = lower(trim(value));
					if (columns != "value" && columns != "time value") {
						if (errorMessage != nullptr) {
							*errorMessage = "Invalid Columns value at line " + std::to_string(line.number) + ".";
						}
						return false;
					}
					hasColumns = true;
					parsed.timeDependent = columns == "time value";
				}
				if (metadataValue(metadata, "replicationnumber", &value)) {
					if (!parseUnsigned(value, &currentReplication)) {
						if (errorMessage != nullptr) {
							*errorMessage = "Invalid ReplicationNumber value at line " + std::to_string(line.number) + ".";
						}
						return false;
					}
				}
				continue;
			}

			std::vector<double> numericTokens;
			for (const std::string& token : tokenizeDataLine(stripped)) {
				double value = 0.0;
				if (!parseDoubleToken(token, &value)) {
					if (errorMessage != nullptr) {
						*errorMessage = "Invalid Record observation at line " + std::to_string(line.number) + ".";
					}
					return false;
				}
				numericTokens.push_back(value);
			}

			const bool useTime = hasColumns ? columns == "time value" : (hasTimeDependent ? parsed.timeDependent : numericTokens.size() == 2);
			const std::size_t expectedTokens = useTime ? 2 : 1;
			if (numericTokens.size() != expectedTokens) {
				if (errorMessage != nullptr) {
					*errorMessage = "Invalid Record observation column count at line " + std::to_string(line.number) + ".";
				}
				return false;
			}

			SimulationResultsObservation observation;
			observation.replication = currentReplication;
			observation.sourceLine = line.number;
			if (useTime) {
				observation.time = numericTokens.at(0);
				observation.hasTime = true;
				observation.value = numericTokens.at(1);
				parsed.timeDependent = true;
			} else {
				observation.value = numericTokens.at(0);
			}
			parsed.observations.push_back(observation);
		}

		if (parsed.observations.empty()) {
			if (errorMessage != nullptr) {
				*errorMessage = "The selected Record file does not contain numeric observations.";
			}
			return false;
		}

		if (hasEnrichedMetadata && parsed.formatKind == SimulationResultsDatasetFormat::RecordLegacy) {
			parsed.formatKind = SimulationResultsDatasetFormat::RecordEnriched;
		}
		if (parsed.datasetName.empty()) {
			parsed.datasetName = !parsed.expressionName.empty() ? parsed.expressionName : fileNameOnly(fileName);
		}
		if (parsed.randomVariableName.empty()) {
			parsed.randomVariableName = !parsed.expressionName.empty() ? parsed.expressionName : fileBaseName(fileName);
		}
		if (parsed.description.empty()) {
			parsed.description = "Genesys Record observations loaded from " + fileNameOnly(fileName) + ".";
		}
		if (parsed.source.empty()) {
			parsed.source = fileName;
			parsed.sourceDescription = fileName;
		}
		*dataset = parsed;
		return true;
	}
}

std::string SimulationResultsDataset::formatKindName() const {
	switch (formatKind) {
		case SimulationResultsDatasetFormat::RawNumeric:
			return "Raw Numeric Dataset";
		case SimulationResultsDatasetFormat::RecordLegacy:
			return "Genesys Record Dataset (legacy)";
		case SimulationResultsDatasetFormat::RecordEnriched:
			return "Enriched Genesys Record Dataset";
		case SimulationResultsDatasetFormat::GuiTabular:
			return "GUI Tabular Dataset";
		case SimulationResultsDatasetFormat::Unknown:
			return "Unknown Dataset";
	}
	return "Unknown Dataset";
}

bool SimulationResultsDataset::isRecordFormat() const {
	return formatKind == SimulationResultsDatasetFormat::RecordLegacy || formatKind == SimulationResultsDatasetFormat::RecordEnriched || recordFile;
}

std::vector<double> SimulationResultsDataset::values() const {
	std::vector<double> result;
	result.reserve(observations.size());
	for (const SimulationResultsObservation& observation : observations) {
		result.push_back(observation.value);
	}
	return result;
}

std::vector<unsigned int> SimulationResultsDataset::replications() const {
	std::set<unsigned int> unique;
	for (const SimulationResultsObservation& observation : observations) {
		unique.insert(observation.replication);
	}
	return std::vector<unsigned int>(unique.begin(), unique.end());
}

std::vector<double> SimulationResultsDataset::valuesForReplication(unsigned int replication) const {
	std::vector<double> result;
	for (const SimulationResultsObservation& observation : observations) {
		if (observation.replication == replication) {
			result.push_back(observation.value);
		}
	}
	return result;
}

bool SimulationResultsDatasetParser::loadFromTextFile(const std::string& fileName, SimulationResultsDataset* dataset,
                                                      std::string* errorMessage) {
	if (dataset == nullptr) {
		if (errorMessage != nullptr) {
			*errorMessage = "Invalid dataset output parameter.";
		}
		return false;
	}

	std::ifstream file(fileName);
	if (!file.is_open()) {
		if (errorMessage != nullptr) {
			*errorMessage = "Could not open selected file.";
		}
		return false;
	}

	std::vector<TextLine> lines;
	std::string line;
	unsigned int lineNumber = 0;
	while (std::getline(file, line)) {
		lines.push_back({line, ++lineNumber});
	}

	// Format detection is ordered by contract: semantic CSV first, Record metadata second, raw numeric last.
	// This prevents old numeric heuristics from stealing a file after a richer format has been identified.
	const SimulationResultsDatasetFormat format = detectFormat(lines);
	if (format == SimulationResultsDatasetFormat::GuiTabular) {
		return parseGuiTabularDataset(lines, fileName, dataset, errorMessage);
	}
	if (format == SimulationResultsDatasetFormat::RecordLegacy) {
		return parseRecordDataset(lines, fileName, dataset, errorMessage);
	}
	return parseRawNumericDataset(lines, fileName, dataset, errorMessage);
}
