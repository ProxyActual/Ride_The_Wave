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
        std::string orders = robinhoodConnector.getOrders();
        std::cout << "Robinhood Holdings: " << orders << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood holdings: " << e.what() << std::endl;
    }

    

}