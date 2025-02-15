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

class LocationConf {
	private:
		string_map _settings;

	public:
		LocationConf();
		LocationConf(const LocationConf &obj);
		~LocationConf();

		LocationConf &operator=(const LocationConf &obj);
		void setData(const std::string &key, const std::string &value);
		const string_map &getSettings() const;

	// to debug
		void showLocationData(void);
};

#endif
