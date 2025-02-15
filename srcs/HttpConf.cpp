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
	size_t hash = hashTable(key);
	this->_hashedKeys.push_back(std::make_pair(hash, key));
	this->_settings[key] = value;
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

size_t HttpConf::hashTable(std::string key) const {
	size_t hash = 0;
	for (size_t i (0); i < key.length(); i++) {
		hash = hash * 31 + static_cast<size_t>(key[i]);
	}
	return hash;
}

std::string HttpConf::getData(std::string key) const {
	size_t hash = hashTable(key);
	for (size_t i (0); i < _hashedKeys.size(); i++) {
		if (_hashedKeys[i].first == hash && _hashedKeys[i].second == key) {
			return this->_settings.find(key)->second;
		}
	}
	return "";
}
