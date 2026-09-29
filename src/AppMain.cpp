#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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
        for (const auto& order : robinhoodConnector.getOrders()) {
            std::cout << order.createdAt << "  " << order.symbol << "  " << order.side
                      << "  " << order.type << "  " << order.state
                      << "  qty=" << order.filledAssetQuantity
                      << "  avg=" << order.averagePrice << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood orders: " << e.what() << std::endl;
    }



    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood orders: " << e.what() << std::endl;
    }

    

}