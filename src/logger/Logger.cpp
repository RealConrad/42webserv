#include "Logger.hpp"

std::ostream* Logger::output = &std::clog;
std::ofstream Logger::fileStream;
bool Logger::loggingEnabled;
bool Logger::useColour;

void Logger::initialize(bool enableLogging, bool enableLogFile) {
    loggingEnabled = enableLogging;
    setUseColour(true);
    output = &std::clog;

    if (enableLogFile) {
        setLogFile("webserv.log");
    }
}

void Logger::setUseColour(bool value) {
    useColour = value;
}

void Logger::log(const std::string& message, LogLevel level) {
    if (!loggingEnabled)
        return;

    // Get current time
    std::time_t t = std::time(NULL);
    std::tm* localTime = std::localtime(&t); // Gets the current UTC timezone
    localTime->tm_hour += 2;
    if (localTime->tm_hour >= 24) { // Change day if necessary
        localTime->tm_hour -= 24;
        localTime->tm_mday += 1;
    }

    // Buffer to hold the time string
    char timeStr[6]; // HH:MM + null terminator
    std::strftime(timeStr, sizeof(timeStr), "%H:%M", localTime);

    std::string colorStart = "";
    std::string colorEnd = "";
    if (useColour) {
        colorStart = getColor(level);
        colorEnd = "\033[0m";
    }

    *output << colorStart << "[" << getLevelString(level) << "] " << "[" << timeStr << "] " << message << colorEnd << std::endl;
}


void Logger::setLogFile(const std::string& filename) {
    if (fileStream.is_open())
        fileStream.close();

    fileStream.open(filename.c_str());
    if (fileStream.fail()) {
        log("Failed to open log file: " + filename, ERROR);
        output = &std::cout;
        setUseColour(true); // Enable when logging out to console
    } else {
        output = &fileStream;
        setUseColour(false); // Disable color when logging to a file
    }
}

std::string Logger::getLevelString(LogLevel level) {
    switch (level) {
        case SUCCESS:
            return "SUCCESS";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            log("UNKNOWN LOG LEVEL!", ERROR);
            return "UNKNOWN";
    }
}

std::string Logger::getColor(LogLevel level) {
    switch (level) {
        case SUCCESS:
            return "\033[32m"; // Green
        case INFO:
            return "\033[34m"; // Blue
        case WARNING:
            return "\033[33m"; // Yellow
        case ERROR:
            return "\033[31m"; // Red
        default:
            return "\033[0m";  // Reset
    }
}