#include "CryptoHolding.h"
#include <format>
#include <random>
#include <iostream>

CryptoHolding::CryptoHolding(
    const std::string& asset_code, 
    double asset_quantity,
    double originalCostUSD,
    RobinhoodConnector& robinhoodConnector)
    : asset_code_(asset_code), asset_quantity_(asset_quantity), robinhoodConnector_(robinhoodConnector), originalCostUSD_(originalCostUSD)
{
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> jitter(0, 30);
    fullTimeJitter_ = static_cast<double>(jitter(generator));
}

void CryptoHolding::update() {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> jitter(0, 30);
    fullTimeJitter_ = static_cast<double>(jitter(generator));
    lastUpdated_ = std::chrono::system_clock::now();
    std::string modifiedCode = asset_code_ + "-USD";
    RobinhoodConnector::MarketValue usdMarketValue;
    for (auto& marketValue : robinhoodConnector_.getMarketValue(modifiedCode, "both", std::to_string(asset_quantity_))) {
        if (marketValue.side == "bid") {
            usdMarketValue = marketValue;
        }
    }
    double currentMarketValue = usdMarketValue.bidInclusiveOfSellSpread * asset_quantity_;
    if (currentMarketValue > highMarketValueUSD_) {
        highMarketValueUSD_ = currentMarketValue;
    }
    history_.push_back(currentMarketValue);
    if(history_.size() > 50) {
        history_.erase(history_.begin());
    }
    currentMarketValue_ = currentMarketValue;
    profitPrecent_ = (currentMarketValue_ - originalCostUSD_) / originalCostUSD_ * 100.0;
}

std::string CryptoHolding::sell() {
    std::string sellOrder = robinhoodConnector_.makeMarketOrderJson(
        robinhoodConnector_.makeClientOrderId(),
        false,
        asset_code_ + "-USD",
        asset_quantity_
    );
    //std::cout << "Sell order: " << sellOrder << std::endl;
    return robinhoodConnector_.postOrder(sellOrder);
}

std::string CryptoHolding::getColorTextSummary() {
    std::string result;

    std::string RedColor = "\x1b[31m";
    std::string GreenColor = "\x1b[32m";
    std::string ResetColor = "\x1b[0m";

    std::string profitColor;

    if(getCurrentProfit() > 0.02) {
        profitColor = GreenColor;
    } else if(getCurrentProfit() < 0) {
        profitColor = RedColor;
    } else {
        profitColor = ResetColor;
    }

    std::string historyString;
    for (std::size_t i = history_.size(); i > 1; --i) {
        const double entry = history_[i - 1];
        const double previousEntry = history_[i - 2];
        if (entry > previousEntry) {
            historyString += GreenColor + "↑" + ResetColor;
        } else if (entry < previousEntry) {
            historyString += RedColor + "↓" + ResetColor;
        } else {
            historyString += "=";
        }
    }
    result = std::format("{:<6.6} : {} {:>7.3f} {} : {:>7.3f} : {:>7.3f} : {}Profit {:>7.3f}% {} : Time {:02}:{:02} History {:50}", 
        asset_code_, 
        profitColor,
        getCurrentProfit(),
        ResetColor,
        getHighProfit(),
        getHighProfit() - getCurrentProfit(),
        profitColor,
        profitPrecent_,
        ResetColor,
        countDownSeconds_ / 60,
        countDownSeconds_ % 60,
        historyString
    );

    return result;
}

double CryptoHolding::getCurrentProfit() {
    return currentMarketValue_ - originalCostUSD_;
}

bool CryptoHolding::needsUpdate() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::seconds timeout = std::chrono::minutes(5) + std::chrono::seconds(static_cast<int>(fullTimeJitter_));

    if(history_.size() < 2) {
        return true;
    }


    if(getCurrentProfit() > 0){
        timeout = std::chrono::minutes(1);
    }

    if(getCurrentProfit() > 0 && (getHighProfit() - getCurrentProfit() > 0.001)){
        timeout = std::chrono::seconds(30);
    }

    countDownSeconds_ = (timeout - std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdated_)).count();
    
    return (now - lastUpdated_) > timeout;
}