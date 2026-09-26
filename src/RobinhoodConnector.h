#pragma once

#include <string>


class RobinhoodConnector {
public:
    RobinhoodConnector(std::string apiKey, std::string privateKey);
    ~RobinhoodConnector();

    std::string getOrders();

private:
    std::string apiKey_;
    std::string privateKey_;
};