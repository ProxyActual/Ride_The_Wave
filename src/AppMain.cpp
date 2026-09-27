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

    RobinhoodConnector robinhoodConnector(configManager.localConfig_.api_key, configManager.localConfig_.private_key);
    
    try {
        std::string orders = robinhoodConnector.getOrders();
        std::cout << "Robinhood Holdings: " << orders << std::endl;

        // orders = robinhoodConnector.postOrder(robinhoodConnector.makeMarketOrderJson(
        //     "11299b2b-61e3-43e7-b9f7-dee77210bb29",
        //     true,
        //     "DOGE-USD",
        //     1.0
        // ));
        orders = robinhoodConnector.makeClientOrderId();

        std::cout << "\n\nOrder Response: " << orders << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error fetching Robinhood holdings: " << e.what() << std::endl;
    }

}