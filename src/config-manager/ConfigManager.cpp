#include "ConfigManager.hpp"

ConfigManager::ConfigManager() {
    this->httpConfig.server_timeout_time = -1;
}

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
    configFile.close();
    validateConfiguration();
}

/* -------------------------------------------------------------------------- */
/*                              Handle HTTP block                             */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseHttpSection(std::ifstream& configFile, std::string& line) {
    this->sectionStack.push(HTTP); // Add HTTP to the stack

    while (!sectionStack.empty()) {
        if (!getline(configFile, line)) {
            throw std::runtime_error("Configuration file is missing closing brace '}' for a section");
        }
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;
        std::string key, value;
        splitKeyValue(line, key, value);
        if (line == "}") {
            sectionStack.pop(); // Exiting current section
        } else if (key == "server_timeout_time") {
            if (value.empty())
                throw std::runtime_error("Value is missing for 'server_timeout_time'");
            this->httpConfig.server_timeout_time = convertStringToInt(value);
        } else if (line == "server {") {
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
        if (line.empty() || line[0] == '#')
            continue;

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
    } else if (key == "listen") {
        serverConfig.listenPort = convertStringToInt(value);
    } else if (key == "max_body_size") {
        serverConfig.clientMaxBodySize = convertStringToInt(value);
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
    
    checkLocationPath(line, locConfig);
    while (!this->sectionStack.empty()) {
        getline(configFile, line);
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

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

void ConfigManager::checkLocationPath(std::string& line, LocationConfig& locConfig) {
    std::string key, value;
    splitKeyValue(line, key, locConfig.locationPath);

    // After calling splitKeyValue, locConfig.locationPath contains "/path {"
    // First, let's trim the trailing "{" from locConfig.locationPath
    size_t bracePos = locConfig.locationPath.find('{');
    if (bracePos != std::string::npos) {
        locConfig.locationPath = locConfig.locationPath.substr(0, bracePos);
        locConfig.locationPath = trim(locConfig.locationPath); // Trim any whitespace left after removing "{"
    }

    // Check the number of parts in the location directive
    std::istringstream iss(line);
    std::vector<std::string> parts;
    std::string part;
    while (iss >> part) {
        parts.push_back(part);
    }

    // Expecting 3 parts: "location", "/path", and "{"
    if (parts.size() != 3 || locConfig.locationPath.empty()) {
        throw std::runtime_error("Invalid location path format: " + line);
    }
}

/* -------------------------------------------------------------------------- */
/*                               Validate Config                              */
/* -------------------------------------------------------------------------- */

void ConfigManager::validateConfiguration() {
    if (this->httpConfig.server_timeout_time == -1)
        throw std::runtime_error("Http config missing required 'server_timeout_time'");
    if (this->httpConfig.serverConfigs.size() == 0) {
        throw std::runtime_error("Http config missing required 'server'");
    }
    for (size_t i = 0; i < this->httpConfig.serverConfigs.size(); i++) {
        validateServerConfig(this->httpConfig.serverConfigs[i]);

        std::set<std::string> uniquePaths;
        for (size_t j = 0; j < this->httpConfig.serverConfigs[i].locations.size(); j++) {
            validateLocationConfig(this->httpConfig.serverConfigs[i].locations[j], uniquePaths);
        }
    }
}

void ConfigManager::validateServerConfig(ServerConfig& serverConfig) {
    if (serverConfig.listenPort == 0)
        throw std::runtime_error("Server config missing required 'listenPort'");
    if (serverConfig.rootDirectory.empty())
        throw std::runtime_error("Server config missing required 'rootDirectory'");
    if (serverConfig.indexFile.empty())
        throw std::runtime_error("Server config missing required 'index'");
    if (serverConfig.serverName.empty())
        throw std::runtime_error("Server config missing required 'server_name'");
}

void ConfigManager::validateLocationConfig(LocationConfig& locationConfig, std::set<std::string> uniquePaths) {
    
    if (locationConfig.allowedRequestTypes.empty()) {
        throw std::runtime_error("Location config missing required 'allowedRequestTypes'");
    }
    if (!uniquePaths.insert(locationConfig.locationPath).second) {
        // Insert failed, indicating a duplicate
        throw std::runtime_error("Duplicate path found: " + locationConfig.locationPath);
    }
}
