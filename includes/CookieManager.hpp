#ifndef COOKIEMANAGER_HPP
#define COOKIEMANAGER_HPP
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <ctime>
#include <iomanip>
#include "Logger.hpp"

class CookieManager {
	private:
		static std::map<std::string, std::string> _cookies;

	public:
		static std::string setCookie(const std::string &key, const std::string &value, int maxAge);
		static std::string getCookie(const std::string &key);
		static void deleteCookie(const std::string &key);
		static std::string getAllCookies(void);
		static void parseCookie(const std::string &cookieHeader);
};

#endif
 