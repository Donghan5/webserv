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
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t now = tv.tv_sec;
	struct tm tstruct = *localtime(&now);

	char buf[80];
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	int milliseconds = tv.tv_usec / 1000;

	std::ostringstream oss;
	oss << buf << "." << std::setw(3) << std::setfill('0') << milliseconds;

	return oss.str();
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
