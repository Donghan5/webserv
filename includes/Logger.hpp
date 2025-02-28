#ifndef LOGGER_HPP
#define LOGGER_HPP
#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <ctime>

class Logger {
	public:
		enum LogLevel { INFO, WARNING, ERROR, DEBUG };
		static void log(LogLevel level, const std::string &message);
		static void init();

	private:
		static std::ofstream logFile;
		static std::string getCurrentTime();
		static std::string logLevelToString(LogLevel level);
};

#endif
