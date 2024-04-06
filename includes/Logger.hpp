#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
    public:
        enum LogLevel {
            SUCCESS,
            INFO,
            WARNING,
            ERROR
        };

        /**
         * @brief Initializes the logger. Enables or disables logging and file logging based on parameters.
         * @param enableLogging Enables overall logging if true.
         * @param enableLogFile Enables logging to a file instead of the console if true.
         */
        static void initialize(bool enableLogging, bool enableLogFile = false);

        /**
         * @brief Sets whether log messages should include ANSI colour codes or not
         * @param value If true, log messages will include ANSI colour codes
         */
        static void setUseColour(bool value);

        /**
         * @brief Logs a message at a specified log level
         * @param message The message to log.
         * @param level The log level (default is set to INFO).
         */
        static void log(const std::string& message, LogLevel level = INFO);
    private:
        // Pointer to the output stream (console or file).
        static std::ostream* output;
        static std::ofstream fileStream;
        static bool loggingEnabled;
        static bool useColour;

        /**
         * @brief A string representation of the log level
         * @param level The level to convert to string.
         * @return String representation of the log level
         */
        static std::string getLevelString(LogLevel level);

        /**
         * @brief Returns the ANSI color code for the given log level
         * @param level The log level to get the colour code for.
         * @return ANSI color code as a string
         */
        static std::string getColor(LogLevel level);
        
        /**
         * @brief Sets the file to which log messages will be written to
         * @param filename The name of the file to use for logging
         */
        static void setLogFile(const std::string& filename);
};

#endif