#include "../includes/LocationConf.hpp"

LocationConf::LocationConf() {} // constructor

LocationConf::~LocationConf() {} // destructor

LocationConf::LocationConf(const LocationConf &obj) {
	*this = obj;
}

LocationConf &LocationConf::operator=(const LocationConf &obj) {
	if (this != &obj) {
		this->_settings = obj._settings;
	}
	return (*this);
}

void LocationConf::setData(const std::string &key, const std::string &value) {
	this->_settings.insert(std::make_pair(key, value));
}

const string_map &LocationConf::getSettings(void) const {
	return this->_settings;
}

/*
	Print all data in locations
*/
void LocationConf::showLocationData(void) {
	string_map::iterator it = this->_settings.begin();
	for (; it != this->_settings.end(); ++it) {
		std::cout << "Key: " << it->first << " Value: " << it->second << std::endl;
	}
}
