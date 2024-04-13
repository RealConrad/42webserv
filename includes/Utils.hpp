#ifndef UTILS_HPP
# define UTILS_HPP

# include <string>
# include "Structs.hpp"
# include <iostream>
# include <sstream>
# include <limits>

# include "Utils.tpp"

void splitKeyValue(const std::string& line, std::string& key, std::string& value);
std::string trim(const std::string& str);
bool endsWith(const std::string& fullString, const std::string& ending);



/* -------------------------------------------------------------------------- */
/*                                 Conversion                                 */
/* -------------------------------------------------------------------------- */

std::string requestTypeToString(RequestTypes type);
RequestTypes stringToRequestType(const std::string& str);
int convertStringToInt(const std::string& str);

/* -------------------------------------------------------------------------- */
/*                                  Printing                                  */
/* -------------------------------------------------------------------------- */

void printHTTPConfig(const HTTPConfig& config);

#endif