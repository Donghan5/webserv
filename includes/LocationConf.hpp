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
		std::string _path;

	public:
		LocationConf();
		LocationConf(const LocationConf &obj);
		~LocationConf();

		LocationConf &operator=(const LocationConf &obj);
		void setData(const std::string &key, const std::string &value);
		std::string getData(std::string key) const;
		const string_map &getSettings() const;
		std::string getRootDir(void) const;


	// get and set the path
	const std::string &getPath(void) const;
	void setPath(const std::string &path);

	// to debug
		void showLocationData(void);
};

#endif
