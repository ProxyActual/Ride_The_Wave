#include "ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ConfigManager::ConfigManager() {
    // Initialize settings map if needed
}

std::map<std::string, std::string> ConfigManager::getSettings() const {
    return settings;
}

void ConfigManager::loadSettings(const std::string& filename) {
    std::ifstream configFile(filename);
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            settings[key] = value;
        }
    }
}

void ConfigManager::saveSettings(const std::string& filename) {
    std::ofstream configFile(filename);
    for (const auto& pair : settings) {
        configFile << pair.first << ":" << pair.second << std::endl;
    }
}