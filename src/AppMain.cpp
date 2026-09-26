#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ConfigManager.h"

int main(){
    std::cout << "Hello, Ride The Wave!" << std::endl;

    ConfigManager configManager;
    configManager.loadSettings("config.txt");

    for (const auto& setting : configManager.getSettings()) {
        std::cout << setting.first << ":" << setting.second << std::endl;
    }
}