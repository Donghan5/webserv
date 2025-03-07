#ifndef UTILS_HPP
#define UTILS_HPP
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

class Utils {
	public:
		static std::string intToString(int num);
		static int stringToInt(const std::string& str);
		static std::string &trimString(std::string &str);
		static std::vector<std::string> split(const std::string &str, char delimiter);
};

#endif
