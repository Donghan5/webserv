#ifndef PARSECONF_HPP
#define PARSECONF_HPP
#pragma once

#include "HttpConf.hpp"
#include "EventConf.hpp"
#include "ServerConf.hpp"
#include "LocationConf.hpp"

#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <string>


typedef std::map<std::string, std::string> string_map;
typedef std::vector<std::string> string_vector;

enum e_type_key {
	KEY_HTTP,
	KEY_SERVER,
	KEY_LOCATION,
	KEY_CLOSING_BRACE,
	KEY_EVENT,
	KEY_UNKNOWN
};

class ParseConf {
	private:
		std::string _confFileName;
		std::vector<HttpConf> httpblocks;
		HttpConf _httpConfig;
		EventConf _eventConfig;

		ParseConf(); // default constructor

	public:
		ParseConf(std::string confFileName);
		ParseConf(const ParseConf &obj);
		~ParseConf();

		ParseConf &operator=(const ParseConf &obj);

		// DEFAULT FUNCITON PARSE ETC...
		void confParse(const std::string &confFileName);
		string_vector confReadToken(std::ifstream &file);
		string_map confParseHandler(string_vector &token);

		// GETTER
		e_type_key getKeyType(const std::string &key);
		const HttpConf &getHttpConfig() const;
		const EventConf &getEventConfig() const;

		// handler func
		void handleHttpBlock(std::ifstream &file);
		void handleServerBlock(std::ifstream &file, HttpConf &httpConfig);
		void handleLocationBlock(std::ifstream &file, ServerConf &serverConfig);
		void handleEventBlock(std::ifstream &file);

		// increment
		void addHttp(const HttpConf &http);
};

#endif
