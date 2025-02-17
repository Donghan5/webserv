#include "../includes/HttpConf.hpp"

HttpConf::HttpConf() {} // constructor

HttpConf::~HttpConf() {} // destructor

HttpConf::HttpConf(const HttpConf &obj) {
	*this = obj;
}

HttpConf &HttpConf::operator=(const HttpConf &obj) {
	if (this != &obj) {
		this->_servers = obj._servers;
		this->_settings = obj._settings;
	}
	return (*this);
}

/*
	Add the number of server
*/
void HttpConf::addServer(const ServerConf &server) {
	this->_servers.push_back(server);
}

/*
	Getting the data
*/
const server_vector &HttpConf::getServerConfig(void) const {
	return this->_servers;
}

/*
	Setting the data
*/
void HttpConf::setData(const std::string &key, const std::string &value) {
	this->_settings.insert(std::make_pair(key, value));
}

/*
	Showing the data in HTTP conf
*/
void HttpConf::showHttpData(void) {
	string_map::iterator it = this->_settings.begin();
	for(; it != this->_settings.end(); ++it) {
		std::cout << "Key: " << it->first << " Value: " << it->second << std::endl;
	}
}

std::string HttpConf::getData(std::string key) const {
	string_map::const_iterator it = this->_settings.find(key);
	if (it != _settings.end()) {
		return it->second;
	}
	return "";
}
