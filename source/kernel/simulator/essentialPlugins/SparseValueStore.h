/*
 * File:   SparseValueStore.h
 * Author: rafael.luiz.cancian
 *
 * Created on 16 de Abril de 2026
 */

#ifndef SPARSEVALUESTORE_H
#define SPARSEVALUESTORE_H

#include "../Persistence.h"
#include "../../util/Util.h"

#include <list>
#include <map>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

/*!
 * \brief Sparse N-dimensional numeric store using GenESyS textual index keys.
 *
 * The store preserves the existing Variable semantics: the scalar position is
 * represented by an empty key, multidimensional indexes are represented as
 * comma-separated unsigned integers such as "1,2,3", and any missing position
 * reads back as 0.0. It is intentionally small so Variable, Attribute and Entity
 * can share the same behavior without changing the persistence format.
 */
class SparseValueStore {
public:
	/*! \brief Builds the textual key used by legacy sparse matrices from index values. */
	static std::string makeIndexKey(const std::vector<unsigned int>& indexes);

	/*! \brief Builds the textual key used by legacy sparse matrices from expression values. */
	static std::string makeIndexKeyFromDoubles(const std::vector<double>& indexes);

	/*! \brief Appends one index to an existing textual sparse key. */
	static std::string appendIndexKey(const std::string& currentKey, unsigned int index);

	/*! \brief Appends one expression value to an existing textual sparse key. */
	static std::string appendIndexKeyFromDouble(const std::string& currentKey, double index);

	/*! \brief Returns a normalized copy of an already textual key. */
	static std::string normalizeIndexKey(const std::string& index);

	/*! \brief Reads a sparse position, returning 0.0 when the position is absent. */
	double value(const std::string& index = "") const;

	/*! \brief Writes a sparse position, creating it when needed. */
	void setValue(double value, const std::string& index = "");

	/*! \brief Replaces all sparse values with a copy of the provided values. */
	void setValues(const std::map<std::string, double>& values);

	/*! \brief Inserts one dimension size at the end of the dimension list. */
	void insertDimensionSize(unsigned int size);

	/*! \brief Returns mutable dimension sizes for compatibility with legacy callers. */
	std::list<unsigned int>* dimensionSizes();

	/*! \brief Returns immutable dimension sizes. */
	const std::list<unsigned int>* dimensionSizes() const;

	/*! \brief Returns mutable sparse values for compatibility with legacy callers. */
	std::map<std::string, double>* values();

	/*! \brief Returns immutable sparse values. */
	const std::map<std::string, double>* values() const;

	/*! \brief Clears dimensions and values. */
	void clear();

	/*! \brief Loads dimensions using the legacy fields dimensions/dimension[i]. */
	void loadDimensions(PersistenceRecord* fields);

	/*! \brief Saves dimensions using the legacy fields dimensions/dimension[i]. */
	void saveDimensions(PersistenceRecord* fields, bool saveDefaultValues) const;

	/*! \brief Loads sparse values using the provided count/position/value field prefixes. */
	void loadValues(PersistenceRecord* fields,
	                const std::string& countField,
	                const std::string& positionPrefix,
	                const std::string& valuePrefix);

	/*! \brief Saves sparse values using the provided count/position/value field prefixes. */
	void saveValues(PersistenceRecord* fields,
	                const std::string& countField,
	                const std::string& positionPrefix,
	                const std::string& valuePrefix,
	                bool saveDefaultValues) const;

