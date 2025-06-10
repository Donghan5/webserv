#include "../includes/Logger.hpp"
#include "../includes/AConfigBase.hpp"

// get current time
STR Logger::getCurrentTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];

	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return STR(buf);
}

// convert log level to string
STR Logger::logLevelToString(LogLevel level) {
	switch(level) {
		case INFO: return STR(BLUE) + "INFO" + STR(END);
		case WARNING: return STR(YELLOW) + "WARNING" + STR(END);
		case ERROR: return STR(RED) + "ERROR" + STR(END);
		case DEBUG: return STR(GREEN) + "DEBUG" + STR(END);
		default: return "UNKNOWN";
	}
}

void Logger::log(LogLevel level, const STR &message) {
	if (level == DEBUG) {
		return ;
	}
	STR logEntry = "[" + Logger::getCurrentTime() + "] " + "[" + Logger::logLevelToString(level) + "] " + ": " + message;

	if (level == ERROR)
		std::cerr << logEntry << std::endl;
	else
		std::cout << logEntry << std::endl;
}
