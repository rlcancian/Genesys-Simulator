#pragma once

#include "../GenesysApplication_if.h"
#include <string>

/**
 * @brief Entry point for the standalone Worker application flavor of Genesys.
 *
 * The worker variant bootstraps a session manager, HTTP server and router, then
 * keeps processing requests until the configured request limit is reached or a
 * transport error occurs.
 */
class BaseGenesysWebApplication : public GenesysApplication_if {
public:
    std::string getVersionName() const { return _versionName; }
    unsigned int getVersionNumber() const { return _versionNumber; }
    /**
     * @brief Parses command-line arguments, starts the HTTP server and runs the worker app.
     * @param argc Number of command-line arguments.
     * @param argv Command-line argument vector.
     * @return Zero on normal termination; non-zero on startup or server failure.
     */
    int main(int argc, char** argv) override;

private:
    const std::string _versionName = "inaiera";
    const unsigned int _versionNumber = 261115;
    /**
     * @brief Reads the listening port from the command line.
     * @param argc Number of command-line arguments.
     * @param argv Command-line argument vector.
     * @return TCP port used by the embedded HTTP server.
     */
    static unsigned short _readPort(int argc, char** argv);
    /**
     * @brief Reads the optional request cap used to stop the worker server loop.
     * @param argc Number of command-line arguments.
     * @param argv Command-line argument vector.
     * @return Maximum number of requests to serve, or zero for unlimited.
     */
    static unsigned long _readMaxRequests(int argc, char** argv);
};
