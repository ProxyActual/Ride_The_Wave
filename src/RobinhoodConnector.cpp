#include "RobinhoodConnector.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sodium.h>

#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <limits>

namespace {
using json = nlohmann::json;

std::string getString(const json& object, const char* key) {
    const auto it = object.find(key);
    return (it != object.end() && it->is_string()) ? it->get<std::string>() : "";
}

// Robinhood returns some numeric fields as strings, so accept either form.
double getNumber(const json& object, const char* key) {
    const auto it = object.find(key);
    if (it == object.end() || it->is_null()) {
        return 0.0;
    }
    if (it->is_number()) {
        return it->get<double>();
    }
    if (it->is_string()) {
        try {
            return std::stod(it->get<std::string>());
        } catch (const std::exception&) {
        }
    }
    return 0.0;
}

std::string quoteJson(const std::string& value) {
    std::string quoted = "\"";
    for (const char character : value) {
        if (character == '\\' || character == '"') {
            quoted += '\\';
        }
        quoted += character;
    }
    return quoted + "\"";
}

std::string orderSide(const bool isBuy) {
    return isBuy ? "buy" : "sell";
}
}

RobinhoodConnector::RobinhoodConnector(std::string apiKey, std::string privateKey) {
    apiKey_ = apiKey;
    privateKey_ = privateKey;
}

RobinhoodConnector::~RobinhoodConnector() {
}

std::vector<RobinhoodConnector::Order> RobinhoodConnector::getOrders() {
    const json response = json::parse(signedGet("/api/v1/crypto/trading/orders/"));

    std::vector<Order> orders;
    for (const json& item : response.value("results", json::array())) {
        Order order;
        order.id = getString(item, "id");
        order.clientOrderId = getString(item, "client_order_id");
        order.accountNumber = getString(item, "account_number");
        order.symbol = getString(item, "symbol");
        order.side = getString(item, "side");
        order.type = getString(item, "type");
        order.state = getString(item, "state");
        order.averagePrice = getNumber(item, "average_price");
        order.filledAssetQuantity = getNumber(item, "filled_asset_quantity");
        order.createdAt = getString(item, "created_at");
        order.updatedAt = getString(item, "updated_at");
        orders.push_back(order);
    }
    return orders;
}

std::vector<RobinhoodConnector::Account> RobinhoodConnector::getAccounts() {
    std::string responseStr = signedGet("/api/v1/crypto/trading/accounts/");
    const json response = json::parse(responseStr);
    std::vector<Account> accounts;
    Account account;
    account.account_number = getNumber(response, "account_number");
    account.status = getString(response, "status");
    account.buying_power_currency = getString(response, "buying_power_currency");
    account.buying_power = getNumber(response, "buying_power");
    accounts.push_back(account);
    return accounts;
}

std::vector<RobinhoodConnector::Holding> RobinhoodConnector::getHoldings(){
    const json response = json::parse(signedGet("/api/v1/crypto/trading/holdings/"));
    std::vector<Holding> holdings;
    for (const json& item : response.value("results", json::array())) {
        Holding holding;
        holding.account_number = getString(item, "account_number");
        holding.asset_code = getString(item, "asset_code");
        holding.total_quantity = getNumber(item, "total_quantity");
        holding.quantity_available_for_trading = getNumber(item, "quantity_available_for_trading");
        holdings.push_back(holding);
    }
    return holdings;
}

