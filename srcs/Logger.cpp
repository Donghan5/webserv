#include "../includes/Logger.hpp"


// get current time
std::string Logger::getCurrentTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return std::string(buf);
}

// convert log level to string
std::string Logger::logLevelToString(LogLevel level) {
	switch(level) {
		case INFO: return std::string(BLUE) + "INFO" + std::string(END);
		case WARNING: return std::string(YELLOW) + "WARNING" + std::string(END);
		case ERROR: return std::string(RED) + "ERROR" + std::string(END);
		case DEBUG: return std::string(GREEN) + "DEBUG" + std::string(END);
		default: return "UNKNOWN";
	}
}

// Just print the log cout
void Logger::log(LogLevel level, const std::string &message) {

	std::string logEntry = "[" + Logger::getCurrentTime() + "] " + "[" + Logger::logLevelToString(level) + "] " + ": " + message;

	std::cout << logEntry << std::endl;
}

// separate error log using cerr
void Logger::cerrlog(LogLevel level, const std::string &message) {
	std::string logEntry = "[" + Logger::getCurrentTime() + "] " + "[" + Logger::logLevelToString(level) + "] " + ": " + message;

	std::cerr << logEntry << std::endl;
}
