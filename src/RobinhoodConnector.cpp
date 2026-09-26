#include "RobinhoodConnector.h"
#include <array>
#include <cstdio>
#include <stdexcept>

RobinhoodConnector::RobinhoodConnector() {
}

RobinhoodConnector::~RobinhoodConnector() {
}

void RobinhoodConnector::authentication() {
    if(!authToken.has_value()) {
        // Execute some form of authentication here
    }
}

std::string RobinhoodConnector::getResponse() {
    // The markets listing is public and does not require an authentication token.
    FILE* pipe = popen("curl --fail --silent --show-error --location --max-time 15 https://api.robinhood.com/markets/", "r");
    if (pipe == nullptr) {
        throw std::runtime_error("Could not start curl");
    }

    std::string response;
    std::array<char, 4096> buffer{};
    while (std::size_t count = std::fread(buffer.data(), 1, buffer.size(), pipe)) {
        response.append(buffer.data(), count);
    }
    if (std::ferror(pipe) || pclose(pipe) != 0) {
        throw std::runtime_error("Failed to fetch Robinhood markets");
    }
    return response;
}