	/*! \brief Returns a compact textual representation used by show/debug paths. */
	std::string showValues() const;

private:
	static std::string trimCopy(const std::string& text) {
		std::size_t first = 0;
		while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) {
			++first;
		}
		std::size_t last = text.size();
		while (last > first && std::isspace(static_cast<unsigned char>(text[last - 1]))) {
			--last;
		}
		return text.substr(first, last - first);
	}

	// Returns true when every comma-separated token in index consists of digits only
	// (or when index is empty, representing the scalar position).
	static bool isNumericKey(const std::string& index) {
		if (index.empty()) {
			return true;
		}
		for (char c : index) {
			if (!std::isdigit(static_cast<unsigned char>(c)) && c != ',' && c != ' ' && c != '\t') {
				return false;
			}
		}
		return true;
	}

	static std::vector<unsigned int> parseIndex(const std::string& index) {
		std::vector<unsigned int> parsed;
		if (index.empty()) {
			return parsed;
		}
		std::stringstream input(index);
		std::string token;
		while (std::getline(input, token, ',')) {
			token = trimCopy(token);
			if (token.empty()) {
				continue;
			}
			parsed.push_back(static_cast<unsigned int>(std::stoul(token)));
		}
		return parsed;
	}

	static std::size_t productOfDimensions(const std::list<unsigned int>& dimensions) {
		std::size_t product = 1u;
		for (unsigned int dimension : dimensions) {
			product *= static_cast<std::size_t>(dimension);
		}
		return product;
	}

	static std::vector<std::size_t> stridesFor(const std::list<unsigned int>& dimensions) {
		std::vector<std::size_t> strides;
		strides.reserve(dimensions.size());
		std::size_t stride = 1u;
		for (auto it = dimensions.rbegin(); it != dimensions.rend(); ++it) {
			strides.push_back(stride);
			stride *= static_cast<std::size_t>(*it);
		}
		std::reverse(strides.begin(), strides.end());
		return strides;
	}

	std::size_t linearIndex(const std::vector<unsigned int>& indexes) const {
		if (_dimensionSizes.empty()) {
			return indexes.empty() ? 0u : static_cast<std::size_t>(-1);
		}
		if (indexes.size() != _dimensionSizes.size()) {
			return static_cast<std::size_t>(-1);
		}
		const std::vector<std::size_t> strides = stridesFor(_dimensionSizes);
		std::size_t linear = 0u;
		auto dimIt = _dimensionSizes.begin();
		for (std::size_t i = 0; i < indexes.size(); ++i, ++dimIt) {
			if (indexes[i] >= *dimIt) {
				return static_cast<std::size_t>(-1);
			}
			linear += static_cast<std::size_t>(indexes[i]) * strides[i];
		}
		return linear;
	}

	void rebuildDenseFromCache() {
		const std::size_t size = _dimensionSizes.empty() ? 1u : productOfDimensions(_dimensionSizes);
		_denseValues.assign(size, 0.0);
		for (const auto& entry : _values) {
			if (entry.first.empty()) {
				continue;
			}
			// Skip non-numeric string keys — they live only in the sparse map.
			if (!isNumericKey(entry.first)) {
				continue;
			}
			const std::vector<unsigned int> indexes = parseIndex(entry.first);
			const std::size_t linear = linearIndex(indexes);
			if (linear != static_cast<std::size_t>(-1) && linear < _denseValues.size()) {
				_denseValues[linear] = entry.second;
			}
		}
		_denseActive = true;
	}

	void syncCacheFromDense() {
		// Preserve non-numeric string keys across the rebuild.
		std::map<std::string, double> stringKeyed;
		for (const auto& entry : _values) {
			if (!entry.first.empty() && !isNumericKey(entry.first)) {
				stringKeyed.insert(entry);
			}
		}
		_values.clear();
		if (_hasScalarValue) {
			_values[""] = _scalarValue;
		}
		if (!_denseActive) {
			_values.insert(stringKeyed.begin(), stringKeyed.end());
			return;
		}
		if (_dimensionSizes.empty()) {
			if (!_denseValues.empty() && !_hasScalarValue) {
				_values[""] = _denseValues.front();
			}
			_values.insert(stringKeyed.begin(), stringKeyed.end());
			return;
		}
		std::vector<unsigned int> indexes;
		indexes.reserve(_dimensionSizes.size());
		const std::vector<std::size_t> strides = stridesFor(_dimensionSizes);
		for (std::size_t linear = 0; linear < _denseValues.size(); ++linear) {
			if (_denseValues[linear] == 0.0) {
				continue;
			}
			std::size_t remainder = linear;
			indexes.clear();
			for (std::size_t i = 0; i < strides.size(); ++i) {
				const unsigned int index = static_cast<unsigned int>(remainder / strides[i]);
				indexes.push_back(index);
				remainder %= strides[i];
			}
			_values[makeIndexKey(indexes)] = _denseValues[linear];
		}
		_values.insert(stringKeyed.begin(), stringKeyed.end());
	}

	void ensureDenseForIndex(const std::vector<unsigned int>& indexes) {
		if (indexes.empty() && _dimensionSizes.empty()) {
			if (!_denseActive) {
				_denseActive = true;
				_denseValues.assign(1u, 0.0);
			}
			return;
		}

		if (_dimensionSizes.empty()) {
			_dimensionSizes.clear();
			for (unsigned int index : indexes) {
				_dimensionSizes.push_back(index + 1u);
			}
			rebuildDenseFromCache();
			return;
		}

		if (indexes.size() != _dimensionSizes.size()) {
			return;
		}

		bool needsResize = false;
		auto dimIt = _dimensionSizes.begin();
		for (std::size_t i = 0; i < indexes.size(); ++i, ++dimIt) {
			if (indexes[i] >= *dimIt) {
				*dimIt = indexes[i] + 1u;
				needsResize = true;
			}
		}
		if (needsResize || !_denseActive) {
			rebuildDenseFromCache();
		}
	}

	bool _denseActive = false;
	bool _hasScalarValue = false;
	double _scalarValue = 0.0;
	std::vector<double> _denseValues;
	std::list<unsigned int> _dimensionSizes;
	std::map<std::string, double> _values;
};

