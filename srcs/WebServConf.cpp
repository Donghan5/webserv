#include "../includes/WebServConf.hpp"
#include "../includes/Logger.hpp"
#include "../includes/Utils.hpp"

WebServConf::WebServConf() {
	_hconf = new HttpConf();
}

WebServConf::WebServConf(const WebServConf &obj) {
	_hconf = new HttpConf(*obj._hconf);
	_econf = obj._econf;
}

WebServConf &WebServConf::operator=(const WebServConf &obj) {
	if (this != &obj) {
		_econf = obj._econf;
		delete _hconf;
		_hconf = new HttpConf(*obj._hconf);
	}
	return *this;
}

WebServConf::~WebServConf() {
	delete _hconf;
}


HttpConf &WebServConf::getHttpBlock(void) {
	return *_hconf;
}

void WebServConf::setHttpBlock(const HttpConf &hconf) {
	delete _hconf;
	_hconf = new HttpConf(hconf);
}

/*
	Searching server by server_name (in configuration file)
	@param
		serverName: server_name
	@return
		result: the specific server
*/
const_server_vector WebServConf::findServerByName(std::string serverName) const {
	const_server_vector result;
	const std::vector<ServerConf> &servers = _hconf->getServerConfig();
	for (size_t j (0); j < servers.size(); j++) {
		if (servers[j].getData("server_name") == serverName) {
			result.push_back(&servers[j]);
		}
	}
	return result;
}

const_location_vector WebServConf::findLocations(std::string path) const {
	const_location_vector result;
	const std::vector<ServerConf> &servers = _hconf->getServerConfig();
	for (size_t j (0); j < servers.size(); j++) {
		const std::vector<LocationConf> &locations = servers[j].getLocations();
		for (size_t k (0); k < locations.size(); k++) {
			const std::string &locPath = locations[k].getData("path");
			if (locPath == path) {
				result.push_back(&locations[k]);
			}
		}
	}

	return result;
}

EventConf &WebServConf::getEventConf(void) {
	return _econf;
}

/*
	Get the first port
*/
std::string WebServConf::getFirstListenValue() const {
	const std::vector<ServerConf>& servers = _hconf->getServerConfig();
	if (servers.empty())
		return "";
	return servers[0].getData("listen");
}

/*
	Resolve root_dir from server
	@param
		host: it cames from request of the client (Http header)
	In process_request function in serv2.cpp
	host = extract_host(request);
*/
std::string WebServConf::resolveRootFromServer(const ServerConf &serverConf, std::string requestPath, std::string httpRoot) const {
	const location_vector &locations = serverConf.getLocations();
	for (size_t i (0); i < locations.size(); i++) {
		if (requestPath.find(locations[i].getPath()) == 0) {
			std::string locRoot = locations[i].getRootDir();
			if (!locRoot.empty()) {
				return locRoot;
			}
		}
	}

	std::string serRoot = serverConf.getRootDir();
	if (!serRoot.empty()) {
		return serRoot;
	}

	// have to think about how treat as multiple httph
	if (!httpRoot.empty()) {
		return httpRoot;
	}
	return "./www"; // return default value (if not set)
}

/*
	Find server_name using host(from request)
	@param
		host: request host name
		requestPath: URL root (GET methods)
			i.e.) GET /images/logo.png
*/
std::string WebServConf::resolveRoot(std::string host, std::string requestPath) const {
	const std::vector<ServerConf> &servers = _hconf->getServerByName(host);
		// std::cout << _httpblocks[i].getServerByName(host) << std::endl;
	for (size_t j (0); j < servers.size(); j++) {
		if (servers[j].getData("server_name") == host) {
			// std::cout << servers[j].getData("server_name") << std::endl;
			return resolveRootFromServer(servers[j], requestPath, _hconf->getData("root"));
		}
	}
	Logger::log(Logger::WARNING, "No matching server_name. Using default server");
	return "./www";
}


ServerConf *WebServConf::findMatchingServer(const std::string &host, int port) {
	ServerConf *defaultServer = NULL;
	Logger::log(Logger::DEBUG, "Entered host (findMatchingServer): " + host);

	const std::vector<ServerConf> &servers = _hconf->getServerConfig();

	if (servers.empty()) {
		Logger::log(Logger::ERROR, "Wait, what? servers is empty?");
		return NULL;
	}
	
	for (size_t i = 0; i < servers.size(); ++i) {
		Logger::log(Logger::DEBUG, "listen value: " + servers[i].getData("listen"));
		if (servers[i].getData("listen") == Utils::intToString(port)) {
			if (!defaultServer) { // first server is default server
				defaultServer = const_cast<ServerConf*>(&servers[i]);
			}
		}

		std::string serverNames = servers[i].getData("server_name");
		Logger::log(Logger::DEBUG, "server_name: " + serverNames);
		if (!serverNames.empty()) {
			std::vector<std::string> names = Utils::split(serverNames, ' ');
			if (names.empty()) {
				Logger::log(Logger::ERROR, "No server name found in" + serverNames);
			}
			for(size_t j = 0; j < names.size(); ++j) {
				Logger::log(Logger::DEBUG, "Checking server name: [" + names[j] + "] vs Host: [" + host + "]");
				Logger::log(Logger::DEBUG, "Checking server name: " + names[j]);
				if (names[j] == host) {
					return const_cast<ServerConf*>(&servers[i]);
				}
			}
		}
	}
	Logger::log(Logger::ERROR, "No matching server found, return defalut server");
	return defaultServer;
}

LocationConf *WebServConf::findMatchingLocation(const ServerConf &server, const std::string &path) {
	std::string bestMatch = "";
	LocationConf *bestLocation = NULL;
	const std::vector<LocationConf>& locations = server.getLocations();

	for (size_t i = 0; i < locations.size(); ++i) {
		if (path.find(locations[i].getPath()) == 0) {
			if (locations[i].getPath().length() > bestMatch.length()) { // select the longest path
				bestMatch = locations[i].getPath();
				bestLocation = const_cast<LocationConf*>(&locations[i]);
			}
		}
	}

	return bestLocation;
}
