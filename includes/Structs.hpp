#ifndef STRUCTS_HPP
# define STRUCTS_HPP

#include <vector>
#include <map>
#include <string>

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

struct ClientState {
	std::string readBuffer;
	std::string writeBuffer;
	bool keepAlive;
	time_t lastActivity;
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
	std::vector<LocationConfig> locations;
};

struct HTTPConfig {
	std::vector<ServerConfig> serverConfigs;
	int server_timeout_time;
};

#endif