std::vector<RobinhoodConnector::MarketValue> RobinhoodConnector::getMarketValue(const std::string& symbol,
                                               const std::string& side,
                                               const std::string& quantity) {
    if (symbol.empty() || quantity.empty()) {
        throw std::invalid_argument("Market value requires a symbol and quantity");
    }
    if (side != "bid" && side != "ask" && side != "both") {
        throw std::invalid_argument("Market value side must be bid, ask, or both");
    }

    const std::string path = "/api/v1/crypto/marketdata/estimated_price/?symbol=" +
                             symbol + "&side=" + side + "&quantity=" + quantity;
    const json response = json::parse(signedGet(path));
    std::vector<MarketValue> marketValues;
    for (const json& item : response.value("results", json::array())) {
        MarketValue marketValue;
        marketValue.symbol = getString(item, "symbol");
        marketValue.price = getNumber(item, "price");
        marketValue.quantity = getNumber(item, "quantity");
        marketValue.side = getString(item, "side");
        marketValue.bidInclusiveOfSellSpread = getNumber(item, "bid_inclusive_of_sell_spread");
        marketValue.sellSpread = getNumber(item, "sell_spread");
        marketValue.timestamp = getString(item, "timestamp");
        marketValues.push_back(marketValue);
    }
    return marketValues;
}

std::string RobinhoodConnector::postOrder(const std::string& orderJson) {
    return signedRequest("/api/v1/crypto/trading/orders/", "POST", orderJson);
}

std::string RobinhoodConnector::signedGet(const std::string& path) {
    return signedRequest(path, "GET", "");
}

std::string RobinhoodConnector::signedRequest(const std::string& path,
                                              const std::string& method,
                                              const std::string& body) {
    const char* apiKey = apiKey_.c_str();
    const char* encodedSeed = privateKey_.c_str();
    if (!apiKey || !encodedSeed) {
        throw std::runtime_error("Robinhood credentials are not set");
    }
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed");
    }

    unsigned char seed[crypto_sign_SEEDBYTES];
    size_t seedLength = 0;
    if (sodium_base642bin(seed, sizeof(seed), encodedSeed,
                          std::strlen(encodedSeed), nullptr, &seedLength,
                          nullptr, sodium_base64_VARIANT_ORIGINAL) != 0 ||
        seedLength != sizeof(seed)) {
        throw std::runtime_error("Invalid Robinhood private-key seed");
    }

    unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char secretKey[crypto_sign_SECRETKEYBYTES];
    crypto_sign_seed_keypair(publicKey, secretKey, seed);
    sodium_memzero(seed, sizeof(seed));

    const std::string timestamp = std::to_string(std::time(nullptr));
    const std::string message = std::string(apiKey) + timestamp + path + method + body;

    unsigned char signature[crypto_sign_BYTES];
    crypto_sign_detached(signature, nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(), secretKey);
    sodium_memzero(secretKey, sizeof(secretKey));

    char encodedSignature[sodium_base64_ENCODED_LEN(
        crypto_sign_BYTES, sodium_base64_VARIANT_ORIGINAL)];
    sodium_bin2base64(encodedSignature, sizeof(encodedSignature),
                      signature, sizeof(signature),
                      sodium_base64_VARIANT_ORIGINAL);

    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Could not initialize curl");
    }

    curl_slist* headers = nullptr;
    const auto addHeader = [&](const std::string& header) {
        curl_slist* updated = curl_slist_append(headers, header.c_str());
        if (!updated) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            throw std::runtime_error("Could not allocate HTTP headers");
        }
        headers = updated;
    };
    addHeader(std::string("x-api-key: ") + apiKey);
    addHeader("x-timestamp: " + timestamp);
    addHeader(std::string("x-signature: ") + encodedSignature);
    addHeader("Content-Type: application/json; charset=utf-8");

    std::string response;
    const std::string url = "https://trading.robinhood.com" + path;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    if (!body.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     +[](char* data, size_t size, size_t count, void* context) -> size_t {
                         const size_t bytes = size * count;
                         static_cast<std::string*>(context)->append(data, bytes);
                         return bytes;
                     });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    const CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK || status < 200 || status >= 300) {
        throw std::runtime_error("Robinhood request failed (HTTP " +
                                 std::to_string(status) + "): " + response);
    }
    return response;
}

