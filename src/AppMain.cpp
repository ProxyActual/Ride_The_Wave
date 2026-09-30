#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include "ConfigManager.h"
#include "RobinhoodConnector.h"

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
    
    try {
        auto orders = robinhoodConnector.getOrders();

        auto holdings = robinhoodConnector.getHoldings();

        std::map<std::string, float> profitHighs;

        bool orderPlaced = false;

        while(true){

            for (const auto& holding: holdings) {
                std::string modifiedCode = holding.asset_code + "-USD";
                robinhoodConnector.getMarketValue(modifiedCode, "both", std::to_string(holding.total_quantity));

                float usdMarketValue = 0.0f;
                for (const auto& marketValue : robinhoodConnector.getMarketValue(modifiedCode, "both", std::to_string(holding.total_quantity))) {
                    if (marketValue.side == "ask") {
                        usdMarketValue += marketValue.price * marketValue.quantity;
                    }
                }
                std::cout << holding.asset_code << " : " << holding.total_quantity  << " (" << usdMarketValue << " USD)" << std::endl;
                


                bool orderFound = false;
                for (const auto& order : orders) {

                    if (order.symbol == modifiedCode) {
                        if(order.side == "buy" && order.filledAssetQuantity == holding.total_quantity) {
                            float costUSD = order.averagePrice * order.filledAssetQuantity;
                            float profitUSD = usdMarketValue - costUSD;
                            std::cout << "\tCurrent return :    ";
                            if(profitUSD < 0){
                                std::cout << "\033[31m";
                            } else {
                                std::cout << "\033[32m";
                            }
                            std::cout << profitUSD << " USD" << std::endl;
                            std::cout << "\033[0m";
                            orderFound = true;

                            if(profitHighs.find(modifiedCode) == profitHighs.end() || profitUSD > profitHighs[modifiedCode]) {
                                profitHighs[modifiedCode] = profitUSD;
                            }

                            if(profitHighs.find(modifiedCode) != profitHighs.end()) {
                                std::cout << "\tHighest return :    ";
                                if(profitHighs[modifiedCode] < 0){
                                    std::cout << "\033[31m";
                                } else {
                                    std::cout << "\033[32m";
                                }
                                std::cout << profitHighs[modifiedCode] << " USD" << std::endl;
                                std::cout << "\033[0m";
                                float amountBelowHigh = profitUSD - profitHighs[modifiedCode];
                                std::cout << "\tAmount Below High : ";
                                if(amountBelowHigh < -0.02){
                                    std::cout << "\033[31m";
                                } else {
                                    std::cout << "\033[32m";
                                }
                                std::cout << amountBelowHigh << " USD" << std::endl;
                                std::cout << "\033[0m";

                                if(amountBelowHigh < -0.01 && profitUSD > .02){

                                    robinhoodConnector.makeMarketOrderJson(
                                        robinhoodConnector.makeClientOrderId(),
                                        false,
                                        modifiedCode,
                                        holding.quantity_available_for_trading
                                    );

                                    orderPlaced = true;
                                }

                            }

                        }
                    }
                }
                if (!orderFound) {
                    std::cout << "\tNo matching order found for this holding." << std::endl;
                }
            }

            if(orderPlaced) {
                std::cout << "\tOrder placed, refreshing orders and holdings..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(15));
                orders = robinhoodConnector.getOrders();
                holdings = robinhoodConnector.getHoldings();
                orderPlaced = false;
            }


            for (int i = 0; i < 2 * 60; ++i) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                int timeLeft = 2 * 60 - i;
                std::cout << "\033[2K\r";
                std::cout.flush();
                std::cout << "Time left: " << timeLeft / 60 << " minutes " << timeLeft % 60 << " seconds";
                std::cout.flush();

            }
            std::cout << std::endl;
        }
         
    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood orders: " << e.what() << std::endl;
    }

}