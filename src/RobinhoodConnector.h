#pragma once

#include <optional>
#include <string>

class RobinhoodConnector {
    public:
        RobinhoodConnector();
        ~RobinhoodConnector();
        void authentication();
        std::string getResponse();

    private:
        std::optional<std::string> authToken{std::nullopt};
};