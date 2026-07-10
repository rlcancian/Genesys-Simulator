#ifndef WHOLECELLPARAMETERREADER_H
#define WHOLECELLPARAMETERREADER_H

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

/**
 * Minimal JSON reader for the CovertLab/WholeCell M. genitalium parameter files.
 *
 * Reads parameters.json and fixedConstants.json (MIT license) which have the
 * two-level structure:
 *   {"processes": {"ProcessName": {"key": value, ...}, ...},
 *    "states":    {"StateName":   {"key": value, ...}, ...}}
 *
 * Scalar double values are extracted into a flat map keyed by
 * "processes.ProcessName.key" or "states.StateName.key".
 *
 * Arrays and nested objects are skipped. Non-numeric leaf values are ignored.
 * No external JSON library is required.
 *
 * Usage:
 *   auto params = WholeCellParameterReader::read("data/parameters.json");
 *   double rate = params.count("processes.Transcription.rnaPolymeraseElongationRate")
 *                 ? params.at("processes.Transcription.rnaPolymeraseElongationRate")
 *                 : 50.0;
 */
class WholeCellParameterReader {
public:
	static std::map<std::string, double> read(const std::string& filePath) {
		std::map<std::string, double> result;
		std::ifstream file(filePath);
		if (!file.is_open()) return result;
		std::ostringstream oss;
		oss << file.rdbuf();
		const std::string json = oss.str();
		_parseTopLevel(json, result);
		return result;
	}

private:
	static void _skipWhitespace(const std::string& s, std::size_t& pos) {
		while (pos < s.size() && std::isspace(static_cast<unsigned char>(s[pos]))) ++pos;
	}

	static bool _parseString(const std::string& s, std::size_t& pos, std::string& out) {
		_skipWhitespace(s, pos);
		if (pos >= s.size() || s[pos] != '"') return false;
		++pos;
		out.clear();
		while (pos < s.size() && s[pos] != '"') {
			if (s[pos] == '\\') {
				++pos;
				if (pos < s.size()) { out += s[pos]; ++pos; }
			} else {
				out += s[pos++];
			}
		}
		if (pos < s.size()) ++pos;
		return true;
	}

	static bool _parseDouble(const std::string& s, std::size_t& pos, double& out) {
		_skipWhitespace(s, pos);
		const std::size_t start = pos;
		if (pos < s.size() && (s[pos] == '-' || s[pos] == '+')) ++pos;
		bool hasDigit = false;
		while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) { ++pos; hasDigit = true; }
		if (pos < s.size() && s[pos] == '.') {
			++pos;
			while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) { ++pos; hasDigit = true; }
		}
		if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
			++pos;
			if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
			while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
		}
		if (!hasDigit) return false;
		try { out = std::stod(s.substr(start, pos - start)); } catch (...) { return false; }
		return true;
	}

	static void _skipValue(const std::string& s, std::size_t& pos) {
		_skipWhitespace(s, pos);
		if (pos >= s.size()) return;
		if (s[pos] == '"') {
			std::string dummy; _parseString(s, pos, dummy);
		} else if (s[pos] == '{') {
			int depth = 1; ++pos;
			while (pos < s.size() && depth > 0) {
				if (s[pos] == '{') ++depth;
				else if (s[pos] == '}') --depth;
				else if (s[pos] == '"') { std::string d; _parseString(s, pos, d); continue; }
				++pos;
			}
		} else if (s[pos] == '[') {
			int depth = 1; ++pos;
			while (pos < s.size() && depth > 0) {
				if (s[pos] == '[') ++depth;
				else if (s[pos] == ']') --depth;
				else if (s[pos] == '"') { std::string d; _parseString(s, pos, d); continue; }
				++pos;
			}
		} else {
			while (pos < s.size() && s[pos] != ',' && s[pos] != '}' && s[pos] != ']') ++pos;
		}
	}

	// Parse {"key": value, ...} and call handler for each key-value pair
	static void _parseObject(const std::string& s, std::size_t& pos,
	                         const std::string& prefix,
	                         std::map<std::string, double>& out) {
		_skipWhitespace(s, pos);
		if (pos >= s.size() || s[pos] != '{') return;
		++pos;
		while (pos < s.size()) {
			_skipWhitespace(s, pos);
			if (pos >= s.size() || s[pos] == '}') { if (pos < s.size()) ++pos; break; }
			if (s[pos] == ',') { ++pos; continue; }
			std::string key;
			if (!_parseString(s, pos, key)) { ++pos; continue; }
			_skipWhitespace(s, pos);
			if (pos < s.size() && s[pos] == ':') ++pos;
			_skipWhitespace(s, pos);
			const std::string fullKey = prefix.empty() ? key : prefix + "." + key;
			if (pos < s.size() && s[pos] == '{') {
				// nested object — recurse one level
				_parseObject(s, pos, fullKey, out);
			} else if (pos < s.size() && s[pos] != '[' && s[pos] != '"') {
				// scalar value — try as double
				double val = 0.0;
				const std::size_t before = pos;
				if (_parseDouble(s, pos, val)) {
					out[fullKey] = val;
				} else {
					pos = before;
					_skipValue(s, pos);
				}
			} else {
				// array or string — skip
				_skipValue(s, pos);
			}
		}
	}

	static void _parseTopLevel(const std::string& json, std::map<std::string, double>& out) {
		std::size_t pos = 0;
		_skipWhitespace(json, pos);
		if (pos >= json.size() || json[pos] != '{') return;
		++pos;
		while (pos < json.size()) {
			_skipWhitespace(json, pos);
			if (json[pos] == '}') break;
			if (json[pos] == ',') { ++pos; continue; }
			std::string section;
			if (!_parseString(json, pos, section)) { ++pos; continue; }
			_skipWhitespace(json, pos);
			if (pos < json.size() && json[pos] == ':') ++pos;
			_parseObject(json, pos, section, out);
		}
	}
};

#endif /* WHOLECELLPARAMETERREADER_H */
