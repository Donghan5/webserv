#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include <map>

#define REQUEST200 "HTTP/1.1 200 OK\r\n\r\nFile deleted successfully"
#define REQUEST201 "HTTP/1.1 201 Created\r\n\r\nFile uploaded successfully"
#define REQUEST403 "HTTP/1.1 403 Forbidden\r\n\r\nForbidden"
#define REQUEST404 "HTTP/1.1 404 Not Found\r\n\r\nFile not found"
#define REQUEST405 "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed"
#define REQUEST500 "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to delete file"

class FileHandler {
	private:
		static std::map<std::string, std::string> _mimeTypes;
		static std::map<std::string, std::string> initMimeTypes();
	public:
		static std::string getContentType(const std::string &path);
		static bool exists(const std::string &path);
		static std::string handleGetRequest(const std::string &path);
		static std::string handlePostRequest(const std::string &path, const std::string &body);
		static std::string handleDeleteRequest(const std::string &path);
		static std::string generateSessionID(void);
};

#endif
