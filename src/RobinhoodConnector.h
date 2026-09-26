#pragma once

#include <string>


class RobinhoodConnector {
public:
    RobinhoodConnector(std::string apiKey, std::string privateKey);
    ~RobinhoodConnector();

    std::string getOrders();
    std::string getAccounts();
    std::string getHoldings();

private:
    std::string apiKey_;
    std::string privateKey_;
    std::string signedGet(const std::string& path);
};