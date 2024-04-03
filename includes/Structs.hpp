#ifndef STRUCTS_HPP
# define STRUCTS_HPP

#include <vector>
#include <map>

enum RequestTypes {
    GET,
    DELETE,
    POST,
};

enum SectionTypes {
    HTTP,
    SERVER,
    LOCATION,
    UNKNOWN,
};

struct LocationConfig {
    std::vector<RequestTypes> allowedRequestTypes;
    std::string locationPath;
};

struct ServerConfig {
    std::string indexFile;
    std::string serverName;
    int clientMaxBodySize;
    int listenPort;
    std::string rootDirectory;
    bool directoryListing;
    std::vector<LocationConfig> locations; // Would be keyed by path OR could just be vector
};

struct HTTPConfig {
    std::vector<ServerConfig> serverConfigs; // Would be keyed by listenPort OR could just be vector
    int server_timeout_time;
};

#endif