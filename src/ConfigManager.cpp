#include "ConfigManager.hpp"
#include "Logger.hpp"

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
		if (line == "http {") {
			parseHttpSection(configFile, line);
		} else {
			throw std::runtime_error("Unexpected line: " + line);
		}
	}
	configFile.close();
	validateConfiguration();
	SUCCESS("Parsing Config was successful");
}

void ConfigManager::initServerConfig(ServerConfig& serverConfig) {
	serverConfig.clientMaxBodySize = 100;
	serverConfig.directoryListing = false;

	this->required.clear();
	this->defined.clear();
	this->required.push_back("index");
	this->required.push_back("root");
	this->required.push_back("server_name");
	this->required.push_back("listen");
}

/* -------------------------------------------------------------------------- */
/*                              Handle HTTP block                             */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseHttpSection(std::ifstream& configFile, std::string& line) {
	this->sectionStack.push(HTTP);

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
			sectionStack.pop();
		} else if (key == "server_timeout_time") {
			if (value.empty())
				throw std::runtime_error("Value is missing for 'server_timeout_time'");
			this->httpConfig.server_timeout_time = convertStringToInt(value);
		} else if (line == "server {") {
			ServerConfig serverConfig;
			initServerConfig(serverConfig);
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
	this->sectionStack.push(SERVER);

	while (!this->sectionStack.empty()) {
		getline(configFile, line);
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;

		if (line == "}") {
			this->sectionStack.pop();
			break;
		} else if (line.find("location") == 0) {
			LocationConfig locConfig;
			parseLocationSection(configFile, line, locConfig);
			serverConfig.locations.push_back(locConfig);
		} else {
			handleServerDirective(line, serverConfig);
		}
	}
	if (!this->required.empty()){
		std::string missing = this->required[0];
		for (size_t i = 1; i < this->required.size(); ++i) {
			missing += " " + this->required[i];
		}
		throw std::runtime_error("Server config missing required elements: " + missing);
	}
}

void ConfigManager::handleServerDirective(std::string& line, ServerConfig& serverConfig) {
	std::string key, value;
	splitKeyValue(line, key, value);

	if (key.empty() || value.empty())
		throw std::runtime_error("Could not find key or value for Server directive: " + line);
	if (std::find(this->defined.begin(), this->defined.end(), key) != this->defined.end())
		throw std::runtime_error("Duplicate key found: " + key);
	if (key == "index") {
		serverConfig.indexFile = value;
	} else if (key == "server_name") {
		serverConfig.serverName = value;
	} else if (key == "listen") {
		serverConfig.listenPort = convertStringToInt(value);
	} else if (key == "keepalive_timeout") {
		serverConfig.keepAliveTimeout = convertStringToInt(value);
	} else if (key == "send_timeout") {
		serverConfig.sendTimeout = convertStringToInt(value);
	} else if (key == "max_body_size") {
		serverConfig.clientMaxBodySize = convertStringToInt(value);
	} else if (key == "root") {
		serverConfig.rootDirectory = value;
	} else if (key == "directory_listing") {
		serverConfig.directoryListing = (value == "true");
	} else {
		throw std::runtime_error("Unknown server key: " + key);
	}
	this->required.erase(std::remove(this->required.begin(), this->required.end(), key), this->required.end());
	this->defined.push_back(key);
}

/* -------------------------------------------------------------------------- */
/*                            Handle Location Block                           */
/* -------------------------------------------------------------------------- */

void ConfigManager::parseLocationSection(std::ifstream& configFile, std::string& line, LocationConfig& locConfig) {
	this->sectionStack.push(LOCATION);
	std::string key, value;
	
	checkLocationPath(line, locConfig);
	while (!this->sectionStack.empty()) {
		getline(configFile, line);
		line = trim(line);
		if (line.empty() || line[0] == '#')
			continue;
		if (line == "}") {
			this->sectionStack.pop();
			break;
		} else {
			splitKeyValue(line, key, value);
			if (key == "request_types") {
				std::istringstream iss(value);
				std::string requestType;
				while (iss >> requestType) {
					RequestTypes type = stringToRequestType(trim(requestType));
					locConfig.allowedRequestTypes.push_back(type);
				} 
			} else if (key == "redirection") {
				locConfig.redirection = value;
			} else {
				throw std::runtime_error("Unknown key in Location section: " + key);
			}
		}
	}
}

void ConfigManager::checkLocationPath(std::string& line, LocationConfig& locConfig) {
	std::string key;
	splitKeyValue(line, key, locConfig.locationPath);

	size_t bracePos = locConfig.locationPath.find('{');
	if (bracePos != std::string::npos) {
		locConfig.locationPath = locConfig.locationPath.substr(0, bracePos);
		locConfig.locationPath = trim(locConfig.locationPath);
	}
	std::istringstream iss(line);
	std::vector<std::string> parts;
	std::string part;
	while (iss >> part) {
		parts.push_back(part);
	}
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
}
