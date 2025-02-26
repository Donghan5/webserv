#include "../includes/CookieManager.hpp"

std::map<std::string, std::string> CookieManager::_cookies;

/*
	trim all the spaces
*/
static std::string &trimString(std::string &str) {
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

/*
	Parse cookie request
*/
void CookieManager::parseCookie(const std::string &cookieHeader) {
	std::istringstream ss(cookieHeader);
	std::string token;

	while (std::getline(ss, token, ';')) {
		size_t pos = token.find('=');

		if (pos == std::string::npos) {
			Logger::log(Logger::ERROR, "Cannot find '=' in this scope");
			continue;
		}
		std::string key = token.substr(0, pos);
		std::string value = token.substr(pos + 1);

		trimString(key);
		trimString(value);

		if (key == "session_id") {
			size_t sessionPos = value.find("session_id=");
			if (sessionPos != std::string::npos) {
				value = value.substr(sessionPos + 11);
			}
		}

		if (!key.empty() && !value.empty() && CookieManager::_cookies.find(key) == CookieManager::_cookies.end())
			CookieManager::_cookies[key] = value;
	}

	std::map<std::string, std::string>::iterator it = CookieManager::_cookies.begin();
	for (; it != CookieManager::_cookies.end(); ++it) {
		Logger::log(Logger::DEBUG, " - " + it->first + ": " + it->second);
	}
}

/*
	To send information of cookie
*/
std::string CookieManager::setCookie(const std::string &key, const std::string &value, int maxAge) {
	std::ostringstream cookie;

	cookie << key << '=' << value << "; Path=/; HttpOnly";
	if (maxAge > 0) {
		cookie << "; Max-Age= " << maxAge;
	}
	if (maxAge < 0) {
		Logger::log(Logger::ERROR, "Max-Age value must be positive number");
		throw std::logic_error("maxAge value must be positive number");
	}
	CookieManager::_cookies[key] = value;
	// return cookie.str();
	return "Set-Cookie: " + cookie.str() + "\r\n";
}

/*
	Getting sepecific cookie
	if cannot find, return empty string
*/
std::string CookieManager::getCookie(const std::string &key) {
	if (CookieManager::_cookies.find(key) != CookieManager::_cookies.end()) {
		std::string value = CookieManager::_cookies[key];
		if (key == "session_id") {
			size_t sessionPos = value.find("session_id=");
			if (sessionPos != std::string::npos) {
				value = value.substr(sessionPos + 11);
			}
		}
		return value;
	}
	return "";
}

void CookieManager::deleteCookie(const std::string &key) {
	CookieManager::_cookies.erase(key);
}

std::string CookieManager::getAllCookies(void) {
	std::ostringstream cookieHeader;
	std::map<std::string, std::string>::const_iterator it = CookieManager::_cookies.begin();
	for (; it != CookieManager::_cookies.end(); ++it) {
		cookieHeader << "Set-Cookie: " << it->second << "\r\n";
	}
	return cookieHeader.str();
}
