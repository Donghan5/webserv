#include "Utils.hpp"
#include "AConfigBase.hpp"

STR  Utils::intToString(int num) {
	std::ostringstream oss;
	oss << num;

	return oss.str();
}

STR  Utils::floatToString(float num) {
	std::ostringstream oss;
	oss << num;

	return oss.str();
}
