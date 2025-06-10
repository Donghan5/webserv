#ifndef PARSERUTILS_HPP
#define PARSERUTILS_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <limits.h>
#include "HttpConfig.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

class ParserUtils {
	public:
		static int verifyPort(std::string port_str);
		static bool verifyAutoIndex(std::string autoindex_str);
		static long long verifyClientMaxBodySize(std::string client_max_body_size_str);
		static bool isDirectiveOk(std::string line, int start, int end);
		static bool isBlockOk(std::string line, int start, int end);
		static bool isBlockEndOk(STR line, int start);
		static bool check_location_path_duplicate(STR new_path, MAP<STR, LocationConfig*> locs);
		static bool minimum_value_check(HttpConfig *conf);
};

#endif // PARSERUTILS_HPP
