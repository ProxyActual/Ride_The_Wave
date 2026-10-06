#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "ConfigManager.h"
#include "CryptoHolding.h"
#include "ConsoleUI.h"
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

static const std::vector<std::vector<double>> BaseTextColors{
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
    {192, 192, 192},
    {128, 128, 128},
    {0, 0, 0},
    {255, 165, 0},
    {128, 0, 128},
    {0, 128, 0},
    {0, 128, 128}
};

int main(){
    static ConsoleUI consoleUI;
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

    std::string RedColor = "\x1b[31m";
    std::string GreenColor = "\x1b[32m";
    std::string ResetColor = "\x1b[0m";

    while(true){
        bool anyUpdates = false;
        static constexpr bool allowSell = true;

        std::string soldText;
        int color = 0;
        for (CryptoHolding& cryptoHolding : cryptoHoldings) {
            consoleUI.setCursorPosition(color + 1, 1);
            consoleUI.setColor(BaseTextColors[color % BaseTextColors.size()]);
            std::cout << cryptoHolding.getColorTextSummary() << std::endl;
            if(cryptoHolding.needsUpdate()){
                cryptoHolding.update();
                //std::cout << cryptoHolding.getAssetCode() << " updated. Current profit: " << cryptoHolding.getCurrentProfit() << std::endl;
                if(!allowSell){
                    continue;
                }
                if(cryptoHolding.getCurrentProfit() > 0.02 && cryptoHolding.getHighProfit() - cryptoHolding.getCurrentProfit() > 0.01){
                    anyUpdates = true;
                    soldText += GreenColor + "Selling " + cryptoHolding.getAssetCode() + " with profit: " + std::to_string(cryptoHolding.getCurrentProfit()) + ResetColor + 
                    "\n\t" + cryptoHolding.sell() + "\n";
                }

                if(cryptoHolding.getProfitPrecent() < -10.0){
                    anyUpdates = true;
                    std::string SellReturnCode = cryptoHolding.sell();
                    soldText += RedColor + "LOSS PREVENTION: " + cryptoHolding.getAssetCode() + " with profit: " + std::to_string(cryptoHolding.getCurrentProfit()) + ResetColor + 
                    "\n\t" + SellReturnCode + "\n";
                }
            }
            consoleUI.setColor(BaseTextColors[color % BaseTextColors.size()]);
            consoleUI.drawGraph(10, 1, 30, 150, cryptoHolding.getHistory());

            color++;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        consoleUI.clearScreen();
        if(anyUpdates){
            std::cout << "Some holdings were sold." << std::endl;
            std::cout << soldText + ResetColor;
            cryptoHoldings = getCryptoHoldings(robinhoodConnector);
        }
    }
}