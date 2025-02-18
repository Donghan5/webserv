#ifndef SERVERCONF_HPP
#define SERVERCONF_HPP
#pragma once

#include "LocationConf.hpp"
#include <string>
#include <map>
#include <vector>

class LocationConf;

typedef std::vector<LocationConf> location_vector;
typedef std::map<std::string, std::string> string_map;

class ServerConf {
	private:
		location_vector _locations;
		string_map _settings;

	public:
		ServerConf();
		ServerConf(const ServerConf &obj);
		~ServerConf();

		ServerConf &operator=(const ServerConf &obj);
		void addLocation(const LocationConf &location);
		const location_vector &getLocations() const;
		void setData(const std::string &key, const std::string &value);
		const string_map &getData(void) const;
		std::string getData(std::string key) const;
		std::string getRootDir(void) const;

		// To DEBUG
		void showServerData();
};

#endif
