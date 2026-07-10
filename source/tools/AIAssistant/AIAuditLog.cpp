/*
 * File:   AIAuditLog.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "AIAuditLog.h"

#include <cstdlib>   // std::getenv
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

// ---------------------------------------------------------------------------
// Timestamp helpers (UTC, ISO 8601)
// ---------------------------------------------------------------------------

static std::string timestampToIso8601(
        const std::chrono::system_clock::time_point& tp) {
    const std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm gmt{};
    ::gmtime_r(&t, &gmt);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &gmt);
    return buf;
}

static std::chrono::system_clock::time_point iso8601ToTimestamp(
        const std::string& s) {
    std::tm gmt{};
    if (s.size() >= 19) {
        gmt.tm_year = std::stoi(s.substr(0, 4))  - 1900;
        gmt.tm_mon  = std::stoi(s.substr(5, 2))  - 1;
        gmt.tm_mday = std::stoi(s.substr(8, 2));
        gmt.tm_hour = std::stoi(s.substr(11, 2));
        gmt.tm_min  = std::stoi(s.substr(14, 2));
        gmt.tm_sec  = std::stoi(s.substr(17, 2));
    }
    const std::time_t t = ::timegm(&gmt);
    return std::chrono::system_clock::from_time_t(t);
}

// ---------------------------------------------------------------------------
// Minimal JSON helpers (no external library)
// ---------------------------------------------------------------------------

static std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (const char c : s) {
        if      (c == '"')  { out += "\\\""; }
        else if (c == '\\') { out += "\\\\"; }
        else if (c == '\n') { out += "\\n";  }
        else if (c == '\r') { out += "\\r";  }
        else if (c == '\t') { out += "\\t";  }
        else                { out += c;       }
    }
    return out;
}

static std::string jsonExtractString(const std::string& json,
                                     const std::string& key) {
    const std::string needle = "\"" + key + "\":\"";
    const auto pos = json.find(needle);
    if (pos == std::string::npos) return {};
    const auto start = pos + needle.size();
    std::string val;
    bool escaped = false;
    for (auto i = start; i < json.size(); ++i) {
        const char c = json[i];
        if (escaped) {
            if      (c == '"')  val += '"';
            else if (c == '\\') val += '\\';
            else if (c == 'n')  val += '\n';
            else if (c == 'r')  val += '\r';
            else if (c == 't')  val += '\t';
            else                val += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '"') {
            break;
        } else {
            val += c;
        }
    }
    return val;
}

static bool jsonExtractBool(const std::string& json, const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const auto pos = json.find(needle);
    if (pos == std::string::npos) return false;
    const auto start = pos + needle.size();
    return json.compare(start, 4, "true") == 0;
}

static long long jsonExtractLongLong(const std::string& json,
                                     const std::string& key) {
    const std::string needle = "\"" + key + "\":";
    const auto pos = json.find(needle);
    if (pos == std::string::npos) return 0;
    try {
        return std::stoll(json.substr(pos + needle.size()));
    } catch (...) {
        return 0;
    }
}

// ---------------------------------------------------------------------------
// AIAuditLog
// ---------------------------------------------------------------------------

AIAuditLog::AIAuditLog(const std::string& logFilePath)
    : _logFilePath(logFilePath.empty() ? defaultLogFilePath() : logFilePath) {
}

std::string AIAuditLog::defaultLogFilePath() {
    const char* xdg = std::getenv("XDG_DATA_HOME");
    std::string base;
    if (xdg && xdg[0] != '\0') {
        base = xdg;
    } else {
        const char* home = std::getenv("HOME");
        base = (home != nullptr) ? std::string(home) + "/.local/share" : "/tmp";
    }
    return base + "/genesys/ai_audit.log";
}

void AIAuditLog::_ensureDirectoryExists() const {
    const auto slash = _logFilePath.rfind('/');
    if (slash == std::string::npos) return;
    const std::string dir = _logFilePath.substr(0, slash);

    // Walk each prefix and create missing directories (equivalent to mkdir -p).
    std::string partial;
    for (const char c : dir) {
        if (c == '/' && !partial.empty()) {
            ::mkdir(partial.c_str(), 0755); // Ignore EEXIST.
        }
        partial += c;
    }
    ::mkdir(partial.c_str(), 0755);
}

std::string AIAuditLog::_encodeEntry(const AuditEntry& entry) {
    std::ostringstream os;
    os << "{"
       << "\"ts\":\""  << jsonEscape(timestampToIso8601(entry.timestamp)) << "\","
       << "\"op\":\""  << jsonEscape(entry.operation)     << "\","
       << "\"pr\":\""  << jsonEscape(entry.provider)      << "\","
       << "\"mo\":\""  << jsonEscape(entry.modelName)     << "\","
       << "\"pp\":\""  << jsonEscape(entry.promptPreview) << "\","
       << "\"ok\":"    << (entry.success ? "true" : "false") << ","
       << "\"dr\":"    << (entry.dryRun  ? "true" : "false") << ","
       << "\"ms\":"    << entry.durationMs << ","
       << "\"err\":\"" << jsonEscape(entry.errorSummary)  << "\""
       << "}";
    return os.str();
}

bool AIAuditLog::_decodeEntry(const std::string& line, AuditEntry& out) {
    if (line.empty() || line[0] != '{') return false;
    try {
        out.timestamp     = iso8601ToTimestamp(jsonExtractString(line, "ts"));
        out.operation     = jsonExtractString(line, "op");
        out.provider      = jsonExtractString(line, "pr");
        out.modelName     = jsonExtractString(line, "mo");
        out.promptPreview = jsonExtractString(line, "pp");
        out.success       = jsonExtractBool(line,      "ok");
        out.dryRun        = jsonExtractBool(line,      "dr");
        out.durationMs    = jsonExtractLongLong(line,  "ms");
        out.errorSummary  = jsonExtractString(line, "err");
        return true;
    } catch (...) {
        return false;
    }
}

void AIAuditLog::append(const AuditEntry& entry) {
    _ensureDirectoryExists();
    std::ofstream f(_logFilePath, std::ios::app);
    if (f.is_open()) {
        f << _encodeEntry(entry) << '\n';
    }
}

std::vector<AuditEntry> AIAuditLog::loadRecentEntries(
        unsigned int maxEntries) const {
    std::ifstream f(_logFilePath);
    if (!f.is_open()) return {};

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) lines.push_back(std::move(line));
    }

    const unsigned int total = static_cast<unsigned int>(lines.size());
    const unsigned int skip  = (total > maxEntries) ? total - maxEntries : 0u;

    std::vector<AuditEntry> result;
    result.reserve(total - skip);
    for (unsigned int i = skip; i < total; ++i) {
        AuditEntry e;
        if (_decodeEntry(lines[i], e)) {
            result.push_back(std::move(e));
        }
    }
    return result;
}

unsigned int AIAuditLog::exportCsv(const std::string& csvPath) const {
    const auto entries = loadRecentEntries(1000000u);
    std::ofstream f(csvPath);
    if (!f.is_open()) return 0u;

    f << "Timestamp,Operation,Provider,Model,PromptPreview,Success,DryRun,DurationMs,Error\n";

    auto csvField = [](const std::string& s) -> std::string {
        std::string r = "\"";
        for (const char c : s) {
            if (c == '"') r += '"';
            r += c;
        }
        return r + '"';
    };

    unsigned int count = 0;
    for (const auto& e : entries) {
        f << csvField(timestampToIso8601(e.timestamp)) << ","
          << csvField(e.operation)     << ","
          << csvField(e.provider)      << ","
          << csvField(e.modelName)     << ","
          << csvField(e.promptPreview) << ","
          << (e.success ? "true" : "false") << ","
          << (e.dryRun  ? "true" : "false") << ","
          << e.durationMs << ","
          << csvField(e.errorSummary)  << "\n";
        ++count;
    }
    return count;
}
