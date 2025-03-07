#include "../includes/Utils.hpp"

std::string Utils::intToString(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

int Utils::stringToInt(const std::string& str) {
    std::stringstream ss(str);
    int result;
    ss >> result;
    return result;
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

std::vector<std::string> Utils::split(const std::string &str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		if (!token.empty()) {
			tokens.push_back(token);
		}
	}

	return tokens;
}
