#ifndef LOCATION_HPP
#define LOCATION_HPP
#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "Logger.hpp"


class Location {
	public:
		std::string path;
		std::string root;
		std::string alias;
		std::vector<std::string> index;
		bool autoindex;
		std::vector<std::string> allowed_methods;
		std::string return_url;
		size_t client_max_body_size;
		bool deny_all;
		std::string fastcgi_pass;
		std::string fastcgi_index;
		std::vector<std::string> fastcgi_params;
		std::string rewrite_url;
		bool rewrite_permanent;
		std::map<int, std::string> error_pages;

		static const size_t MAX_PATH_LENGTH = 4096;
		static const size_t MAX_BODY_SIZE = 100 * 1024 * 1024; // 100MB
		static const size_t DEFAULT_BODY_SIZE = 1024 * 1024;   // 1MB

		Location();
		~Location();

		Location(const Location& other);

		Location& operator=(const Location& other);
};

#endif
