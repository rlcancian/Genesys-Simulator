#ifndef DATASETLOADER_H
#define DATASETLOADER_H

#include <string>
#include <vector>

/**
 * @brief Small, kernel-independent loader for numeric datasets.
 *
 * DatasetLoader accepts plain text and binary double files, validates finite
 * values and keeps both the original and sorted samples. It intentionally owns
 * only descriptive statistics needed by tools/analysis; simulation-specific
 * metadata is handled by SimulationResultsDataset.
 */
class DatasetLoader {
public:
    /** @brief Creates an empty loader. */
    DatasetLoader() = default;
    /** @brief Destroys the loader without owning external resources. */
    virtual ~DatasetLoader() = default;

public:
    /**
     * @brief Loads a delimited text file, falling back to binary double data.
     *
     * Empty lines and '#' comment lines are ignored in text mode. The method
     * returns false when the file cannot be parsed as a finite numeric dataset.
     */
    bool loadFromFile(
        const std::string& filename,
        char separator = ','
    );

    /**
     * @brief Loads a validated in-memory dataset.
     */
    bool loadFromVector(const std::vector<double>& values);

    /** @brief Resets loaded data, sorted data and computed statistics. */
    void clear();

public:
    /** @brief Returns whether a load attempt produced a loaded state. */
    bool isLoaded() const;
    /** @brief Returns whether the current dataset can be analyzed. */
    bool isUsable() const;

    /** @brief Returns values in their original input order. */
    const std::vector<double>& data() const;
    /** @brief Returns values sorted in nondecreasing order. */
    const std::vector<double>& sortedData() const;

    /** @brief Returns the number of numeric observations. */
    std::size_t count() const;

    /** @brief Returns the minimum observation. */
    double min() const;
    /** @brief Returns the maximum observation. */
    double max() const;
    /** @brief Returns the arithmetic sample mean. */
    double mean() const;
    /** @brief Returns the sample variance using n - 1 when possible. */
    double variance() const;
    /** @brief Returns the sample standard deviation. */
    double stddev() const;

    /** @brief Returns whether any observation is negative. */
    bool hasNegativeData() const;

private:
    /** @brief Attempts to parse a text file using the requested separator. */
    bool _loadDelimitedTextFile(
        const std::string& filename,
        char separator
    );

    /** @brief Attempts to parse a binary file containing raw double values. */
    bool _loadBinaryFile(
        const std::string& filename
    );

    /** @brief Computes sorted data and descriptive statistics for current data. */
    void _computeStatistics();

private:
    bool _loaded = false;
    bool _usable = false;

    std::vector<double> _data;
    std::vector<double> _sortedData;

    std::size_t _count = 0;

    double _min = 0.0;
    double _max = 0.0;
    double _mean = 0.0;
    double _variance = 0.0;
    double _stddev = 0.0;

    bool _hasNegativeData = false;
};

#endif
