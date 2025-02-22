#ifndef PARSEDREQUEST_HPP
#define PARSEDREQUEST_HPP

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <cstdlib>
#include <sys/socket.h>

typedef std::map<std::string, std::string> string_map;

class ParsedRequest {
	private:
		std::string _method; // method like GET, DELETE ...
		std::string _path;
		std::string _host;
		std::string _version;
		std::string _body;
		string_map _headers;
		string_map _cookies;

		ParsedRequest(); // treate default request path(?)

		public:
		ParsedRequest(const std::string &request);
		ParsedRequest(const ParsedRequest &obj);
		~ParsedRequest();

		ParsedRequest &operator=(const ParsedRequest &obj);

		// getter
		std::string getData(std::string key) const;
		std::string getMethod(void) const;
		std::string getPath(void) const;
		std::string getHost(void) const;
		std::string getVersion(void) const;
		std::string getBody(void) const;
		string_map getHeaders(void) const;

		// Cookie part
		void parseCookie(const std::string &cookieHeader);


		// setter
		void setHeaders(std::string key, std::string value);

		// Main parse
		void parseHttpRequest(const std::string &request);

};

#endif
