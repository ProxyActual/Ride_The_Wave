#pragma once

#include <map> 
#include <string>

class ConfigManager {
    public:
        struct Config {
            std::string api_key;
            std::string public_key;
            std::string private_key;
        } localConfig_;

        ConfigManager();
        std::map<std::string, std::string> getSettings() const;
        bool loadSettings(const std::string& filename);
        void saveSettings(const std::string& filename);
    
    private:
        std::map<std::string, std::string> settings;
};