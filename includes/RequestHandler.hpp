#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP
#pragma once

#include <iostream>
#include <string>
#include "ParsedRequest.hpp"
#include "FileHandler.hpp"
#include "Logger.hpp"
#include "CgiHandler.hpp"
#include "WebServConf.hpp"

#define REQUEST200 "HTTP/1.1 200 OK\r\n\r\nFile deleted successfully"
#define REQUEST201 "HTTP/1.1 201 Created\r\n\r\nFile uploaded successfully"
#define REQUEST403 "HTTP/1.1 403 Forbidden\r\n\r\nForbidden"
#define REQUEST404 "HTTP/1.1 404 Not Found\r\n\r\nFile not found"
#define REQUEST405 "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed"
#define REQUEST500 "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete file"


class RequestHandler {
	public:
		static bool file_exists(const std::string &path);
		static bool ends_with(const std::string &str, const std::string &suffix);
		static std::string process_request(const std::string &request);
		//sdsfsdf
};

#endif
