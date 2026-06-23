#include "Json.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <regex>
#include <sstream>

namespace genesys::distributed::json {

namespace {

/// Unescapes the minimal set of JSON escapes produced by the worker (`\"`, `\\`, `\/`).
std::string unescape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[i + 1];
            if (next == '"' || next == '\\' || next == '/') {
                out.push_back(next);
                ++i;
                continue;
            }
        }
        out.push_back(value[i]);
    }
    return out;
}

} // namespace

std::optional<std::string> getString(const std::string& body, const std::string& field) {
    const std::regex pattern("\"" + field + "\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return unescape(match[1].str());
}

std::optional<double> getDouble(const std::string& body, const std::string& field) {
    const std::regex pattern("\"" + field + "\"\\s*:\\s*(-?(?:[0-9]+(?:\\.[0-9]+)?|\\.[0-9]+)(?:[eE][+-]?[0-9]+)?)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return std::strtod(match[1].str().c_str(), nullptr);
}

std::optional<long long> getInt(const std::string& body, const std::string& field) {
    const std::regex pattern("\"" + field + "\"\\s*:\\s*(-?[0-9]+)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return std::strtoll(match[1].str().c_str(), nullptr, 10);
}

std::optional<bool> getBool(const std::string& body, const std::string& field) {
    const std::regex pattern("\"" + field + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (!std::regex_search(body, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }
    return match[1].str() == "true";
}

std::optional<std::string> getArray(const std::string& body, const std::string& field) {
    const std::string key = "\"" + field + "\"";
    std::size_t pos = body.find(key);
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    pos = body.find(':', pos + key.size());
    if (pos == std::string::npos) {
        return std::nullopt;
    }
    ++pos;
    while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos])) != 0) {
        ++pos;
    }
    if (pos >= body.size() || body[pos] != '[') {
        return std::nullopt;
    }

    int depth = 0;
    bool inString = false;
    bool escaped = false;
    const std::size_t start = pos + 1;
    for (std::size_t i = pos; i < body.size(); ++i) {
        const char character = body[i];
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (character == '\\') {
                escaped = true;
            } else if (character == '"') {
                inString = false;
            }
            continue;
        }
        if (character == '"') {
            inString = true;
        } else if (character == '[') {
            ++depth;
        } else if (character == ']') {
            --depth;
            if (depth == 0) {
                return body.substr(start, i - start);
            }
        }
    }
    return std::nullopt;
}

std::vector<std::string> splitObjects(const std::string& arrayBody) {
    std::vector<std::string> objects;
    int depth = 0;
    bool inString = false;
    bool escaped = false;
    std::size_t start = std::string::npos;
    for (std::size_t i = 0; i < arrayBody.size(); ++i) {
        const char character = arrayBody[i];
        if (inString) {
            if (escaped) {
                escaped = false;
            } else if (character == '\\') {
                escaped = true;
            } else if (character == '"') {
                inString = false;
            }
            continue;
        }
        if (character == '"') {
            inString = true;
        } else if (character == '{') {
            if (depth == 0) {
                start = i;
            }
            ++depth;
        } else if (character == '}') {
            --depth;
            if (depth == 0 && start != std::string::npos) {
                objects.push_back(arrayBody.substr(start, i - start + 1));
                start = std::string::npos;
            }
        }
    }
    return objects;
}

std::string escape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (const char character : value) {
        if (character == '"' || character == '\\') {
            out.push_back('\\');
        }
        out.push_back(character);
    }
    return out;
}

std::string number(double value) {
    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    return stream.str();
}

} // namespace genesys::distributed::json