inline std::string SparseValueStore::makeIndexKey(const std::vector<unsigned int>& indexes) {
	std::ostringstream key;
	bool first = true;
	for (const unsigned int index : indexes) {
		if (!first) {
			key << ",";
		}
		first = false;
		key << index;
	}
	return key.str();
}

inline std::string SparseValueStore::makeIndexKeyFromDoubles(const std::vector<double>& indexes) {
	std::vector<unsigned int> unsignedIndexes;
	unsignedIndexes.reserve(indexes.size());
	for (const double index : indexes) {
		unsignedIndexes.push_back(static_cast<unsigned int>(index));
	}
	return makeIndexKey(unsignedIndexes);
}

inline std::string SparseValueStore::appendIndexKey(const std::string& currentKey, unsigned int index) {
	const std::string nextPart = std::to_string(index);
	if (currentKey.empty()) {
		return nextPart;
	}
	return currentKey + "," + nextPart;
}

inline std::string SparseValueStore::appendIndexKeyFromDouble(const std::string& currentKey, double index) {
	return appendIndexKey(currentKey, static_cast<unsigned int>(index));
}

inline std::string SparseValueStore::normalizeIndexKey(const std::string& index) {
	return index;
}

inline double SparseValueStore::value(const std::string& index) const {
	if (index.empty() && _hasScalarValue) {
		return _scalarValue;
	}
	// Non-numeric string keys (e.g. "slot", "pi") bypass dense storage.
	if (!index.empty() && !isNumericKey(index)) {
		const auto it = _values.find(index);
		return (it != _values.end()) ? it->second : 0.0;
	}
	if (_denseActive) {
		const std::vector<unsigned int> indexes = parseIndex(index);
		const std::size_t linear = linearIndex(indexes);
		if (linear == static_cast<std::size_t>(-1) || linear >= _denseValues.size()) {
			// Dimension count mismatch or out of bounds — fall back to sparse map.
			const auto it = _values.find(normalizeIndexKey(index));
			return (it != _values.end()) ? it->second : 0.0;
		}
		return _denseValues[linear];
	}
	const auto it = _values.find(normalizeIndexKey(index));
	if (it == _values.end()) {
		return 0.0;
	}
	return it->second;
}

inline void SparseValueStore::setValue(double value, const std::string& index) {
	if (index.empty()) {
		_scalarValue = value;
		_hasScalarValue = true;
		_values[""] = value;
		if (_denseActive && _dimensionSizes.empty()) {
			if (_denseValues.empty()) {
				_denseValues.assign(1u, 0.0);
			}
			_denseValues.front() = value;
		}
		return;
	}
	// Non-numeric string keys (e.g. "slot", "pi") are stored directly in the
	// sparse map without touching the dense storage or dimension metadata.
	if (!isNumericKey(index)) {
		_values[index] = value;
		return;
	}
	const std::vector<unsigned int> indexes = parseIndex(index);
	ensureDenseForIndex(indexes);
	if (_denseActive) {
		const std::size_t linear = linearIndex(indexes);
		if (linear != static_cast<std::size_t>(-1)) {
			if (linear >= _denseValues.size()) {
				_denseValues.resize(linear + 1u, 0.0);
			}
			_denseValues[linear] = value;
			syncCacheFromDense();
			return;
		}
	}
	_values[normalizeIndexKey(index)] = value;
}

