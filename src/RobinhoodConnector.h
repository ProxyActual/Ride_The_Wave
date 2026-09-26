#pragma once

#include <string>


class RobinhoodConnector {
public:
    RobinhoodConnector();
    ~RobinhoodConnector();

    std::string getOrders();
};