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
	size_t hash = hashTable(key);
	this->_hashedKeys.push_back(std::make_pair(hash, key));
	this->_settings[key] = value;
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

size_t ServerConf::hashTable(std::string key) const {
	size_t hash = 0;
	for (size_t i (0); i < key.length(); i++) {
		hash = hash * 31 + static_cast<size_t>(key[i]);
	}
	return hash;
}

std::string ServerConf::getData(std::string key) const {
	size_t hash = hashTable(key);

	for (size_t i (0); i < _hashedKeys.size(); i++) {
		if (_hashedKeys[i].first == hash && _hashedKeys[i].second == key) {
			return this->_settings.find(key)->second;
		}
	}
	return "";
}
