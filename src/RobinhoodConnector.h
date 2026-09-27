#pragma once

#include <string>


class RobinhoodConnector {
public:
    RobinhoodConnector(std::string apiKey, std::string privateKey);
    ~RobinhoodConnector();

    std::string getOrders();
    std::string getAccounts();
    std::string getHoldings();
    std::string postOrder(const std::string& orderJson);

    std::string makeClientOrderId();

    std::string makeMarketOrderJson(const std::string& clientOrderId, // Unique identifier for the order
                                    const bool isBuy,
                                    const std::string& symbol,
                                    const float& assetQuantity);
    std::string makeLimitOrderJson(const std::string& clientOrderId,
                                   const bool isBuy,
                                   const std::string& symbol,
                                   const float& assetQuantity,
                                   const float& limitPrice,
                                   const std::string& timeInForce);
    std::string makeStopLossOrderJson(const std::string& clientOrderId,
                                      const bool isBuy,
                                      const std::string& symbol,
                                      const float& assetQuantity,
                                      const float& stopPrice,
                                      const std::string& timeInForce);
    std::string makeStopLimitOrderJson(const std::string& clientOrderId,
                                       const bool isBuy,
                                       const std::string& symbol,
                                       const float& assetQuantity,
                                       const float& limitPrice,
                                       const float& stopPrice,
                                       const std::string& timeInForce);

private:
    std::string apiKey_;
    std::string privateKey_;
    std::string signedGet(const std::string& path);
    std::string signedRequest(const std::string& path,
                              const std::string& method,
                              const std::string& body);
};