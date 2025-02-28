#include "../includes/Logger.hpp"

std::ofstream Logger::logFile;

/*
	Initialize and create log folder (if not exist)
*/
void Logger::init() {
	struct stat info;
	if (stat("logs", &info) != 0) {
		mkdir ("logs", 0777);
	}
	logFile.open("logs/webserv.log", std::ios::app);
}

/*
	Return current time
*/
std::string Logger::getCurrentTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return std::string(buf);
}

/*
	Convert to string log-level
*/
std::string Logger::logLevelToString(LogLevel level) {
	switch(level) {
		case INFO: return "INFO";
		case WARNING: return "WARNING";
		case ERROR: return "ERROR";
		case DEBUG: return "DEBUG";
		default: return "UNKNOWN";
	}
}

/*
	Save the log
*/
void Logger::log(LogLevel level, const std::string &message) {
	std::string logEntry = "[" + Logger::getCurrentTime() + "] " + Logger::logLevelToString(level) + ": " + message;

	if (logFile.is_open()) {
		logFile << logEntry << std::endl;
		logFile.flush();
	}

	std::cout << logEntry << std::endl;
}
