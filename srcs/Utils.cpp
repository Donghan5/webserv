#include "../includes/utils.hpp"

std::string Utils::intToString(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string &Utils::trimString(std::string &str) {
	const std::string whitespace = " \t\r\n";
	size_t start = str.find_first_not_of(whitespace);
	size_t end = str.find_last_not_of(whitespace);

	if (start == std::string::npos) { // string is empty
		str.clear();
	} else {
		str = str.substr(start, end - start + 1);
	}

	if (!str.empty() && str[str.size() - 1] == '\r') {
		str.erase(str.size() - 1);
	}
	return str;
}
