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
#include "ConsoleWidget.h"

static constexpr int REFRESH_INTERVAL_SECONDS = 5 * 60;

std::vector<CryptoHolding> getCryptoHoldings(RobinhoodConnector& robinhoodConnector){
    std::vector<CryptoHolding> cryptoHoldings;
    auto holdings = robinhoodConnector.getHoldings();
    auto orders = robinhoodConnector.getOrders();
    for(auto holding : holdings) {
        double originalCostUSD = 0.0;
        double totalQuantity = 0.0;
        for (const auto& order : orders) {
            if (order.symbol == holding.asset_code + "-USD") {
                if(order.side == "buy"){
                    totalQuantity += order.filledAssetQuantity;
                    originalCostUSD += order.averagePrice * order.filledAssetQuantity;
                    std::cout << "Looking at order for " << order.symbol << " " << order.side << " with filled quantity: " << order.filledAssetQuantity ;
                    std::cout << " Total quantity so far: " << totalQuantity << " holding Quantity: " << holding.total_quantity << std::endl;
                    if (totalQuantity == holding.total_quantity) {
                        break;
                    }
                }
                else if(order.side == "sell"){
                    std::cout << "Ride the way does not currently handle complex sell and buy histories." << std::endl;
                    std::cout << "Please clean up so the history of " << holding.asset_code << " only consists of buy orders." << std::endl;
                }
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

void loadConsoleItems(ConfigManager& configManager){
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
}

std::string convertColor(std::vector<double> color){
    return "\x1b[38;2;" + std::to_string(static_cast<int>(color[0])) + ";" + std::to_string(static_cast<int>(color[1])) + ";" + std::to_string(static_cast<int>(color[2])) + "m";
}

int main(){
    static ConsoleUI consoleUI;
    std::cout << "Welcome to Ride The Wave!" << std::endl;

    std::cout << "Loading configuration..." << std::endl;

    ConfigManager configManager;
    loadConsoleItems(configManager);
    
    RobinhoodConnector robinhoodConnector(configManager.localConfig_.api_key, configManager.localConfig_.private_key);
    
    std::vector<CryptoHolding> cryptoHoldings = getCryptoHoldings(robinhoodConnector);

    std::map<CryptoHolding*, ConsoleWidget*> TextWidgetMap;

    ConsoleWidget Title(1, 1, 150, 4);

    Title.putText(0, 0, "======================================================", ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);
    Title.putText(0, 1, "||        Welcome to Ride The Wave!                 ||", ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);
    Title.putText(0, 2, "======================================================", ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);
    Title.putText(0, 3, "COIN    PROFIT", ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);

    int number = 5;
    for (CryptoHolding& cryptoHolding : cryptoHoldings) {
        ConsoleWidget* textWidget = new ConsoleWidget(1, number++, 150, 1);
        TextWidgetMap.insert({&cryptoHolding, textWidget});
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    consoleUI.clearScreen();

    Title.update();

    std::cout << "\n\n" << std::endl;


    while(true){
        bool sold = false;
        for(CryptoHolding& cryptoHolding : cryptoHoldings) {
            ConsoleWidget* textWidget = TextWidgetMap[&cryptoHolding];
            //std::cout << "Updating widget for " << cryptoHolding.getAssetCode() << std::endl;
            textWidget->putText(0, 0, cryptoHolding.getAssetCode(), ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);

            if(cryptoHolding.needsUpdate()){
                cryptoHolding.update();
            }

            double currentProfit = cryptoHolding.getCurrentProfit();
            std::string Quantity = "" + std::to_string(currentProfit);
            ConsoleWidget::Color profitColor = (currentProfit < 0) ? ConsoleWidget::RED : ConsoleWidget::WHITE;
            profitColor = (currentProfit > 2 + 1) ? ConsoleWidget::GREEN : profitColor;

            textWidget->putText(7, 0, Quantity, profitColor, ConsoleWidget::DEFAULT);

            textWidget->putText(18, 0, cryptoHolding.getTimeLeft(), ConsoleWidget::WHITE, ConsoleWidget::DEFAULT);

            ConsoleWidget::Color highestProfitColor = (cryptoHolding.getProfitPrecent() < 0) ? ConsoleWidget::RED : ConsoleWidget::WHITE;
            highestProfitColor = (cryptoHolding.getProfitPrecent() > 2) ? ConsoleWidget::GREEN : highestProfitColor;

            textWidget-> putText(30, 0, std::to_string(cryptoHolding.getHighMarketValueUSD()), highestProfitColor, ConsoleWidget::DEFAULT);

            if(cryptoHolding.getProfitPrecent() > 0){
                if(cryptoHolding.getHighMarketValueUSD() - cryptoHolding.getCurrentMarketValue() > 0.01){
                    cryptoHolding.sell();
                    sold = true;
                }
            }
            textWidget->update();
        }
        consoleUI.setCursorPosition(40, 0);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if(sold){
            cryptoHoldings = getCryptoHoldings(robinhoodConnector);
            consoleUI.clearScreen();
            Title.forceUpdate();
            for (CryptoHolding& cryptoHolding : cryptoHoldings) {
                ConsoleWidget* textWidget = TextWidgetMap[&cryptoHolding];
                textWidget->forceUpdate();
            }
        }
    }
}