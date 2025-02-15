#include "../includes/ServerConf.hpp"

ServerConf::ServerConf() {} // constructor

ServerConf::~ServerConf() {} // destructor

ServerConf::ServerConf(const ServerConf &obj) {
	*this = obj;
}

ServerConf &ServerConf::operator=(const ServerConf &obj) {
	if (this != &obj) {
		this->_locaitons = obj._locations;
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

void ServerConf::setData(const std:string &key, const std::string &value) {
	this->_settings.insert(std::make_pair(key, value));
}

const string_map &ServerConf::getData(void) const {
	return this->_settings;
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
