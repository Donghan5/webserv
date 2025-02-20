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

/*
	Setting the data by using hash table
*/
void LocationConf::setData(const std::string &key, const std::string &value) {
	this->_settings.insert(std::make_pair(key, value));
}

/*
	Get all settings value
*/
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


std::string LocationConf::getData(std::string key) const {
	string_map::const_iterator it = this->_settings.find(key);
	if (it != _settings.end()) {
		return it->second;
	}
	return "";
}

const std::string &LocationConf::getPath(void) const {
	return _path;
}

void LocationConf::setPath(const std::string &path) {
	this->_path = path;
}

std::string LocationConf::getRootDir() const {
	return getData("root");
}

void LocationConf::setRootDir(const std::string &root) {
    this->_settings["root"] = root;
}

