#include "../includes/ServerConf.hpp"

ServerConf::ServerConf() {} // constructor

ServerConf::~ServerConf() {} // destructor

ServerConf::ServerConf(const ServerConf &obj) {
	*this = obj;
}

ServerConf &ServerConf::operator=(const ServerConf &obj) {
	if (this != &obj) {
		this->_locations = obj._locations;
		this->_settings = obj._settings;
	}
	return (*this);
}

void ServerConf::addLocation(const LocationConf &location) {
	this->_locations.push_back(location);
}

const location_vector &ServerConf::getLocations(void) const {
	return this->_locations;
}

void ServerConf::setData(const std::string &key, const std::string &value) {
	this->_settings.insert(std::make_pair(key, value));
}

/*
	Print all data in serverconf
*/
void ServerConf::showServerData(void) {
	string_map::iterator it = this->_settings.begin();
	for (; it != this->_settings.end(); ++it) {
		std::cout << "Key: " << it->first << " Value: " << it->second << std::endl;
	}
}

std::string ServerConf::getData(std::string key) const {
	string_map::const_iterator it = this->_settings.find(key);
	if (it != _settings.end()) {
		return it->second;
	}
	return "";
}
