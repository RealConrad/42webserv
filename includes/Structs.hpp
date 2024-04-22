#ifndef STRUCTS_HPP
# define STRUCTS_HPP

#include <vector>
#include <map>
#include <string>
#include <ctime>

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
	std::vector<LocationConfig> locations;
	int keepAliveTimeout;
	int sendTimeout;
};

struct HTTPConfig {
	std::vector<ServerConfig> serverConfigs;
	int server_timeout_time;
	int keepAliveTimeout;
};

struct ClientState {
	std::string readBuffer;
	std::string writeBuffer;
	size_t totalRead;
	size_t contentLength;
	size_t headerEndIndex;
	bool headersComplete;
	bool keepAlive;
	time_t lastActivity;
	bool closeConnection;
	int serverPort;
	ServerConfig serverConfig;
	bool assignedConfig;
	bool responding;
	bool killTheChild;
	ClientState() :
		totalRead(0),
		contentLength(0), 
		headerEndIndex(0), 
		headersComplete(false), 
		closeConnection(false),
		assignedConfig(false),
		responding(false),
		killTheChild(false)
	{};
};

#endif