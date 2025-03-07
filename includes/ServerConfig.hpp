#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP
#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "Logger.hpp"

class ServerConfig {
	public:
		std::string host;
		int port;
		std::vector<std::string> server_names;
		std::map<std::string, Location> locations;
		std::string root;
		std::vector<std::string> index;
		size_t client_max_body_size;
		bool ssl_enabled;
		std::string ssl_certificate;
		std::string ssl_certificate_key;
		std::map<int, std::string> error_pages;

		// Add safety limits
		static const size_t MAX_SERVER_NAMES = 10;
		static const size_t MAX_LOCATIONS = 20;
		static const size_t DEFAULT_BODY_SIZE = 1024 * 1024;

		ServerConfig();

		~ServerConfig();

		ServerConfig(const ServerConfig& other);

		ServerConfig& operator=(const ServerConfig& other);
};

#endif
