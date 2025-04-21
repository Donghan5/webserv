#include "Utils.hpp"

std::string  Utils::intToString(int num) {
	std::ostringstream oss;
	oss << num;

	return oss.str();
}

std::string  Utils::floatToString(float num) {
	std::ostringstream oss;
	oss << num;

	return oss.str();
}
