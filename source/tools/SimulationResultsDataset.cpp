#include "tools/SimulationResultsDataset.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

namespace {
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

	static std::string quotedValueAfter(const std::string& line, const std::string& key) {
		const std::string lowerLine = lower(line);
		const std::string lowerKey = lower(key);
		const std::size_t keyPos = lowerLine.find(lowerKey);
		if (keyPos == std::string::npos) {
			return "";
		}
		const std::size_t equalsPos = line.find('=', keyPos + key.size());
		if (equalsPos == std::string::npos) {
			return "";
		}
		std::size_t valueStart = equalsPos + 1;
		while (valueStart < line.size() && std::isspace(static_cast<unsigned char>(line.at(valueStart))) != 0) {
			++valueStart;
		}
		if (valueStart >= line.size()) {
			return "";
		}
		if (line.at(valueStart) == '"') {
			const std::size_t valueEnd = line.find('"', valueStart + 1);
			if (valueEnd == std::string::npos) {
				return "";
			}
			return line.substr(valueStart + 1, valueEnd - valueStart - 1);
		}
		std::size_t valueEnd = valueStart;
		while (valueEnd < line.size() && line.at(valueEnd) != ',' && std::isspace(static_cast<unsigned char>(line.at(valueEnd))) == 0) {
			++valueEnd;
		}
		return line.substr(valueStart, valueEnd - valueStart);
	}

	static bool replicationFromComment(const std::string& line, unsigned int* replication) {
		const std::string lowerLine = lower(line);
		const std::string key = "replicationnumber";
		const std::size_t keyPos = lowerLine.find(key);
		if (keyPos == std::string::npos) {
			return false;
		}
		const std::size_t equalsPos = line.find('=', keyPos + key.size());
		if (equalsPos == std::string::npos) {
			return false;
		}
		char* end = nullptr;
		const unsigned long parsed = std::strtoul(line.c_str() + equalsPos + 1, &end, 10);
		if (end == line.c_str() + equalsPos + 1 || parsed == 0 || parsed > std::numeric_limits<unsigned int>::max()) {
			return false;
		}
		if (replication != nullptr) {
			*replication = static_cast<unsigned int>(parsed);
		}
		return true;
	}
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

	SimulationResultsDataset parsed;
	parsed.sourceDescription = fileName;
	unsigned int currentReplication = 1;
	std::string line;
	unsigned int lineNumber = 0;
	while (std::getline(file, line)) {
		++lineNumber;
		if (parsed.previewLines.size() < 20) {
			parsed.previewLines.push_back(line);
		}
		const std::string stripped = trim(line);
		if (stripped.empty()) {
			continue;
		}
		if (stripped.front() == '#') {
			parsed.recordFile = true;
			unsigned int replication = currentReplication;
			if (replicationFromComment(stripped, &replication)) {
				currentReplication = replication;
				continue;
			}
			const std::string expression = quotedValueAfter(stripped, "Expression");
			if (!expression.empty()) {
				parsed.expression = expression;
			}
			const std::string expressionName = quotedValueAfter(stripped, "ExpressionName");
			if (!expressionName.empty()) {
				parsed.expressionName = expressionName;
			}
			const std::string columns = lower(quotedValueAfter(stripped, "Columns"));
			if (columns.find("time") != std::string::npos) {
				parsed.timeDependent = true;
			}
			continue;
		}

		std::vector<double> numericTokens;
		for (const std::string& token : tokenizeDataLine(stripped)) {
			double value = 0.0;
			if (parseDoubleToken(token, &value)) {
				numericTokens.push_back(value);
			}
		}
		if (numericTokens.empty()) {
			continue;
		}

		SimulationResultsObservation observation;
		observation.replication = currentReplication;
		observation.sourceLine = lineNumber;
		observation.value = numericTokens.back();
		if (numericTokens.size() >= 2) {
			observation.time = numericTokens.at(numericTokens.size() - 2);
			observation.hasTime = true;
			parsed.timeDependent = true;
		}
		parsed.observations.push_back(observation);
	}

	if (parsed.observations.empty()) {
		if (errorMessage != nullptr) {
			*errorMessage = "The selected file does not contain numeric observations.";
		}
		return false;
	}

	*dataset = parsed;
	return true;
}
