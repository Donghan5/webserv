#include "../includes/WebServConf.hpp"

WebServConf::WebServConf() {}

WebServConf::~WebServConf() {}

void WebServConf::addHttpBlock(const HttpConf &hconf) {
	_httpblocks.push_back(hconf);
}

const std::vector<HttpConf> &WebServConf::getHttpBlocks(void) const {
	return _httpblocks;
}

/*
	Searching server by server_name (in configuration file)
	@param
		serverName: server_name
	@return
		result: the specific server
*/
const_server_vector WebServConf::findServersByName(std::string serverName) const {
	const_server_vector result;
	for (size_t i (0); i < _httpblocks.size(); i++) {
		const std::vector<ServerConf> &servers = _httpblocks[i].getServerConfig();
		for (size_t j (0); j < servers.size(); j++) {
			if (servers[j].getData("server_name") == serverName) {
				locations[j].push_back(&servers[j]);
			}
		}
	}
	return result;
}

const_location_vector WebServConf::findLocations(std::string path) const {
	const_location_vector result;
	for (size_t i (0); i < _httpblocks.size; i++) {
		const std::vector<ServerConf> &servers = _httpblocks[i].getServerConfig();
		for (size_t j (0); j < servers.size(); j++) {
			const std::vector<LocationConf> &locations = servers[j].getLocations();
			for (size_t k (0); k < locations.size(); k++) {
				const std::string &locPath = locations[k].getData("path");
				if (locPath == path) {
					result.push_back(&locations[k]);
				}
			}
		}
	}
	return result;
}

const EventConf &WebServConf::getEventConf(void) {
	return _eventConf;
}

std::string WebServConf::getFirstListenValue() const {
	if (_httpBlocks.empty())
		return "";
	const std::vector<ServerConf>& servers = _httpBlocks[0].getServers();
	if (servers.empty())
		return "";
	return servers[0].getData("listen");
}

