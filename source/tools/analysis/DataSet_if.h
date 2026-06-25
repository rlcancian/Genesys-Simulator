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
	/** @brief Destroys the dataset interface. */
	virtual ~DataSet_if() = default;
	/** @brief Returns the number of observations exposed by the dataset. */
	virtual unsigned int size() const = 0;
	/** @brief Returns the observation at a zero-based index. */
	virtual double value(unsigned int index) const = 0;
	/** @brief Returns whether the dataset source is currently loaded. */
	virtual bool isLoaded() const = 0;
	/** @brief Returns a human-readable description of the dataset source. */
	virtual std::string sourceDescription() const = 0;
};

#endif /* DATASET_IF_H */
