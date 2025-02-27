#ifndef UTILS_HPP
#define UTILS_HPP
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>

class Utils {
	public:
		static std::string intToString(int num);
		static std::string &trimString(std::string &str);
};

#endif
