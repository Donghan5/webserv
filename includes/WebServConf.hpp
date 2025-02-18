#ifndef WEBSERVCONF_HPP
#define WEBSERVCONF_HPP

#pragma once

/*
	This Class is aim to store and manage all data
	After Parse all data (Block) at the ParseConf, it store directly

	Simple Flow of Data Search
		WebServConf
			|
		HttpConf  --------- EventConf
			|
		ServerConf
			|
		LocationConf
*/

#include <vector>
#include <map>
#include <iostream>
#include <string>
#include "EventConf.hpp"
#include "HttpConf.hpp"
#include "ServerConf.hpp"
#include "LocationConf.hpp"

class HttpConf;
class EventConf;
class ServerConf;
class LocationConf;

typedef std::vector<const ServerConf*> const_server_vector;
typedef std::vector<const LocationConf*> const_location_vector;

class WebServConf {
	private:
		EventConf _econf;
		HttpConf *_hconf;

	public:
		WebServConf();
		WebServConf(const WebServConf &obj);
		~WebServConf();

		WebServConf &operator=(const WebServConf &obj);
		void setHttpBlock(const HttpConf &hconf);
		const HttpConf &getHttpBlock(void) const;
		const_server_vector findServerByName(std::string serverName) const; // using server_name in block
		const_location_vector findLocations(std::string path) const;

		EventConf &getEventConf(void);

		std::string getFirstListenValue() const;

		std::string resolveRoot(std::string host, std::string requestPath) const;
		std::string resolveRootFromServer(const ServerConf &serverConf, std::string requestPath, std::string httpRoot) const;
};

#endif
