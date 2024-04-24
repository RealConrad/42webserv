#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream> 
#include <ctime>

class Logger;

#define DEBUG(message) { \
	std::ostringstream oss; \
	oss << message; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::DEBUG); \
}
#define BLOCK(message) { \
	std::ostringstream oss; \
	  std::string visibleText = Logger::makeVisible(message); \
	oss << std::endl << std::endl << visibleText << "*END*" << std::endl << std::endl; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::DEBUG); \
}
#define SUCCESS(message) { \
	std::ostringstream oss; \
	oss << message; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::SUCCESS); \
}
#define INFO(message) { \
	std::ostringstream oss; \
	oss << message; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::INFO); \
}
#define WARNING(message) { \
	std::ostringstream oss; \
	oss << message; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::WARNING); \
}
#define ERROR(message) { \
	std::ostringstream oss; \
	oss << message; \
	Logger::log(oss.str(), __FILE__, __LINE__, Logger::ERROR); \
}

class Logger {
	public:
		enum LogLevel {
			DEBUG,
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
		 * @brief does not print to output or file a specific level from logging.
		 * @param level level of logging to hide. 
		 */
		static void hide(LogLevel level);

		/**
		 * @brief does not print to output or file a File and Line part of log (Code link at the end).
		 */
		static void hideFileLine();

		/**
		 * @brief does not print to output or file a Timestamp part of log (time in front of message).
		 */
		static void hideTimestamp();

		/**
		 * @brief does not print to output or file a Level part of log (INFO/DEBUG/ERROR).
		 */
		static void hideLevel();

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
		static void log(const std::string& message, const char* file, int line, LogLevel level = INFO);
		static std::string makeVisible(const std::string& input);
		~Logger();
	private:
		Logger(); // Do not make the class instatiable
		// Pointer to the output stream (console or file).
		static std::ostream* output;
		static std::ofstream fileStream;
		static bool debugEnabled;
		static bool successEnabled;
		static bool infoEnabled;
		static bool warningEnabled;
		static bool errorEnabled;
		static bool useColour;
		static bool useFileLine;
		static bool useTimestamp;
		static bool useLevel;

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