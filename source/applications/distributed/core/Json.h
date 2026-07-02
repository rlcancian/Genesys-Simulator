#pragma once

#include <optional>
#include <string>
#include <vector>

// Minimal, self-contained JSON helpers for the distributed layer. They are intentionally
// lightweight (scalar field reads via regex, array/object scanning by hand) and mirror the
// hand-rolled JSON approach already used by the web worker, avoiding any external dependency.
// They assume the well-formed responses produced by the GenESyS worker API, not arbitrary JSON.
namespace genesys::distributed::json {

/** @brief Reads a string field `"field":"..."`, unescaping `\"`, `\\` and `\/`. */
std::optional<std::string> getString(const std::string& body, const std::string& field);

/** @brief Reads a floating-point field `"field":<number>`. */
std::optional<double> getDouble(const std::string& body, const std::string& field);

/** @brief Reads an integer field `"field":<int>`. */
std::optional<long long> getInt(const std::string& body, const std::string& field);

/** @brief Reads a boolean field `"field":true|false`. */
std::optional<bool> getBool(const std::string& body, const std::string& field);

/**
 * @brief Returns the content between the matching brackets of `"field":[ ... ]`.
 * @details Bracket matching is string- and nesting-aware. The returned text excludes the
 * outer brackets. Returns nullopt when the field is missing or is not an array.
 */
std::optional<std::string> getArray(const std::string& body, const std::string& field);

/** @brief Splits an array body into its top-level `{...}` object substrings (braces included). */
std::vector<std::string> splitObjects(const std::string& arrayBody);

/** @brief Escapes `"` and `\` for embedding a string into a JSON document. */
std::string escape(const std::string& value);

/** @brief Formats a double with full round-trip precision (17 significant digits). */
std::string number(double value);

} // namespace genesys::distributed::json
