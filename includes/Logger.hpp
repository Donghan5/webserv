#ifndef LOGGER_HPP
#define LOGGER_HPP
#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <ctime>

// colors
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define END "\033[0m"


class Logger {
	public:
		enum LogLevel { INFO, WARNING, ERROR, DEBUG };
		static void log(LogLevel level, const std::string &message);
		static void cerrlog(LogLevel level, const std::string &message);

	private:
		static std::string getCurrentTime();
		static std::string logLevelToString(LogLevel level);
};

#endif
