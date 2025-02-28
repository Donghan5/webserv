#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP
#pragma once

#include <iostream>
#include <string>
#include "ParsedRequest.hpp"
#include "FileHandler.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"

class RequestHandler {
	public:
		static bool file_exists(const std::string &path);
		static bool ends_with(const std::string &str, const std::string &suffix);
		static std::string process_request(const std::string &request);
		//sdsfsdf
};

#endif
