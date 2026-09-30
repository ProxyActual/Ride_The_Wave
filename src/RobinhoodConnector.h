#pragma once

#include <string>
#include <vector>


class RobinhoodConnector {
public:

    struct Order {
        std::string id;
        std::string clientOrderId;
        std::string accountNumber;
        std::string symbol;
        std::string side;
        std::string type;
        std::string state;
        double averagePrice = 0.0;
        double filledAssetQuantity = 0.0;
        std::string createdAt;
        std::string updatedAt;
    };

    struct Account {
        float account_number = 0.0;
        std::string status;
        std::string buying_power_currency;
        float buying_power = 0.0;
    };

    struct MarketValue {
        std::string symbol;
        double price = 0.0;
        double quantity = 0.0;
        std::string side;
        double bidInclusiveOfSellSpread = 0.0;
        double sellSpread = 0.0;
        std::string timestamp;
    };

    struct Holding {
        std::string account_number;
        std::string asset_code;
        double total_quantity = 0.0;
        double quantity_available_for_trading = 0.0;
    };

    RobinhoodConnector(std::string apiKey, std::string privateKey);
    ~RobinhoodConnector();

    std::vector<Order> getOrders();
    std::vector<Account> getAccounts();
    std::vector<Holding> getHoldings();

    std::vector<MarketValue> getMarketValue(const std::string& symbol,
                               const std::string& side,
                               const std::string& quantity);
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