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

typedef std::vector<const ServerConf*> const_server_vector;
typedef std::vector<const LocationConf*> const_location_vector;

class WebServConf {
	private:
		EventConf _eventConf;
		std::vector<HttpConf> _httpblocks;

	public:
		WebServConf();
		~WebServConf();

		void addHttpBlock(const HttpConf &hconf);
		const std::vector<HttpConf> &getHttpBlocks(void) const;
		const_server_vector findServersByName(std::string serverName) const; // using server_name in block
		const_location_vector findLocations(std::string path) const;

		EventConf getEventConf(void);

		std::string getFirstListenValue() const;
};

#endif
