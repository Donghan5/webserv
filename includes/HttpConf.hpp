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
typedef std::vector<std::pair<size_t, std::string> > hash_vector;

class HttpConf {
	private:
		server_vector _servers;
		string_map _settings;
		hash_vector _hashedKeys;

	public:
		HttpConf();
		HttpConf(const HttpConf &obj);
		~HttpConf();

		HttpConf &operator=(const HttpConf &obj);
		void addServer(const ServerConf &server);;
		void setData(const std::string &key, const std::string &value);
		const server_vector &getServerConfig(void) const;

		// Hash table
		size_t hashTable(std::string key) const;
		std::string getData(std::string key) const;
		
		// to debug
		void showHttpData(void);
};

#endif
