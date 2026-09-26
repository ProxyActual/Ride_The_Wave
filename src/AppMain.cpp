#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ConfigManager.h"
#include "RobinhoodConnector.h"

int main(){
    std::cout << "Hello, Ride The Wave!" << std::endl;

    ConfigManager configManager;
    configManager.loadSettings("config.txt");

    RobinhoodConnector robinhoodConnector;
    
    try {
        std::string orders = robinhoodConnector.getOrders();
        std::cout << "Robinhood Orders: " << orders << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood orders: " << e.what() << std::endl;
    }

}