#include "ConfigManager.hpp"

ConfigManager::ConfigManager() {}

ConfigManager::~ConfigManager() {}

HTTPConfig& ConfigManager::getConfig() {
    return this->httpConfig;
}

void ConfigManager::parseConfigFile(std::string configFilePath) {
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configFilePath);
    }

    std::string line;
    while (getline(configFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        if (line == "http {") { // Entering HTTP section
            parseHttpSection(configFile, line);
        } else {
            throw std::runtime_error("Unexpected line: " + line);
        }
    }
}


/* -------------------------------------------------------------------------- */
/*                              Handle HTTP block                             */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseHttpSection(std::ifstream& configFile, std::string& line) {
    this->sectionStack.push(HTTP); // Add HTTP to the stack

    while (!sectionStack.empty()) {
        getline(configFile, line);
        line = trim(line);
        std::string key, value;
        splitKeyValue(line, key, value);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == "}") {
            sectionStack.pop(); // Exiting current section
        } else if (key == "server_timeout_time") { // TODO: CLEANUP create function to convert type
            std::istringstream iss(value);
            double temp;
            iss >> temp;
            if (iss.fail() || temp < 0 || temp > std::numeric_limits<int>::max())
                throw std::runtime_error("Invalid port " + value);
            this->httpConfig.server_timeout_time = temp;
        } else if (line.find("server") == 0) {
            ServerConfig serverConfig;
            parseServerSection(configFile, line, serverConfig);
            this->httpConfig.serverConfigs.push_back(serverConfig);
        } else {
            throw std::runtime_error("Unknown http directive: " + line);
        }
    }
}

/* -------------------------------------------------------------------------- */
/*                             Handle Server Block                            */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseServerSection(std::ifstream& configFile, std::string& line, ServerConfig& serverConfig) {
    this->sectionStack.push(SERVER); // Entering Server section, so add it to stack

    while (!this->sectionStack.empty()) {
        getline(configFile, line);
        line = trim(line);
        if (line == "}") {
            this->sectionStack.pop(); // Exiting current section
            break; // Break since we're done with this server section
        } else if (line.find("location") == 0) {
            LocationConfig locConfig;
            parseLocationSection(configFile, line, locConfig);
            serverConfig.locations.push_back(locConfig);
        } else {
            handleServerDirective(line, serverConfig);
        }
    }
}

void ConfigManager::handleServerDirective(std::string& line, ServerConfig& serverConfig) {
    std::string key, value;
    splitKeyValue(line, key, value);

    if (key.empty() || value.empty())
        throw std::runtime_error("Could not find key or value for Server directive: " + line);

    if (key == "index") {
        serverConfig.indexFile = value;
    } else if (key == "server_name") {
        serverConfig.serverName = value;
    } else if (key == "listen") { // TODO: CLEANUP create function to convert type
        std::istringstream iss(value);
        double temp;
        iss >> temp;
        if (iss.fail() || temp < 0 || temp > std::numeric_limits<int>::max())
            throw std::runtime_error("Invalid port " + value);
        serverConfig.listenPort = temp;
    } else if (key == "max_body_size") { // TODO: CLEANUP create function to convert type
        std::istringstream iss(value);
        double temp;
        iss >> temp;
        if (iss.fail() || temp < 0 || temp > std::numeric_limits<int>::max())
            throw std::runtime_error("Invalid port " + value);
        serverConfig.clientMaxBodySize = temp;
    } else if (key == "root") {
        serverConfig.rootDirectory = value;
    } else if (key == "directory_listing") {
        serverConfig.directoryListing = (value == "true");
    } else {
        throw std::runtime_error("Unknown server key: " + key);
    }
}

/* -------------------------------------------------------------------------- */
/*                            Handle Location Block                           */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseLocationSection(std::ifstream& configFile, std::string& line, LocationConfig& locConfig) {
    this->sectionStack.push(LOCATION); // Entering location section
    
    std::string locationPath;
    // locationPath is dummy value and will not be used. Only created to make function work
    splitKeyValue(line, locationPath, locConfig.locationPath);
    // locConfig.locationPath contains "<path> {". remove the "{":
    size_t bracePos = locConfig.locationPath.find('{');
    if (bracePos != std::string::npos) {
        locConfig.locationPath = locConfig.locationPath.substr(0, bracePos);
        locConfig.locationPath = trim(locConfig.locationPath); // Trim any trailing whitespace left after removing "{"
    }

    while (!this->sectionStack.empty()) {
        getline(configFile, line);
        line = trim(line);

        if (line == "}") {
            this->sectionStack.pop(); // Exit current section
            break; // Break since we are done with location section
        } else {
            std::string key, value;
            splitKeyValue(line, key, value);

            if (key == "request_types") {
                std::istringstream iss(value);
                std::string requestType;
                while (iss >> requestType) { // will automatically split it by whitespace
                    RequestTypes type = stringToRequestType(trim(requestType));
                    locConfig.allowedRequestTypes.push_back(type);
                } 
            } else {
                throw std::runtime_error("Unknown key in Location section: " + key);
            }
        }
    }
}
