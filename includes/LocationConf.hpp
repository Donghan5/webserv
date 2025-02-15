#ifndef LOCATIONCONF_HPP
#define LOCATIONCONF_HPP
#pragma once

#include "ParseConf.hpp"
#include "HttpConf.hpp"
#include "ServerConf.hpp"
#include <string>
#include <map>
#include <vector>

typedef std::map<std::string, std::string> string_map;
typedef std::vector<std::pair<size_t, std::string> > hash_vector;

class LocationConf {
	private:
		string_map _settings;
		hash_vector _hashedKeys;

	public:
		LocationConf();
		LocationConf(const LocationConf &obj);
		~LocationConf();

		LocationConf &operator=(const LocationConf &obj);
		void setData(const std::string &key, const std::string &value);
		std::string getData(std::string key) const;
		const string_map &getSettings() const;

	// hash table
		size_t hashTable(std::string key) const;
	// to debug
		void showLocationData(void);
};

#endif
