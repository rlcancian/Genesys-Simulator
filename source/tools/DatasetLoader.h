#ifndef DATASETLOADER_H
#define DATASETLOADER_H

#include <string>
#include <vector>

class DatasetLoader {
public:
    DatasetLoader() = default;
    virtual ~DatasetLoader() = default;

public:
    bool loadFromFile(
        const std::string& filename,
        char separator = ','
    );

    void clear();

public:
    bool isLoaded() const;
    bool isUsable() const;

    const std::vector<double>& data() const;
    const std::vector<double>& sortedData() const;

    std::size_t count() const;

    double min() const;
    double max() const;
    double mean() const;
    double variance() const;
    double stddev() const;

    bool hasNegativeData() const;

private:
    bool _loadDelimitedTextFile(
        const std::string& filename,
        char separator
    );

    bool _loadBinaryFile(
        const std::string& filename
    );

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