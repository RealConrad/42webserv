#include "Logger.hpp"

std::ostream* Logger::output = &std::clog;
std::ofstream Logger::fileStream;
bool Logger::useColour;
bool Logger::debugEnabled;
bool Logger::successEnabled;
bool Logger::infoEnabled;
bool Logger::warningEnabled;
bool Logger::errorEnabled;
bool Logger::useFileLine;
bool Logger::useTimestamp;
bool Logger::useLevel;

Logger::Logger() {}

Logger::~Logger() {}

std::string Logger::makeVisible(const std::string& input) {
	std::ostringstream result;
	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];
		switch (c) {
			case '\n':
				result << "\\n\n";
				break;
			case '\t':
				result << "\\t\t";
				break;
			case '\r':
				result << "\\r";
				break;
			default:
				result << c;
				break;
		}
	}
	return result.str();
}

void Logger::initialize(bool enableLogging, bool enableLogFile) {
	debugEnabled = enableLogging;
	successEnabled = enableLogging;
	infoEnabled = enableLogging;
	warningEnabled = enableLogging;
	errorEnabled = enableLogging;
	useFileLine = enableLogging;
	useTimestamp = enableLogging;
	useLevel = enableLogging;
	setUseColour(true);
	output = &std::clog;

	if (enableLogFile) {
		setLogFile("webserv.log");
	}
}

void Logger::setUseColour(bool value) {
	useColour = value;
}

void Logger::hide(LogLevel level){
	if (level == DEBUG)
		debugEnabled = false;
	else if (level == SUCCESS)
		successEnabled = false;
	else if (level == INFO)
		infoEnabled = false;
	else if (level == WARNING)
		warningEnabled = false;
	else if (level == ERROR)
		errorEnabled = false;
}

void Logger::hideFileLine(){
	useFileLine = false;
}
void Logger::hideLevel(){
	useLevel = false;
}
void Logger::hideTimestamp(){
	useTimestamp = false;
}

void Logger::log(const std::string& message, const char* file, int line, LogLevel level) {
	if (level == DEBUG && !debugEnabled)
		return;
	if (level == SUCCESS && !successEnabled)
		return;
	if (level == INFO && !infoEnabled)
		return;
	if (level == WARNING && !warningEnabled)
		return;
	if (level == ERROR && !errorEnabled)
		return;

	std::time_t t = std::time(NULL);
	std::tm* localTime = std::localtime(&t);

	char timeStr[9];
	std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", localTime);
	std::string colorStart = "";
	std::string colorEnd = "";
	if (useColour) {
		colorStart = getColor(level);
		colorEnd = "\033[0m";
	}
	*output << colorStart;
	if (useTimestamp)
		*output << "[" << timeStr << "] ";
	if (useLevel)
		*output << "[" << getLevelString(level) << "]  \t";
	*output << message << colorEnd;
	if (useFileLine)
		*output << " [" << file << ":" << line << "]";
	*output << std::endl;
}


void Logger::setLogFile(const std::string& filename) {
	if (fileStream.is_open())
		fileStream.close();

	fileStream.open(filename.c_str());
	if (fileStream.fail()) {
		std::cerr << "Failed to open log file: " << filename << std::endl;
		output = &std::cout;
		setUseColour(true);
	} else {
		output = &fileStream;
		setUseColour(false);
	}
}

std::string Logger::getLevelString(LogLevel level) {
	switch (level) {
		case SUCCESS:
			return "SUCCESS";
		case DEBUG:
			return "DEBUG";
		case INFO:
			return "INFO";
		case WARNING:
			return "WARNING";
		case ERROR:
			return "ERROR";
		default:
			return "UNKNOWN";
	}
}

std::string Logger::getColor(LogLevel level) {
	switch (level) {
		case DEBUG:
			return "\033[38;5;208m"; // Orange
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
