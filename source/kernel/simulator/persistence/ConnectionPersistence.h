#pragma once

#include "../Persistence.h"

#include <algorithm>
#include <charconv>
#include <string>
#include <vector>

inline std::vector<unsigned int> connectionOutputPorts(PersistenceRecord& record) {
    std::vector<unsigned int> ports;
    const std::string prefix = "nextId[";
    for (auto it = record.begin(); it != record.end(); ++it) {
        const std::string& key = it->first;
        if (!key.starts_with(prefix) || key.size() <= prefix.size() + 1 || key.back() != ']') continue;
        unsigned int port = 0;
        const char* begin = key.data() + prefix.size();
        const char* end = key.data() + key.size() - 1;
        const auto result = std::from_chars(begin, end, port);
        if (result.ec == std::errc{} && result.ptr == end) ports.push_back(port);
    }
    std::sort(ports.begin(), ports.end());
    if (ports.empty() && record.find("nextId") != record.end()) ports.push_back(0);
    return ports;
}
