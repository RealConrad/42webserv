#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
    public:
        enum LogLevel {
            INFO,
            WARNING,
            ERROR
        };
        static void initialize(bool enableLogging, bool enableLogFile = false);
        static void setUseColour(bool value);
        static void log(const std::string& message, LogLevel level = INFO);
    private:
        static std::ostream* output;
        static std::ofstream fileStream;
        static bool loggingEnabled;
        static bool useColour;

        static std::string getLevelString(LogLevel level);
        static std::string getColor(LogLevel level);
        static void setLogFile(const std::string& filename);
};

#endif