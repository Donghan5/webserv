#ifndef HTTPCONF_HPP
#define HTTPCONF_HPP

#pragma once

#include "ServerConf.hpp"
#include <string>
#include <map>
#include <vector>

class ServerConf;

typedef std::vector<ServerConf> server_vector;
typedef std::map<std::string, std::string> string_map;

class HttpConf {
	private:
		server_vector _servers;
		string_map _settings;

	public:
		HttpConf();
		HttpConf(const HttpConf &obj);
		~HttpConf();

		HttpConf &operator=(const HttpConf &obj);
		void addServer(const ServerConf &server);;
		void setData(const std::string &key, const std::string &value);
		const server_vector &getServerConfig(void) const;

		std::string getData(std::string key) const;

		// to debug
		void showHttpData(void);
};

#endif
