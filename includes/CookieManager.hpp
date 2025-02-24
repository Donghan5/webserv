#ifndef COOKIEMANAGER_HPP
#define COOKIEMANAGER_HPP
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>

class CookieManager {
	private: std::map<std::string, std::string> _cookies;

	public:
		static void setCookie(const std::string &key, const std::string &value, int maxAge);
		static void getCookie(const std::string &key);
		static void deleteCookie(const std::string &key);
		static std::string getAllCookies(void);
};


#endif
