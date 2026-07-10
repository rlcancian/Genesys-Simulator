/*
 * File:   AISecretStore.cpp
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#include "AISecretStore.h"

#include <cstdio>   // popen, pclose, fgets
#include <cstdlib>  // system
#include <sstream>

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

// Single-quote shell escaping: wrap in '' and escape embedded single-quotes
// using the '"'"' trick. This prevents shell injection through service/account
// names or secret values.
std::string AISecretStore::_shellEscape(const std::string& s) {
    std::string out = "'";
    for (const char c : s) {
        if (c == '\'') out += "'\\''";
        else           out += c;
    }
    return out + "'";
}

std::string AISecretStore::_pipeRead(const std::string& cmd) {
    FILE* f = ::popen(cmd.c_str(), "r");
    if (f == nullptr) return {};
    std::string result;
    char buf[256];
    while (::fgets(buf, sizeof(buf), f) != nullptr) {
        result += buf;
    }
    ::pclose(f);
    // Strip trailing newline that secret-tool appends.
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

// ---------------------------------------------------------------------------
// AISecretStore
// ---------------------------------------------------------------------------

bool AISecretStore::isAvailable() {
    // Confirm secret-tool is on PATH.
    const int rc = ::system("which secret-tool > /dev/null 2>&1");
    return rc == 0;
}

bool AISecretStore::save(const std::string& service,
                         const std::string& account,
                         const std::string& secret) {
    if (!isAvailable()) return false;

    // Pipe the secret through printf to avoid exposing it in the process argv.
    // Label is visible in the keyring UI (e.g., GNOME Passwords).
    std::ostringstream cmd;
    cmd << "printf '%s' " << _shellEscape(secret)
        << " | secret-tool store"
        << " --label=" << _shellEscape("GenESyS AI key: " + account)
        << " service " << _shellEscape(service)
        << " account " << _shellEscape(account)
        << " > /dev/null 2>&1";
    return ::system(cmd.str().c_str()) == 0;
}

std::optional<std::string> AISecretStore::load(const std::string& service,
                                               const std::string& account) {
    if (!isAvailable()) return std::nullopt;
    std::ostringstream cmd;
    cmd << "secret-tool lookup"
        << " service " << _shellEscape(service)
        << " account " << _shellEscape(account)
        << " 2>/dev/null";
    const std::string result = _pipeRead(cmd.str());
    if (result.empty()) return std::nullopt;
    return result;
}

bool AISecretStore::remove(const std::string& service,
                           const std::string& account) {
    if (!isAvailable()) return false;
    std::ostringstream cmd;
    cmd << "secret-tool clear"
        << " service " << _shellEscape(service)
        << " account " << _shellEscape(account)
        << " > /dev/null 2>&1";
    return ::system(cmd.str().c_str()) == 0;
}
