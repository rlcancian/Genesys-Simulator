#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <string>
#include <utility>
#include <unordered_map>

#include "../util/Util.h"
#include "ModelPersistence_if.h"


/*!
 * @brief Serialization/Deserialization structure which behaves like an associative array.
 *
 * Simulation models/components/definitions use this as a "visitor", which is guided
 * across their innards during ser/deserialization and filled with fields. See `PersistentObject_base`.
 * 
 * When this structure reaches the main model persistence interface, it finishes
 * the serialization process in a format-specific manner. See `ModelPersistence_if` and `ModelSerializer`.
 */
class PersistenceRecord {
public:
	using Payload = std::string;
	//! "Raw" entry, users should treat this as an opaque type since payloads could be in any format.
	struct Entry {
		enum Kind { text, numeric };
		std::string first;
		Payload second;
		Kind kind{Kind::text};
	};
	using Iterator = std::unordered_map<std::string, Entry>::iterator;

	// this is the actual abstract interface

	virtual ~PersistenceRecord();

	virtual PersistenceRecord* newInstance();

	virtual std::string loadField(std::string key, std::string defaultValue = "");
	virtual double loadField(std::string key, double defaultValue);
	virtual unsigned int loadField(std::string key, unsigned int defaultValue);
	virtual int loadField(std::string key, int defaultValue);
	virtual Util::TimeUnit loadField(std::string key, Util::TimeUnit defaultValue);

	virtual void saveField(std::string key, std::string value, const std::string defaultValue, const bool saveIfDefault = false);
	virtual void saveField(std::string key, std::string value);
	virtual void saveField(std::string key, double value, const double defaultValue, const bool saveIfDefault = false);
	virtual void saveField(std::string key, unsigned int value, const unsigned int defaultValue, const bool saveIfDefault = false);
	virtual void saveField(std::string key, unsigned int value);
	virtual void saveField(std::string key, int value, const int defaultValue, const bool saveIfDefault = false);
	virtual void saveField(std::string key, Util::TimeUnit value, const Util::TimeUnit defaultValue, const bool saveIfDefault = false);

	// these are only needed because existing code does not abstract away the STL container API
	// still, subclasses implementing this interface may use them to insert post-processed entries

	explicit PersistenceRecord(ModelPersistence_if& config);

	//! Returns the number of distinct entries in the record.
	std::size_t size() const;

	//! Inserts a raw entry in the record.
	void insert(Entry entry);

	//! Inserts multiple entries from the given iterators.
	void insert(Iterator it, Iterator end);

	//! Removes a specific entry from the record.
	void erase(const std::string& key);

	//! Removes all entries from this record.
	void clear();

	//! Returns an iterator over a specific entry, if found.
	Iterator find(const std::string& key);

	//! Returns an iterator over raw entries.
	Iterator begin();

	//! Past-the-end sentinel iterator.
	Iterator end();

private:
	std::unordered_map<std::string, Entry> _fields;
	ModelPersistence_if& _config;
};


/*!
 * @brief Dynamic serializable interface.
 *
 * Any object that's supposed to be saved to and/or loaded from a file must implement this interface.
 */
class PersistentObject_base {
public:
	virtual ~PersistentObject_base() {};

protected:
	/*!
	 * @brief Loads object fields from a deserialization structure.
	 * 
	 * @param fields a table-like structure which can be queried for parsed fields.
	 * 
	 * @return a boolean which is true iff the load operation was successful.
	 */
	virtual bool _loadInstance(PersistenceRecord *fields) = 0;

	/*!
	 * @brief Saves object fields into a serialization structure.
	 * 
	 * @param fields a table-like structure used for serialization.
	 * @param saveDefaultValues indicates whether or not to save fields which are set to their default value.
	 */
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues = false) = 0;
};

#endif /* PERSISTENCE_H */
