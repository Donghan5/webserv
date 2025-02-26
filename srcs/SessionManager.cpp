#include "../includes/SessionManager.hpp"
#include "../includes/Logger.hpp"

std::map< std::string, std::map<std::string, std::string> > SessionManager::_session;


/*
	Create new session key
*/
std::string SessionManager::createSession(void) {
	std::ostringstream sessionID;
	srand(time(0));
	for (size_t i = 0; i < 16; ++i) {
		char c = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"[rand() % 62];
		sessionID << c;
	}

	std::string newSessionID = sessionID.str();
	SessionManager::_session[newSessionID] = std::map<std::string, std::string>();

	return newSessionID;
}

/*
	setting session values
*/
void SessionManager::setSessionValue(const std::string &sessionID, const std::string &key, const std::string &value) {
	// Logger::log(Logger::DEBUG, "Setting session: " + sessionID + " key: " + key + " value: " + value);
	SessionManager::_session[sessionID][key] = value;
}

std::string SessionManager::getSessionValue(const std::string &sessionID, const std::string &key) {
	if (SessionManager::_session.find(sessionID) != SessionManager::_session.end() && SessionManager::_session[sessionID].find(key) != SessionManager::_session[sessionID].end()) {
		return SessionManager::_session[sessionID][key];
	}
	return ""; // Don't find
}

void SessionManager::deleteSessionValue(const std::string &sessionID) {
	SessionManager::_session.erase(sessionID);
}
