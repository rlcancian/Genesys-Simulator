#ifndef MODELSERIALIZER_H
#define MODELSERIALIZER_H

#include <iostream>
#include <functional>

#include "Persistence.h"
#include "../util/Util.h"


/*!
 * @brief Serialization/Deserialization interface for simulation models.
 * 
 * This can be seen as an associative array which knows how to ser/deserialize itself.
 * The "contents" of a serializer are the set of entries it contains.
 * 
 * Its entries are identified by strings, which correspond to unique component/datadef names.
 * Each entry has its own name, a type, an id, and an associated set of fields.
 * All of these are contained within a `PersistenceRecord` instance.
 * 
 * Serializers also act as factories for `PersistenceRecord`s, which can then only be used
 * with the serializer which originated them.
 */
class ModelSerializer {
public:
    virtual ~ModelSerializer();

    /*!
     * @brief Creates an empty persistence record.
     *
     * Although the returned record can be freed normally with `delete`, care must be taken to only use
     * records which come from a implementation, when configuring that implementation's contents.
     */
    virtual PersistenceRecord* newPersistenceRecord() = 0;

    /*!
     * @brief Writes formatted contents to a given output stream.
     * 
     * @param output output stream to write to.
     * 
     * @return whether the current state of this structure's contents was deemed valid for serialization.
     */
    virtual bool dump(std::ostream& output) = 0;
 
    /*!
     * @brief Attempts to load contents from a given input stream.
     * 
     * @return whether the input was well-formatted and loading was successful.
     */
    virtual bool load(std::istream& input) = 0;

    /*!
     * @brief Tries to fetch an entry with the given label as a key.
     * 
     * @param name a label which uniquely identifies an entry.
     * @param entry (out-param) a pointer to a caller-owned entry, which is filled on success.
     * 
     * @return whether the entry was found and the out-param was actually written to.
     */
    virtual bool get(const std::string& name, PersistenceRecord *entry) = 0;

    /*!
     * @brief Upserts an entry into this structure.
     * 
     * @param name a label which uniquely identifies the entry.
     * @param type a type name.
     * @param id a component/datadef ID number.
     * @param fields a persistence record for the entry, whose contents will be copied if valid.
     * 
     * @return whether the entry was valid and was actually copied into this structure.
     */
    virtual bool put(const std::string name, const std::string type, const Util::identification id, PersistenceRecord *fields) = 0;

    /*!
     * @brief Iterates over this structure's entries, ordered by ID.
     * 
     * @param delegate delegate which is applied to every entry-identifying key, until it returns a non-zero value.
     * 
     * @return either zero, or the first non-zero value returned by the delegate.
     */
    virtual int for_each(std::function<int(const std::string&)> delegate) = 0; 
};

#endif // MODELSERIALIZER_H
