#include "Utils.hpp"

/* -------------------------------------------------------------------------- */
/*                           String helper functions                          */
/* -------------------------------------------------------------------------- */

std::string trim(const std::string& str) {
    const std::string whiteSpace = " \n\r\t\f\v"; // Include all whitespace characters

    size_t first = str.find_first_not_of(whiteSpace);
    if (first == std::string::npos)
        return ""; // Entire string is just whitespace

    size_t last = str.find_last_not_of(whiteSpace);
    return str.substr(first, (last - first + 1));
}

void splitKeyValue(const std::string& line, std::string& key, std::string& value) {
    // Find the first whitespace character that is not leading (to get the end of the key)
    size_t keyEnd = line.find_first_of(" \t");
    // Find the start of the value by finding the first non-whitespace character after the key
    size_t valueStart = line.find_first_not_of(" \t", keyEnd);

    if (keyEnd != std::string::npos) {
        key = line.substr(0, keyEnd);
        key = trim(key); // Just in case theres leading whitespace
    }

    if (valueStart != std::string::npos) {
        value = line.substr(valueStart);
        value = trim(value); // Remove potential trailing whitespace
    }
}

/* -------------------------------------------------------------------------- */
/*                              Enum conversions                              */
/* -------------------------------------------------------------------------- */

std::string requestTypeToString(RequestTypes type) {
    switch (type) {
        case GET: return "GET";
        case POST: return "POST";
        case DELETE: return "DELETE";
        default: return "Unknown";
    }
}

RequestTypes stringToRequestType(const std::string& str) {
    if (str == "GET") return GET;
    if (str == "DELETE") return DELETE;
    if (str == "POST") return POST;
    throw std::runtime_error("Unsupported request type: " + str);
}

/* -------------------------------------------------------------------------- */
/*                                  Printing                                  */
/* -------------------------------------------------------------------------- */

void printHTTPConfig(const HTTPConfig& config) {
    std::cout << "==========Printing HTTP Config==========" << std::endl;
    for (size_t i = 0; i < config.serverConfigs.size(); i++) {
        std::cout << std::endl;
        std::cout << "====PRINTING SERVER " << i << "====" << std::endl;
        std::cout << "Index File:\t\t" << config.serverConfigs[i].indexFile << std::endl;
        std::cout << "Server name:\t\t" << config.serverConfigs[i].serverName << std::endl;
        std::cout << "Max Body size:\t\t" << config.serverConfigs[i].clientMaxBodySize << std::endl;
        std::cout << "Port:\t\t\t" << config.serverConfigs[i].listenPort << std::endl;
        std::cout << "Root Directory:\t\t" << config.serverConfigs[i].rootDirectory << std::endl;
        std::cout << "Directory listing:\t" << (config.serverConfigs[i].directoryListing ? "true":"false") << std::endl;

        std::cout << std::endl;
        std::cout << "Location block for server: " << i << std::endl;
        for (size_t j = 0; j < config.serverConfigs[i].locations.size(); j++) {
            std::cout << "Location Path:\t\t" << config.serverConfigs[i].locations[j].locationPath << std::endl;
            
            std::cout << "Allowed:\t\t";
            for (size_t k = 0; k < config.serverConfigs[i].locations[j].allowedRequestTypes.size(); k++) {
                std::cout << requestTypeToString(config.serverConfigs[i].locations[j].allowedRequestTypes[k]) << " ";
            }
            std::cout << std::endl;
            std::cout << "===========" << std::endl;
        }
    }
    std::cout << std::endl;
    std::cout << "FINISHED HTTP CONFIG!" << std::endl;
}