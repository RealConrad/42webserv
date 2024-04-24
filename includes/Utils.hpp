#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include "Structs.hpp"
# include <iostream>
# include <sstream>
# include <limits>

# include "Utils.tpp"

/**
 * @brief Splits a string into key and value based on a delimiter (whitespace).
 * 
 * @param line The input string to be split.
 * @param key Reference to a string variable to store the key.
 * @param value Reference to a string variable to store the value.
 */
void splitKeyValue(const std::string& line, std::string& key, std::string& value);

/**
 * @brief Trims leading and trailing whitespace from a string.
 * 
 * @param str The string to be trimmed.
 * @return The trimmed string.
 */
std::string trim(const std::string& str);

/**
 * Checks if a given string ends with a specified ending.
 *
 * @param fullString The full string to check.
 * @param ending The ending to compare with.
 * @return True if the full string ends with the specified ending, false otherwise.
 */
bool endsWith(const std::string& fullString, const std::string& ending);

/**
 * Converts a RequestTypes enum value to its corresponding string representation.
 *
 * @param type The RequestTypes enum value to convert.
 * @return The string representation of the given RequestTypes value.
 */
std::string requestTypeToString(RequestTypes type);

/**
 * @brief Converts a string to a RequestTypes enum value.
 * 
 * This function takes a string as input and converts it to the corresponding
 * RequestTypes enum value. The string should represent a valid request type,
 * such as "GET", "POST", "PUT", etc.
 * 
 * @param str The string to be converted.
 * @return The RequestTypes enum value corresponding to the input string.
 */
RequestTypes stringToRequestType(const std::string& str);

/**
 * @brief Converts a string to an integer.
 *
 * This function takes a string as input and converts it to an integer.
 *
 * @param str The string to be converted.
 * @return The converted integer value.
 */
int convertStringToInt(const std::string& str);

/**
 * Prints the HTTP configuration.
 *
 * @param config The HTTPConfig object containing the configuration to be printed.
 */
void printHTTPConfig(const HTTPConfig& config);

#endif