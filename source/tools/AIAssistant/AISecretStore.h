/*
 * File:   AISecretStore.h
 * Author: Genesys Team
 *
 * Created on 28 de Maio de 2026
 */

#ifndef AISECRETSTORE_H
#define AISECRETSTORE_H

#include <optional>
#include <string>

/**
 * @brief Static helper for storing API keys in the OS keyring.
 *
 * @details
 * On Linux, uses the @c secret-tool command-line utility (part of the
 * @c libsecret-tools package) to access the GNOME Keyring / KWallet service
 * via the freedesktop Secret Service protocol.
 *
 * All methods return a failure indicator (@c false or @c std::nullopt) when
 * @c secret-tool is not installed; callers should fall back to env-var loading.
 *
 * API keys are never written to disk in plaintext; they live only in the
 * session keyring managed by the OS.
 *
 * ## Usage
 * @code
 * const std::string svc = "genesys-ai-assistant";
 * const std::string acct = "openai";
 * if (AISecretStore::isAvailable()) {
 *     AISecretStore::save(svc, acct, rawKey);
 *     auto loaded = AISecretStore::load(svc, acct);
 * }
 * @endcode
 */
class AISecretStore {
public:
    AISecretStore() = delete;

    /**
     * @brief Returns @c true if the keyring backend is reachable on this system.
     *
     * @details
     * Checks that @c secret-tool is on PATH and that the D-Bus session bus is
     * available (required by the Secret Service protocol).
     */
    static bool isAvailable();

    /**
     * @brief Saves @p secret under the given @p service and @p account labels.
     * @return @c true on success, @c false if the keyring is unavailable or the
     *         operation failed.
     */
    static bool save(const std::string& service,
                     const std::string& account,
                     const std::string& secret);

    /**
     * @brief Loads the secret stored under @p service and @p account.
     * @return The secret, or @c std::nullopt when not found or unavailable.
     */
    static std::optional<std::string> load(const std::string& service,
                                           const std::string& account);

    /**
     * @brief Removes the stored secret.
     * @return @c true on success.
     */
    static bool remove(const std::string& service,
                       const std::string& account);

private:
    static std::string _shellEscape(const std::string& s);
    static std::string _pipeRead(const std::string& cmd);
};

#endif /* AISECRETSTORE_H */
