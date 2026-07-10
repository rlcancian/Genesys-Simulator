#include "TokenService.h"

#include <iomanip>
#include <random>
#include <sstream>

TokenService::TokenService() = default;

std::string TokenService::generateSessionId() {
    return "sess_" + _generateHexToken(12);
}

std::string TokenService::generateAccessToken() {
    return _generateHexToken(24);
}

std::string TokenService::_generateHexToken(std::size_t bytes) {
    thread_local std::random_device seedDevice;
    thread_local std::mt19937_64 rng(seedDevice());
    std::uniform_int_distribution<unsigned int> distribution(0, 255);

    std::ostringstream builder;
    builder << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < bytes; ++i) {
        builder << std::setw(2) << distribution(rng);
    }
    return builder.str();
}
