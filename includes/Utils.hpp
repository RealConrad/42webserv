#ifndef UTILS_HPP
# define UTILS_HPP

#include <string>
#include "Structs.hpp"
#include <iostream>

void splitKeyValue(const std::string& line, std::string& key, std::string& value);
std::string trim(const std::string& str);
void printHTTPConfig(const HTTPConfig& config);
std::string requestTypeToString(RequestTypes type);
RequestTypes stringToRequestType(const std::string& str);

#endif