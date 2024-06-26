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
	// Reset key and value for each call
	key = "";
	value = "";

	// Find the first whitespace character to get the end of the key
	size_t keyEnd = line.find_first_of(" \t");
	// If there's no whitespace, assume the entire line is the key
	if (keyEnd == std::string::npos) {
		key = trim(line); // The entire line is the key
	} else {
		// Split the line into key and value
		key = trim(line.substr(0, keyEnd));
		size_t valueStart = line.find_first_not_of(" \t", keyEnd);
		if (valueStart != std::string::npos) {
			value = trim(line.substr(valueStart));
		}
	}
}

int convertStringToInt(const std::string& str) {
	std::istringstream iss(str);
	double temp;
	iss >> temp;
	if (iss.fail() || temp < 0 || temp > std::numeric_limits<int>::max())
		throw std::runtime_error("Cannot convert to int: " + str);
	return static_cast<int>(temp);
}

bool endsWith(const std::string& fullString, const std::string& ending) {
	// Check if the ending is longer than the full string
	if (ending.length() > fullString.length()) {
		return false;
	}
	// Compare the end of the full string with the ending
	return fullString.compare(fullString.length() - ending.length(), ending.length(), ending) == 0;
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
	std::cout << "FINISHED PRINTING HTTP CONFIG!" << std::endl;
}
