#ifndef DATASET_IF_H
#define DATASET_IF_H

#include <string>

/**
 * @brief Abstract view of a numeric dataset.
 *
 * Purpose:
 * - Represent sample data loaded in memory or abstracted from persistent source.
 *
 * Ownership/lifetime:
 * - Implementations own storage/source handles; clients use this interface as a
 *   non-owning access contract.
 */
class DataSet_if {
public:
	virtual ~DataSet_if() = default;
	virtual unsigned int size() const = 0;
	virtual double value(unsigned int index) const = 0;
	virtual bool isLoaded() const = 0;
	virtual std::string sourceDescription() const = 0;
};

#endif /* DATASET_IF_H */
