#pragma once

#include <map> 
#include <string>

class ConfigManager {
    public:
        ConfigManager();
        std::map<std::string, std::string> getSettings() const;
        void loadSettings(const std::string& filename);
        void saveSettings(const std::string& filename);
    
    private:
        std::map<std::string, std::string> settings;
};