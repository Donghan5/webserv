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

std::vector<std::string>	Utils::split(STR string, char delim, bool use_whitespaces_delim) {
	std::vector<std::string>	result;
	std::string					temp_line;
	std::istringstream	string_stream (string);

	// Split by any whitespace (>> skips at the beginning and then stops at any whitespace by default)
	if (use_whitespaces_delim) {
        std::string token;

        while (string_stream >> token) {
            result.push_back(token);
        }
		return result;
	}

	while (getline(string_stream >> std::ws, temp_line, delim)) {
		result.push_back(temp_line);
	}

	return result;
}
