#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <limits>
#include "Structs.hpp"
#include "Utils.hpp"

class ConfigManager {
    private:
        HTTPConfig httpConfig;
        std::stack<SectionTypes> sectionStack;

        void parseHttpSection(std::ifstream& configFile, std::string& line);

        void parseServerSection(std::ifstream& configFile, std::string& line, ServerConfig& serverConfig);
        void handleServerDirective(std::string& line, ServerConfig& serverConfig);

        void parseLocationSection(std::ifstream& configFile, std::string& line, LocationConfig& locConfig);
    public:
        ConfigManager();
        ~ConfigManager();
        void parseConfigFile(std::string configFilePath);
        HTTPConfig& getConfig();
};

#endif