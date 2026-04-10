#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::unordered_map<std::string, std::string> headers;

    [[nodiscard]] std::string header(const std::string& name) const {
        std::string normalized = name;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        auto it = headers.find(normalized);
        if (it == headers.end()) {
            return {};
        }
        return it->second;
    }
};
