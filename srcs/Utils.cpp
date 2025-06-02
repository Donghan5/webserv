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

void Utils::cleanUpDoublePointer(char **dptr) {
	for (size_t i = 0; dptr[i] != NULL; i++) {
		free(dptr[i]);
	}
	delete[] dptr;
}

