/*
 * File:   AIAuditLog.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef AIAUDITLOG_H
#define AIAUDITLOG_H

#include <chrono>
#include <string>
#include <vector>

/**
 * @brief One entry in the AI assistant audit log.
 *
 * @details
 * Never contains API key values. The promptPreview field is limited to the
 * first 120 characters of the user-visible prompt text.
 */
struct AuditEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string  operation;       ///< e.g. "analyzePrompt", "buildModel"
    std::string  provider;        ///< e.g. "OpenAI", "Anthropic", "Local"
    std::string  modelName;       ///< LLM model identifier
    std::string  promptPreview;   ///< First 120 chars of user-visible input — no keys
    bool         success   = false;
    bool         dryRun    = false;
    long long    durationMs = 0;
    std::string  errorSummary;    ///< Empty on success; capped at 200 chars
};

/**
 * @brief Append-only audit log for AI assistant operations.
 *
 * @details
 * Entries are written in JSON Lines format to a file under
 * $XDG_DATA_HOME/genesys/ (typically ~/.local/share/genesys/ai_audit.log).
 * The log never contains API key values. The directory is created on first write.
 */
class AIAuditLog {
public:
    explicit AIAuditLog(const std::string& logFilePath = "");
    ~AIAuditLog() = default;

    AIAuditLog(const AIAuditLog&)            = delete;
    AIAuditLog& operator=(const AIAuditLog&) = delete;

    /** @brief Appends one entry to the log file (creates directory if needed). */
    void append(const AuditEntry& entry);

    /**
     * @brief Reads up to @p maxEntries most-recent entries.
     * @return Entries in chronological order (oldest first).
     */
    std::vector<AuditEntry> loadRecentEntries(unsigned int maxEntries = 200) const;

    /** @brief Returns the absolute path of the log file. */
    std::string logFilePath() const { return _logFilePath; }

    /**
     * @brief Exports all log entries to a CSV file.
     * @return Number of rows written (excluding the header).
     */
    unsigned int exportCsv(const std::string& csvPath) const;

    /** @brief Returns the platform-default log file path. */
    static std::string defaultLogFilePath();

private:
    static std::string _encodeEntry(const AuditEntry& entry);
    static bool        _decodeEntry(const std::string& line, AuditEntry& out);
    void               _ensureDirectoryExists() const;

    std::string _logFilePath;
};

#endif /* AIAUDITLOG_H */