inline void SparseValueStore::setValues(const std::map<std::string, double>& values) {
	_values = values;
	const auto scalarIt = _values.find("");
	_hasScalarValue = scalarIt != _values.end();
	if (_hasScalarValue) {
		_scalarValue = scalarIt->second;
	}
	if (_dimensionSizes.empty()) {
		if (_values.size() == 1u && _hasScalarValue) {
			_denseActive = true;
			_denseValues.assign(1u, _scalarValue);
		} else {
			_denseActive = false;
			_denseValues.clear();
		}
		return;
	}
	rebuildDenseFromCache();
}

inline void SparseValueStore::insertDimensionSize(unsigned int size) {
	_dimensionSizes.insert(_dimensionSizes.end(), size);
	rebuildDenseFromCache();
}

inline std::list<unsigned int>* SparseValueStore::dimensionSizes() {
	return &_dimensionSizes;
}

inline const std::list<unsigned int>* SparseValueStore::dimensionSizes() const {
	return &_dimensionSizes;
}

inline std::map<std::string, double>* SparseValueStore::values() {
	return &_values;
}

inline const std::map<std::string, double>* SparseValueStore::values() const {
	return &_values;
}

inline void SparseValueStore::clear() {
	_dimensionSizes.clear();
	_values.clear();
	_hasScalarValue = false;
	_scalarValue = 0.0;
	_denseValues.clear();
	_denseActive = false;
}

inline void SparseValueStore::loadDimensions(PersistenceRecord* fields) {
	_dimensionSizes.clear();
	if (fields == nullptr) {
		return;
	}
	const unsigned int count = fields->loadField("dimensions", 0u);
	for (unsigned int i = 0; i < count; i++) {
		insertDimensionSize(fields->loadField("dimension" + Util::StrIndex(i), 0u));
	}
	if (!_dimensionSizes.empty()) {
		rebuildDenseFromCache();
	}
}

inline void SparseValueStore::saveDimensions(PersistenceRecord* fields, bool saveDefaultValues) const {
	if (fields == nullptr) {
		return;
	}
	unsigned int i = 0;
	fields->saveField("dimensions", static_cast<unsigned int>(_dimensionSizes.size()), 0u, saveDefaultValues);
	for (const unsigned int dimension : _dimensionSizes) {
		fields->saveField("dimension" + Util::StrIndex(i), dimension, 1u, saveDefaultValues);
		i++;
	}
}

inline void SparseValueStore::loadValues(PersistenceRecord* fields,
                                         const std::string& countField,
                                         const std::string& positionPrefix,
                                         const std::string& valuePrefix) {
	_values.clear();
	if (fields == nullptr) {
		return;
	}
	const unsigned int count = fields->loadField(countField, 0u);
	for (unsigned int i = 0; i < count; i++) {
		const std::string position = fields->loadField(positionPrefix + Util::StrIndex(i), "");
		const double loadedValue = fields->loadField(valuePrefix + Util::StrIndex(i), 0.0);
		setValue(loadedValue, position);
	}
}

inline void SparseValueStore::saveValues(PersistenceRecord* fields,
                                         const std::string& countField,
                                         const std::string& positionPrefix,
                                         const std::string& valuePrefix,
                                         bool saveDefaultValues) const {
	if (fields == nullptr) {
		return;
	}
	unsigned int i = 0;
	fields->saveField(countField, static_cast<unsigned int>(_values.size()), 0u, saveDefaultValues);
	for (const auto& value : _values) {
		fields->saveField(positionPrefix + Util::StrIndex(i), value.first, "0", saveDefaultValues);
		fields->saveField(valuePrefix + Util::StrIndex(i), value.second, 0.0, saveDefaultValues);
		i++;
	}
}

inline std::string SparseValueStore::showValues() const {
	if (_denseActive && _values.empty() && !_denseValues.empty() && _dimensionSizes.empty()) {
		return "{" + std::to_string(_denseValues.front()) + "}";
	}
	std::string text = "{";
	for (const auto& value : _values) {
		text += value.first + "=" + Util::StrTruncIfInt(std::to_string(value.second)) + ", ";
	}
	if (!_values.empty()) {
		text = text.substr(0, text.length() - 2);
	}
	text += "}";
	return text;
}

#endif /* SPARSEVALUESTORE_H */
