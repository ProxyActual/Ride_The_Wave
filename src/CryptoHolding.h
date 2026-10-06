#pragma once

#include <string>
#include <chrono>
#include "RobinhoodConnector.h"

class CryptoHolding {
    public:
        CryptoHolding(const std::string& asset_code, 
            double asset_quantity,
            double originalCostUSD,
            RobinhoodConnector& robinhoodConnector);

        void update();
        std::string sell();

        std::string getColorTextSummary();

        double getCurrentProfit();
        double getHighProfit() const { return highMarketValueUSD_ - originalCostUSD_; }

        bool needsUpdate();

        double getCurrentMarketValue() const { return currentMarketValue_; }
        double getHighMarketValueUSD() const { return highMarketValueUSD_; }
        std::string getAssetCode() const { return asset_code_; }
        double getAssetQuantity() const { return asset_quantity_; }
        double getOriginalCostUSD() const { return originalCostUSD_; }
        double getProfitPrecent() const { return profitPrecent_; }
        const std::vector<double>& getHistory() const { return history_; }
            
    private:
        std::vector<double> history_;

        RobinhoodConnector& robinhoodConnector_;

        std::string asset_code_;
        double asset_quantity_;

        double originalCostUSD_;
        double currentMarketValue_ {0.0};
        double profitPrecent_ {0.0};

        double highMarketValueUSD_ {0.0};

        double fullTimeJitter_ {0.0};

        std::chrono::time_point<std::chrono::system_clock> lastUpdated_;
        int countDownSeconds_ {0};
};