#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include "Structs.hpp"
#include "Utils.hpp"

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <limits>
#include <algorithm>

class ConfigManager {
	private:
		HTTPConfig httpConfig;
		std::stack<SectionTypes> sectionStack;
		std::vector<std::string> required;
		std::vector<std::string> defined;

		/**
		 * @brief Parses the HTTP section of the configuration file.
		 * 
		 * This method also handles the server block(s) within the HTTP section.
		 * 
		 * @param configFile A reference to the opened config file stream.
		 * @param line A string representing the current line being processed.
		 * @throws `std::runtime_error` If an unknown directive is encountered or section is incorrectly formatted.
		 */
		void parseHttpSection(std::ifstream& configFile, std::string& line);

		/**
		 * @brief Parses a server block within the configuration file.
		 * 
		 * This method reads directives specific to a server configuration, populating a ServerConfig
		 * object with the values found. It handles the initiation of parsing for location blocks and
		 * ensures all required server directives are provided.
		 * 
		 * @param configFile A reference to the ifstream of the configuration file.
		 * @param line A string reference to the current line being processed.
		 * @param serverConfig A reference to the ServerConfig object being populated.
		 * @throws `std::runtime_error` If an unknown directive is encountered, a directive is missing,
		 *         or the section is incorrectly formatted.
		 */
		void parseServerSection(std::ifstream& configFile, std::string& line, ServerConfig& serverConfig);
		
		/**
		 * @brief Processes a single server directive found within a server block.
		 * 
		 * This method takes a line representing a server directive, splits it into a key and value,
		 * and updates the server configuration accordingly. It also checks for duplicate keys and ensures
		 * required directives are provided.
		 * 
		 * @param line A string representing the line from the configuration file containing the directive.
		 * @param serverConfig A reference to the ServerConfig object being updated.
		 * @throws `std::runtime_error` If the key/value pair is malformed, a duplicate key is found, or an unknown key is encountered.
		 */
		void handleServerDirective(std::string& line, ServerConfig& serverConfig);
		
		/**
		 * @brief Parses a location block within a server block of the configuration file.
		 * 
		 * This method configures a LocationConfig object with directives found within a location block.
		 * It supports defining allowed request types for a specific location.
		 * 
		 * @param configFile A reference to the ifstream of the configuration file.
		 * @param line A string reference to the current line being processed.
		 * @param locConfig A reference to the LocationConfig object being populated.
		 * @throws `std::runtime_error` If an unknown directive is encountered or the section is incorrectly formatted.
		 */
		void parseLocationSection(std::ifstream& configFile, std::string& line, LocationConfig& locConfig);
		
		/**
		 * @brief Validates and extracts the path from a location directive.
		 * 
		 * This method checks the format of the location path, ensures it's valid, and extracts the path,
		 * removing any trailing '{'. It updates the LocationConfig object with the path.
		 * 
		 * @param line A string representing the line from the configuration file containing the location directive.
		 * @param locConfig A reference to the LocationConfig object being updated.
		 * @throws `std::runtime_error` If the location directive is incorrectly formatted.
		 */
		void checkLocationPath(std::string& line, LocationConfig& locConfig);

		/**
		 * @brief Initializes server configuration with default values.
		 * 
		 * This method setups initial values for the `ServerConfig` struct.
		 * 
		 * @param serverConfig A reference to a `ServerConfig` struct to be initialized.
		 */
		void initServerConfig(ServerConfig& serverConfig);
	public:
		ConfigManager();
		~ConfigManager();

		/**
		 * @brief Parses the given configuration file and loads settings into the ConfigManager.
		 * 
		 * This method opens the specified configuration file, reads it line by line, and processes
		 * each directive into the HTTPConfig structure.
		 * 
		 * @param configFilePath A string representing the path to the configuration file.
		 * @throws `std::runtime_error` If the configuration file cannot be opened or a parsing error occurs.
		 */
		void parseConfigFile(std::string configFilePath);

		/**
		 * @brief Validates the loaded server configuration.
		 * 
		 * This method checks for essential HTTP configuration settings, ensuring that all required
		 * directives have been provided and are correctly formatted. Specifically, it verifies that
		 * a server timeout has been set and that at least one server block exists.
		 * 
		 * @throws `std::runtime_error` If the configuration is invalid or incomplete.
		 */
		void validateConfiguration();

		/**
		 * @brief Retrieves the current HTTP configuration.
		 * 
		 * @return A reference to the HTTPConfig object containing the server's configuration settings.
		*/
		HTTPConfig& getConfig();
};

#endif