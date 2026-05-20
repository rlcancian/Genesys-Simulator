#include "DatasetLoader.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <utility>

namespace {
	static void skipSpaces(const char*& cursor) {
		while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor)) != 0) {
			++cursor;
		}
	}

	static bool isWhitespaceSeparator(char separator) {
		return std::isspace(static_cast<unsigned char>(separator)) != 0;
	}

	static bool parseFiniteDouble(const char*& cursor, double* value) {
		skipSpaces(cursor);
		if (*cursor == '\0') {
			return false;
		}

		char* end = nullptr;
		errno = 0;
		const double parsed = std::strtod(cursor, &end);
		if (end == cursor || errno == ERANGE || !std::isfinite(parsed)) {
			return false;
		}
		cursor = end;
		if (value != nullptr) {
			*value = parsed;
		}
		return true;
	}

	static bool parseWhitespaceSeparatedLine(const char* cursor, std::vector<double>* values) {
		while (true) {
			skipSpaces(cursor);
			if (*cursor == '\0') {
				return true;
			}

			double value = 0.0;
			if (!parseFiniteDouble(cursor, &value)) {
				return false;
			}
			values->push_back(value);
		}
	}

	static bool parseDelimitedLine(const char* cursor, char separator, std::vector<double>* values) {
		while (true) {
			double value = 0.0;
			if (!parseFiniteDouble(cursor, &value)) {
				return false;
			}
			values->push_back(value);

			skipSpaces(cursor);
			if (*cursor == '\0') {
				return true;
			}
			if (*cursor != separator) {
				return false;
			}
			++cursor;
		}
	}
}

bool DatasetLoader::loadFromFile(const std::string& filename, char separator) {
	clear();
	if (filename.empty()) {
		return false;
	}

	if (_loadDelimitedTextFile(filename, separator) || _loadBinaryFile(filename)) {
		_loaded = true;
		_usable = !_data.empty();
		if (_usable) {
			_computeStatistics();
		}
		return _usable;
	}

	return false;
}

bool DatasetLoader::loadFromVector(const std::vector<double>& values) {
    clear();
    if (values.empty()) {
        _loaded = true;
        return false;
    }
    for (double v : values) {
        if (!std::isfinite(v)) {
            clear();
            return false;
        }
    }
    _data = values;
    _loaded = true;
    _usable = true;
    _computeStatistics();
    return true;
}

void DatasetLoader::clear() {
	_loaded = false;
	_usable = false;
	_data.clear();
	_sortedData.clear();
	_count = 0;
	_min = 0.0;
	_max = 0.0;
	_mean = 0.0;
	_variance = 0.0;
	_stddev = 0.0;
	_hasNegativeData = false;
}

bool DatasetLoader::isLoaded() const {
	return _loaded;
}

bool DatasetLoader::isUsable() const {
	return _usable;
}

const std::vector<double>& DatasetLoader::data() const {
	return _data;
}

const std::vector<double>& DatasetLoader::sortedData() const {
	return _sortedData;
}

std::size_t DatasetLoader::count() const {
	return _count;
}

double DatasetLoader::min() const {
	return _min;
}

double DatasetLoader::max() const {
	return _max;
}

double DatasetLoader::mean() const {
	return _mean;
}

double DatasetLoader::variance() const {
	return _variance;
}

double DatasetLoader::stddev() const {
	return _stddev;
}

bool DatasetLoader::hasNegativeData() const {
	return _hasNegativeData;
}

bool DatasetLoader::_loadDelimitedTextFile(const std::string& filename, char separator) {
	std::ifstream file(filename.c_str());
	if (!file.is_open()) {
		return false;
	}

	std::vector<double> values;
	values.reserve(1024);
	std::string line;
	while (std::getline(file, line)) {
		const char* cursor = line.c_str();
		skipSpaces(cursor);
		if (*cursor == '\0' || *cursor == '#') {
			continue;
		}

		const bool parsed = isWhitespaceSeparator(separator)
				? parseWhitespaceSeparatedLine(cursor, &values)
				: parseDelimitedLine(cursor, separator, &values);
		if (!parsed) {
			return false;
		}
	}

	if (values.empty()) {
		return false;
	}

	_data = std::move(values);
	return true;
}

bool DatasetLoader::_loadBinaryFile(const std::string& filename) {
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.good()) {
		return false;
	}

	const std::streamsize bytes = file.tellg();
	if (bytes <= 0 || (bytes % static_cast<std::streamsize>(sizeof(double))) != 0) {
		return false;
	}

	file.seekg(0, std::ios::beg);
	_data.assign(static_cast<std::size_t>(bytes / static_cast<std::streamsize>(sizeof(double))), 0.0);
	file.read(reinterpret_cast<char*>(_data.data()), bytes);
	if (!file || file.gcount() != bytes) {
		_data.clear();
		return false;
	}

	for (double value : _data) {
		if (!std::isfinite(value)) {
			_data.clear();
			return false;
		}
	}

	return true;
}

void DatasetLoader::_computeStatistics() {
	_count = _data.size();
	if (_count == 0) {
		_usable = false;
		return;
	}

	_min = _data.front();
	_max = _data.front();

	double mean = 0.0;
	double m2 = 0.0;
	std::size_t seen = 0;
	for (double value : _data) {
		_min = std::min(_min, value);
		_max = std::max(_max, value);

		++seen;
		const double delta = value - mean;
		mean += delta / static_cast<double>(seen);
		m2 += delta * (value - mean);
	}

	_mean = mean;
	_hasNegativeData = _min < 0.0;
	if (_count >= 2) {
		_variance = m2 / static_cast<double>(_count - 1U);
		_stddev = std::sqrt(_variance);
	} else {
		_variance = 0.0;
		_stddev = 0.0;
	}

	_sortedData = _data;
	std::sort(_sortedData.begin(), _sortedData.end());
}
