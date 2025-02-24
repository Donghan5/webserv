#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP
#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <iomanip>

class SessionManager {
	private:
		static std::map< std::string, std::map<std::string, std::string> > _session;
	public:
		static std::string createSession(void);
		static void setSessionValue(const std::string &sessionID, const std::string &key, const std::string &value);
		static std::string getSessionValue(const std::string &sessionID, const std::string &key);
		static void deleteSessionValue(const std::string &sessionID);
};

#endif
