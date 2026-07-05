#pragma once

#include <cstddef>
#include <string>

class TokenService {
public:
    TokenService();

    [[nodiscard]] std::string generateSessionId();
    [[nodiscard]] std::string generateAccessToken();

private:
    [[nodiscard]] std::string _generateHexToken(std::size_t bytes);
};
