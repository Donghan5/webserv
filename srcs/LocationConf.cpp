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
	size_t hash = hashTable(key);
	this->_hashedKeys.push_back(std::make_pair(hash, key));
	this->_settings[key] = value;
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

/*
	Inplement hash table
	@oaram
		key : target
	@return
		index of the key(target)
*/
size_t LocationConf::hashTable(std::string key) const {
	size_t hash = 0;
	for (size_t i (0); i < key.length(); i++) {
		hash = hash * 31 + static_cast<size_t>(key[i]); // basic hashing
	}
	return hash;
}

/*
	Get specify key
*/
std::string LocationConf::getData(std::string key) const {
	size_t hash = hashTable(key);

	for (size_t i(0); i < _hashedKeys.size(); i++) {
		if (_hashedKeys[i].first == hash && _hashedKeys[i].second == key) {
			return this->_settings.find(key)->second;
		}
	}
	return "";
}

const std::string &LocationConf::getPath(void) const {
	return _path;
}

void LocationConf::setPath(const std::string &path) {
	this->_path = path;
}
