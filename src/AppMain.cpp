#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "ConfigManager.h"
#include "CryptoHolding.h"
#include "RobinhoodConnector.h"

static constexpr int REFRESH_INTERVAL_SECONDS = 5 * 60;

std::vector<CryptoHolding> getCryptoHoldings(RobinhoodConnector& robinhoodConnector){
    std::vector<CryptoHolding> cryptoHoldings;
    auto holdings = robinhoodConnector.getHoldings();
    auto orders = robinhoodConnector.getOrders();
    for(auto holding : holdings) {
        double originalCostUSD = 0.0;
        for (const auto& order : orders) {
            if (order.symbol == holding.asset_code + "-USD" && order.side == "buy" && order.filledAssetQuantity == holding.total_quantity) {
                originalCostUSD = order.averagePrice * order.filledAssetQuantity;
                break;
            }
        }

        cryptoHoldings.emplace_back(holding.asset_code, holding.quantity_available_for_trading, originalCostUSD, robinhoodConnector);
    }
    return cryptoHoldings;
}

int main(){
    std::cout << "Welcome to Ride The Wave!" << std::endl;

    std::cout << "Loading configuration..." << std::endl;

    ConfigManager configManager;
    if(!configManager.loadSettings("config.txt")){
        std::cerr << "Failed to load configuration." << std::endl;

        std::cout << "Enter your API key: ";
        std::getline(std::cin, configManager.localConfig_.api_key);

        std::cout << "Enter your public key: ";
        std::getline(std::cin, configManager.localConfig_.public_key);

        std::cout << "Enter your private key: ";
        std::getline(std::cin, configManager.localConfig_.private_key);

        configManager.saveSettings("config.txt");
    }

    std::cout << "Configuration loaded successfully." << std::endl;

    RobinhoodConnector robinhoodConnector(configManager.localConfig_.api_key, configManager.localConfig_.private_key);
    
    std::vector<CryptoHolding> cryptoHoldings = getCryptoHoldings(robinhoodConnector);

    for (const auto& cryptoHolding : cryptoHoldings) {
        std::cout << cryptoHolding.getAssetCode() << " : " << cryptoHolding.getOriginalCostUSD() << std::endl;
    }

    std::cout << "\n\n";

    while(true){
        bool anyUpdates = false;

        std::string soldText;
        for (CryptoHolding& cryptoHolding : cryptoHoldings) {

            std::cout << cryptoHolding.getColorTextSummary() << std::endl;
            if(cryptoHolding.needsUpdate()){
                cryptoHolding.update();
                //std::cout << cryptoHolding.getAssetCode() << " updated. Current profit: " << cryptoHolding.getCurrentProfit() << std::endl;

                if(cryptoHolding.getCurrentProfit() > 0.02 && cryptoHolding.getHighProfit() - cryptoHolding.getCurrentProfit() > 0.01){
                    anyUpdates = true;
                    soldText += "Selling " + cryptoHolding.getAssetCode() + " with profit: " + std::to_string(cryptoHolding.getCurrentProfit()) + 
                    "\n\t" + cryptoHolding.sell() + "\n";
                }
            }

        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (CryptoHolding& cryptoHolding : cryptoHoldings) {
            std::cout << "\033[1A\033[2K";
        }
        if(anyUpdates){
            std::cout << "Some holdings were sold." << std::endl;
            std::cout << soldText;
            cryptoHoldings = getCryptoHoldings(robinhoodConnector);
        }
    }
}