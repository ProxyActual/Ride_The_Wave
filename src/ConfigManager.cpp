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

bool ConfigManager::loadSettings(const std::string& filename) {
    std::ifstream configFile(filename);
    if (!configFile) {
        return false;
    }

    settings.clear();
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            settings[key] = value;
        }
    }

    const auto apiKey = settings.find("api_key");
    if (apiKey == settings.end() || apiKey->second.empty()) {
        return false;
    }

    localConfig_.api_key = apiKey->second;
    localConfig_.public_key = settings["public_key"];
    localConfig_.private_key = settings["private_key"];

    return true;
}

void ConfigManager::saveSettings(const std::string& filename) {
    std::ofstream configFile(filename);
    for (const auto& pair : settings) {
        configFile << pair.first << ":" << pair.second << std::endl;
    }
}