std::string RobinhoodConnector::makeClientOrderId() {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed");
    }

    unsigned char bytes[16];
    randombytes_buf(bytes, sizeof(bytes));
    bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0f) | 0x40);
    bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3f) | 0x80);

    const char* hexadecimal = "0123456789abcdef";
    std::string clientOrderId;
    clientOrderId.reserve(36);
    for (size_t index = 0; index < sizeof(bytes); ++index) {
        if (index == 4 || index == 6 || index == 8 || index == 10) {
            clientOrderId += '-';
        }
        clientOrderId += hexadecimal[(bytes[index] >> 4) & 0x0f];
        clientOrderId += hexadecimal[bytes[index] & 0x0f];
    }
    return clientOrderId;
}

std::string RobinhoodConnector::makeMarketOrderJson(const std::string& clientOrderId,
                                                    const bool isBuy,
                                                    const std::string& symbol,
                                                    const double& assetQuantity) {
    std::ostringstream quantity;
    quantity << std::setprecision(std::numeric_limits<double>::max_digits10)
             << assetQuantity;
    return "{"
        "\"client_order_id\":" + quoteJson(clientOrderId) + ","
        "\"side\":" + quoteJson(orderSide(isBuy)) + ","
        "\"symbol\":" + quoteJson(symbol) + ","
        "\"type\":\"market\","
        "\"market_order_config\":{\"asset_quantity\":" + quantity.str() +
        "}}";
}

std::string RobinhoodConnector::makeLimitOrderJson(const std::string& clientOrderId,
                              const bool isBuy,
                              const std::string& symbol,
                              const double& assetQuantity,
                              const double& limitPrice,
                              const std::string& timeInForce) {
    return "{"
        "\"client_order_id\":" + quoteJson(clientOrderId) + ","
        "\"side\":" + quoteJson(orderSide(isBuy)) + ","
        "\"symbol\":" + quoteJson(symbol) + ","
        "\"type\":\"limit\","
        "\"limit_order_config\":{" 
        "\"asset_quantity\":" + std::to_string(assetQuantity) + ","
        "\"limit_price\":" + std::to_string(limitPrice) + ","
        "\"time_in_force\":" + quoteJson(timeInForce) + "}}";
}

std::string RobinhoodConnector::makeStopLossOrderJson(const std::string& clientOrderId,
                                 const bool isBuy,
                                 const std::string& symbol,
                                 const double& assetQuantity,
                                 const double& stopPrice,
                                 const std::string& timeInForce) {
    return "{"
        "\"client_order_id\":" + quoteJson(clientOrderId) + ","
        "\"side\":" + quoteJson(orderSide(isBuy)) + ","
        "\"symbol\":" + quoteJson(symbol) + ","
        "\"type\":\"stop_loss\","
        "\"stop_loss_order_config\":{"
        "\"asset_quantity\":" + std::to_string(assetQuantity) + ","
        "\"stop_price\":" + std::to_string(stopPrice) + ","
        "\"time_in_force\":" + quoteJson(timeInForce) + "}}";
}

std::string RobinhoodConnector::makeStopLimitOrderJson(const std::string& clientOrderId,
                                  const bool isBuy,
                                  const std::string& symbol,
                                  const double& assetQuantity,
                                  const double& limitPrice,
                                  const double& stopPrice,
                                  const std::string& timeInForce) {
    return "{"
        "\"client_order_id\":" + quoteJson(clientOrderId) + ","
        "\"side\":" + quoteJson(orderSide(isBuy)) + ","
        "\"symbol\":" + quoteJson(symbol) + ","
        "\"type\":\"stop_limit\","
        "\"stop_limit_order_config\":{"
        "\"asset_quantity\":" + std::to_string(assetQuantity) + ","
        "\"limit_price\":" + std::to_string(limitPrice) + ","
        "\"stop_price\":" + std::to_string(stopPrice) + ","
        "\"time_in_force\":" + quoteJson(timeInForce) + "}}";
}