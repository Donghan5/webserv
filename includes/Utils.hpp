#ifndef UTILS_HPP
#define UTILS_HPP

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

class Utils {
	public:
		static std::string intToString(int num);
		static std::string floatToString(float num);
		static void cleanUpDoublePointer(char **dptr);
};

#endif
