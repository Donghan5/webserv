#include "../includes/CookieManager.hpp"

std::map<std::string, std::string> CookieManager::_cookies;

/*
	Parse cookie request
*/
void CookieMa::parseCookie(const std::string &cookieHeader) {
	std::istringstream ss(cookieHeader);
	std::string token;

	while (std::getline(ss, token, ';')) {
		size_t pos = token.find('=');
		if (pos != std::string::npos) {
			std::string key = token.substr(0, pos);
			std::string value = token.substr(pos + 1);

			trimString(key);
			trimString(value);

			if (!key.empty() && !value.empty())
				_cookies[key] = value;
		}
	}

	Logger::log(Logger::DEBUG, "Parsed cookies: ");
	for (string_map::iterator it = _cookies.begin(); it != _cookies.end(); ++it) {
		Logger::log(Logger::DEBUG, " - " + it->first + ": " + it->second);
	}
}

/*
	To send information of cookie
*/
void CookieManager::setCookie(const std::string &key, const std::string &value, int maxAge) {
	std::ostringstream cookie;

	cookie << key << '=' << value << "; Path=/; HttpOnly";
	if (maxAge > 0) {
		cookie << "; Max-Age= " << maxAge;
	}
	if (maxAge < 0) {
		Logger::log(Logger::ERROR, "Max-Age value must be positive number");
		throw std::logic_error("maxAge value must be positive number");
	}
	_cookies[key] = value;
	_headers["Set-Cookie"] = cookie.str();
}

/*
	Getting sepecific cookie
	if cannot find, return empty string
*/
void CookieManager::getCookie(const std::string &key) {
	if (_cookies.find(key) != _cookies.end()) {
		return _cookies[key];
	}
	return "";
}

void CookieManager::deleteCookie(const std::string &key) {
	_cookies.erase(key);
}

std::string CookieManager::getAllCookies(void) {
	std::ostringstream cookieHeader;
	std::map<std::string, std::string>::const_iterator it = _cookie.begin();
	for (; it != _cookie.end(); ++it) {
		cookieHeader << "Set-Cookie: " << it->second << "\r\n";
	}
	return cookieHeader.str();
}
