/*
 * File:   SparseValueStore.h
 * Author: rafael.luiz.cancian
 *
 * Created on 16 de Abril de 2026
 */

#ifndef SPARSEVALUESTORE_H
#define SPARSEVALUESTORE_H

#include "Persistence.h"
#include "../util/Util.h"

#include <list>
#include <map>
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
	const auto it = _values.find(normalizeIndexKey(index));
	if (it == _values.end()) {
		return 0.0;
	}
	return it->second;
}

inline void SparseValueStore::setValue(double value, const std::string& index) {
	_values[normalizeIndexKey(index)] = value;
}

inline void SparseValueStore::setValues(const std::map<std::string, double>& values) {
	_values = values;
}

inline void SparseValueStore::insertDimensionSize(unsigned int size) {
	_dimensionSizes.insert(_dimensionSizes.end(), size);
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
